#pragma once

#include <plugify/jit/callback.hpp>
#include <plugify/jit/call.hpp>
#include <plugify/language_module.hpp>
#include <plugify/plugin.hpp>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <asmjit/asmjit.h>
#include <memory>
#include <optional>
#include <string>
#include <map>
#include <unordered_map>

namespace plugify {
	struct Vector2;
	struct Vector3;
	struct Vector4;
	struct Matrix4x4;
}

namespace py3lm {
	struct PythonMethodData {
		plugify::JitCallback jitCallback;
		PyObject* pythonFunction{};
	};

	class Python3LanguageModule final : public plugify::ILanguageModule {
	public:
		Python3LanguageModule();
		~Python3LanguageModule();

		// ILanguageModule
		plugify::InitResult Initialize(std::weak_ptr<plugify::IPlugifyProvider> provider, plugify::ModuleRef module) override;
		void Shutdown() override;
		void OnMethodExport(plugify::PluginRef plugin) override;
		plugify::LoadResult OnPluginLoad(plugify::PluginRef plugin) override;
		void OnPluginStart(plugify::PluginRef plugin) override;
		void OnPluginEnd(plugify::PluginRef plugin) override;
		bool IsDebugBuild() override;

	private:
		PyObject* FindExternal(void* funcAddr) const;
		void* FindInternal(PyObject* object) const;
		void AddToFunctionsMap(void* funcAddr, PyObject* object);

	public:
		PyObject* GetOrCreateFunctionObject(plugify::MethodRef method, void* funcAddr);
		std::optional<void*> GetOrCreateFunctionValue(plugify::MethodRef method, PyObject* object);
		PyObject* CreateVector2Object(const plugify::Vector2& vector);
		std::optional<plugify::Vector2> Vector2ValueFromObject(PyObject* object);
		PyObject* CreateVector3Object(const plugify::Vector3& vector);
		std::optional<plugify::Vector3> Vector3ValueFromObject(PyObject* object);
		PyObject* CreateVector4Object(const plugify::Vector4& vector);
		std::optional<plugify::Vector4> Vector4ValueFromObject(PyObject* object);
		PyObject* CreateMatrix4x4Object(const plugify::Matrix4x4& matrix);
		std::optional<plugify::Matrix4x4> Matrix4x4ValueFromObject(PyObject* object);
		void LogFatal(const std::string& msg) const;

	private:
		PyObject* FindPythonMethod(plugify::MemAddr addr) const;
		PyObject* CreateInternalModule(plugify::PluginRef plugin);
		PyObject* CreateExternalModule(plugify::PluginRef plugin);
		void TryCallPluginMethodNoArgs(plugify::PluginRef plugin, const std::string& name, const std::string& context);

	private:
		std::shared_ptr<plugify::IPlugifyProvider> _provider;
		std::shared_ptr<asmjit::JitRuntime> _jitRuntime;
		struct PluginData {
			PyObject* _module = nullptr;
			PyObject* _instance = nullptr;
		};
		std::map<plugify::UniqueId, PluginData> _pluginsMap;
		std::vector<PythonMethodData> _pythonMethods;
		PyObject* _PluginTypeObject = nullptr;
		PyObject* _PluginInfoTypeObject = nullptr;
		PyObject* _Vector2TypeObject = nullptr;
		PyObject* _Vector3TypeObject = nullptr;
		PyObject* _Vector4TypeObject = nullptr;
		PyObject* _Matrix4x4TypeObject = nullptr;
		PyObject* _ppsModule = nullptr;
		std::vector<std::vector<PyMethodDef>> _moduleMethods;
		std::vector<std::unique_ptr<PyModuleDef>> _moduleDefinitions;
		struct JitHolder {
			plugify::JitCallback jitCallback;
			plugify::JitCall jitCall;
		};
		std::vector<JitHolder> _moduleFunctions;
		struct ExternalHolder {
			plugify::JitCallback jitCallback;
			plugify::JitCall jitCall;
			std::unique_ptr<PyMethodDef> def;
			PyObject* object;
		};
		std::vector<ExternalHolder> _externalFunctions;
		std::vector<PythonMethodData> _internalFunctions;
		std::unordered_map<void*, PyObject*> _externalMap;
		std::unordered_map<PyObject*, void*> _internalMap;
	};
}
