#pragma once

#include <plugify/jit/callback.hpp>
#include <plugify/jit/call.hpp>
#include <plugify/language_module.hpp>
#include <plugify/plugin.hpp>
#include <plugify/numerics.hpp>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <asmjit/asmjit.h>
#include <memory>
#include <optional>
#include <string>
#include <map>
#include <unordered_map>

namespace py3lm {
	struct PythonMethodData {
		plugify::JitCallback jitCallback;
		PyObject* pythonFunction{};
	};

	enum class PyAbstractType : int64_t {
		Invalid          = 0LL,
		Type             = 1LL << 1 ,
		BaseObject       = 1LL << 2 ,
		Long             = 1LL << 3 ,
		Bool             = 1LL << 4 ,
		Ellipsis         = 1LL << 5 ,
		None             = 1LL << 6 ,
		NotImplemented   = 1LL << 7 ,
		ByteArrayIter    = 1LL << 8 ,
		ByteArray        = 1LL << 9 ,
		BytesIter        = 1LL << 10,
		Bytes            = 1LL << 11,
		CFunction        = 1LL << 12,
		CallIter         = 1LL << 13,
		Capsule          = 1LL << 14,
		Cell             = 1LL << 15,
		ClassMethod      = 1LL << 16,
		Complex          = 1LL << 17,
		DictItems        = 1LL << 18,
		DictIterItem     = 1LL << 19,
		DictIterKey      = 1LL << 20,
		DictIterValue    = 1LL << 21,
		DictKeys         = 1LL << 22,
		DictProxy        = 1LL << 23,
		DictValues       = 1LL << 24,
		Dict             = 1LL << 25,
		Enum             = 1LL << 26,
		Filter           = 1LL << 27,
		Float            = 1LL << 28,
		Frame            = 1LL << 29,
		FrozenSet        = 1LL << 30,
		Function         = 1LL << 31,
		Gen              = 1LL << 32,
		InstanceMethod   = 1LL << 33,
		ListIter         = 1LL << 34,
		ListRevIter      = 1LL << 35,
		List             = 1LL << 36,
		LongRangeIter    = 1LL << 37,
		Map              = 1LL << 38,
		MemoryView       = 1LL << 39,
		Method           = 1LL << 40,
		Module           = 1LL << 41,
		Property         = 1LL << 42,
		RangeIter        = 1LL << 43,
		Range            = 1LL << 44,
		SeqIter          = 1LL << 45,
		SetIter          = 1LL << 46,
		Set              = 1LL << 47,
		Slice            = 1LL << 48,
		StaticMethod     = 1LL << 49,
		TraceBack        = 1LL << 50,
		TupleIter        = 1LL << 51,
		Tuple            = 1LL << 52,
		UnicodeIter      = 1LL << 53,
		Unicode          = 1LL << 54,
		Zip              = 1LL << 55,
		StdPrinter       = 1LL << 56,
		Code             = 1LL << 57,
		Reversed         = 1LL << 58,
		ClassMethodDescr = 1LL << 59,
		GetSetDescr      = 1LL << 60,
		WrapperDescr     = 1LL << 61,
		MethodDescr      = 1LL << 62,
		MemberDescr      = 1LL << 63,
		//Super            = 1LL << 64,
	};

	struct PythonType {
		PyAbstractType type;
		const char* name;
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
		PyObject* CreateVector2Object(const plg::vec2& vector);
		std::optional<plg::vec2> Vector2ValueFromObject(PyObject* object);
		PyObject* CreateVector3Object(const plg::vec3& vector);
		std::optional<plg::vec3> Vector3ValueFromObject(PyObject* object);
		PyObject* CreateVector4Object(const plg::vec4& vector);
		std::optional<plg::vec4> Vector4ValueFromObject(PyObject* object);
		PyObject* CreateMatrix4x4Object(const plg::mat4x4& matrix);
		std::optional<plg::mat4x4> Matrix4x4ValueFromObject(PyObject* object);
		PythonType GetObjectType(PyObject* type) const;
		void LogFatal(std::string_view msg) const;
		void LogError() const;

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
		PyObject* _formatException = nullptr;
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
		std::unordered_map<PyTypeObject*, PythonType> _typeMap;
	};
}
