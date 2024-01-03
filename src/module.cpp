#include <wizard/language_module.h>
#include <module_export.h>

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

			// TODO: implement

			return InitResultData{};
		}

		void Shutdown() override {
			// TODO: implement
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
