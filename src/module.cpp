#include <plugify/plugify_provider.h>
#include <plugify/log.h>
#include <plugify/language_module.h>
#include <plugify/module.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/plugin.h>
#include <module_export.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <unordered_map>

using namespace plugify;
namespace fs = std::filesystem;

namespace py3lm {
	namespace {
		void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
			size_t start_pos{};
			while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
				str.replace(start_pos, from.length(), to);
				start_pos += to.length();
			}
		}
	}

	class Python3LanguageModule final : public ILanguageModule {
	public:
		Python3LanguageModule() = default;

		// ILanguageModule
		InitResult Initialize(std::weak_ptr<IPlugifyProvider> provider, const IModule& module) override {
			if (!(_provider = provider.lock())) {
				return ErrorData{ "Provider not exposed" };
			}

			std::error_code ec;
			const fs::path moduleBasePath = fs::absolute(module.GetBaseDir(), ec);
			if (ec) {
				return ErrorData{ "Failed to get module directory path" };
			}

			const fs::path libPath = moduleBasePath / "lib";
			if (!fs::exists(libPath, ec) || !fs::is_directory(libPath, ec)) {
				return ErrorData{ "lib directory not exists" };
			}

			const fs::path pythonBasePath = moduleBasePath / "python3.12";
			if (!fs::exists(pythonBasePath, ec) || !fs::is_directory(pythonBasePath, ec)) {
				return ErrorData{ "python3.12 directory not exists" };
			}

			const fs::path modulesZipPath = pythonBasePath / L"python312.zip";
			const fs::path pluginsPath = fs::absolute(moduleBasePath / ".." / ".." / "plugins", ec);
			if (ec) {
				return ErrorData{ "Failed to get plugins directory path" };
			}

			if (Py_IsInitialized()) {
				return ErrorData{ "Python already initialized" };
			}

			PyStatus status;

			PyConfig config{};
			PyConfig_InitIsolatedConfig(&config);

			for (;;) {
				status = PyConfig_SetString(&config, &config.home, pythonBasePath.wstring().c_str());
				if (PyStatus_Exception(status)) {
					break;
				}

				// Manually set search paths:
				// 1. python zip
				// 2. python dir
				// 3. lib dir in module
				// 4. plugins dir

				config.module_search_paths_set = 1;

				status = PyWideStringList_Append(&config.module_search_paths, modulesZipPath.wstring().c_str());
				if (PyStatus_Exception(status)) {
					break;
				}
				status = PyWideStringList_Append(&config.module_search_paths, pythonBasePath.wstring().c_str());
				if (PyStatus_Exception(status)) {
					break;
				}
				status = PyWideStringList_Append(&config.module_search_paths, libPath.wstring().c_str());
				if (PyStatus_Exception(status)) {
					break;
				}
				status = PyWideStringList_Append(&config.module_search_paths, pluginsPath.wstring().c_str());
				if (PyStatus_Exception(status)) {
					break;
				}

				status = Py_InitializeFromConfig(&config);

				break;
			}

			if (PyStatus_Exception(status)) {
				return ErrorData{ std::format("Failed to init python: {}", status.err_msg) };
			}

			PyObject* const plugifyPluginModuleName = PyUnicode_DecodeFSDefault("plugify.plugin");
			if (!plugifyPluginModuleName) {
				PyErr_Print();
				return ErrorData{ "Failed to allocate plugify.plugin module string" };
			}

			PyObject* const plugifyPluginModule = PyImport_Import(plugifyPluginModuleName);
			Py_DECREF(plugifyPluginModuleName);
			if (!plugifyPluginModule) {
				PyErr_Print();
				return ErrorData{ "Failed to import plugify.plugin python module" };
			}

			Py_DECREF(plugifyPluginModule);

			return InitResultData{};
		}

		void Shutdown() override {
			if (Py_IsInitialized()) {
				for (const auto& [_, pluginData] : _pluginsMap) {
					Py_DECREF(pluginData._instance);
					Py_DECREF(pluginData._module);
				}
				_pluginsMap.clear();

				Py_Finalize();
			}
			_provider.reset();
		}

		void OnMethodExport(const IPlugin& plugin) override {
			// TODO: implement
		}

		LoadResult OnPluginLoad(const IPlugin& plugin) override {
			std::error_code ec;

			const fs::path& baseFolder = plugin.GetBaseDir();
			fs::path filePathRelative = plugin.GetDescriptor().entryPoint;
			if (filePathRelative.empty() || filePathRelative.extension() != ".py") {
				return ErrorData{ "Incorrect entry point: empty or not .py" };
			}
			const fs::path filePath = baseFolder / filePathRelative;
			if (!fs::exists(filePath, ec) || !fs::is_regular_file(filePath, ec)) {
				return ErrorData{ std::format("Module file '{}' not exist", filePath.string()) };
			}
			const fs::path pluginsFolder = baseFolder.parent_path();
			filePathRelative = fs::relative(filePath, pluginsFolder, ec);
			filePathRelative.replace_extension();
			std::string moduleName = filePathRelative.generic_string();
			ReplaceAll(moduleName, "/", ".");

			_provider->Log(std::format("[py3lm] Load plugin module '{}'", moduleName), Severity::Verbose);

			PyObject* const moduleNameString = PyUnicode_DecodeFSDefault(moduleName.c_str());
			if (!moduleNameString) {
				PyErr_Print();
				return ErrorData{ "Failed to allocate string for plugin module name" };
			}

			PyObject* const pluginModule = PyImport_Import(moduleNameString);
			Py_DECREF(moduleNameString);
			if (!pluginModule) {
				PyErr_Print();
				return ErrorData{ std::format("Failed to import {} module", moduleName) };
			}

			PyObject* const pluginInfo = PyObject_GetAttrString(pluginModule, "__plugin__");
			if (!pluginInfo) {
				Py_DECREF(pluginModule);
				PyErr_Print();
				return ErrorData{ "Plugin info (__plugin__) not found in module" };
			}

			PyObject* const classNameString = PyObject_GetAttrString(pluginInfo, "class_name");
			if (!classNameString) {
				Py_DECREF(pluginInfo);
				Py_DECREF(pluginModule);
				PyErr_Print();
				return ErrorData{ "Plugin main class name (__plugin__.class_name) not found in module" };
			}

			PyObject* const pluginInstance = PyObject_CallMethodNoArgs(pluginModule, classNameString);
			Py_DECREF(classNameString);
			if (!pluginInstance) {
				Py_DECREF(pluginInfo);
				Py_DECREF(pluginModule);
				PyErr_Print();
				return ErrorData{ "Failed to create plugin instance" };
			}

			const int resultCode = PyObject_SetAttrString(pluginInfo, "instance", pluginInstance);
			Py_DECREF(pluginInfo);
			if (resultCode != 0) {
				Py_DECREF(pluginInstance);
				Py_DECREF(pluginModule);
				PyErr_Print();
				return ErrorData{ "Failed to save plugin instance" };
			}

			if (_pluginsMap.contains(plugin.GetName())) {
				Py_DECREF(pluginInstance);
				Py_DECREF(pluginModule);
				return ErrorData{ std::format("Plugin name duplicate") };
			}

			const auto& exportedMethods = plugin.GetDescriptor().exportedMethods;
			std::vector<MethodData> methods;
			if (!exportedMethods.empty()) {
				Py_DECREF(pluginInstance);
				Py_DECREF(pluginModule);
				return ErrorData{ std::format("Methods export not implemented") };
			}

			const auto [_, result] = _pluginsMap.try_emplace(plugin.GetName(), pluginModule, pluginInstance);
			if (!result) {
				Py_DECREF(pluginInstance);
				Py_DECREF(pluginModule);
				return ErrorData{ std::format("Save plugin data to map unsuccessful") };
			}

			return LoadResultData{ std::move(methods) };
		}

		void OnPluginStart(const IPlugin& plugin) override {
			TryCallPluginMethodNoArgs(plugin, "plugin_start", "OnPluginStart");
		}

		void OnPluginEnd(const IPlugin& plugin) override {
			TryCallPluginMethodNoArgs(plugin, "plugin_end", "OnPluginEnd");
		}

	private:
		void TryCallPluginMethodNoArgs(const IPlugin& plugin, const std::string& name, const std::string& context) {
			const auto it = _pluginsMap.find(plugin.GetName());
			if (it == _pluginsMap.end()) {
				_provider->Log(std::format("[py3lm] {}: plugin '{}' not found in map", context, plugin.GetName()), Severity::Error);
				return;
			}

			const auto& pluginData = std::get<PluginData>(*it);
			if (!pluginData._instance) {
				_provider->Log(std::format("[py3lm] {}: null plugin instance", context), Severity::Error);
				return;
			}

			PyObject* const nameString = PyUnicode_DecodeFSDefault(name.c_str());
			if (!nameString) {
				PyErr_Print();
				_provider->Log(std::format("[py3lm] {}: failed to allocate name string", context), Severity::Error);
				return;
			}

			if (PyObject_HasAttr(pluginData._instance, nameString)) {
				PyObject* const returnObject = PyObject_CallMethodNoArgs(pluginData._instance, nameString);
				if (!returnObject) {
					PyErr_Print();
					_provider->Log(std::format("[py3lm] {}: call '{}' failed", context, name), Severity::Error);
				}
			}

			Py_DECREF(nameString);

			return;
		}

	private:
		std::shared_ptr<IPlugifyProvider> _provider;
		struct PluginData {
			PyObject* _module = nullptr;
			PyObject* _instance = nullptr;
		};
		std::unordered_map<std::string, PluginData> _pluginsMap;
	};

	Python3LanguageModule g_py3lm;

	extern "C"
	PY3LM_EXPORT ILanguageModule* GetLanguageModule() {
		return &g_py3lm;
	}
}
