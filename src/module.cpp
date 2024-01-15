#include <wizard/language_module.h>
#include <module_export.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>

using namespace wizard;

namespace py3lm {
	class Python3LanguageModule final : public ILanguageModule {
	public:
		Python3LanguageModule() = default;

		// ILanguageModule
		InitResult Initialize(std::weak_ptr<IWizardProvider> provider, const IModule& module) override {
			if (!(_provider = provider.lock())) {
				return ErrorData{ "Provider not exposed" };
			}

			Py_Initialize();
			PyRun_SimpleString("from time import time,ctime\n"
				"print('Today is', ctime(time()))\n");

			return InitResultData{};
		}

		void Shutdown() override {
			Py_Finalize();
		}

		void OnMethodExport(const IPlugin& plugin) override {
			// TODO: implement
		}

		LoadResult OnPluginLoad(const IPlugin& plugin) override {
			return ErrorData{ "Not implemented" };
		}

		void OnPluginStart(const IPlugin& plugin) override {
			// TODO: implement
		}

		void OnPluginEnd(const IPlugin& plugin) override {
			// TODO: implement
		}

	private:
		std::shared_ptr<IWizardProvider> _provider;
	};

	Python3LanguageModule g_py3lm;

	extern "C"
	PY3LM_EXPORT ILanguageModule* GetLanguageModule() {
		return &g_py3lm;
	}
}
