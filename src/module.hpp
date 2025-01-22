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
#include <unordered_set>

template <>
struct std::hash<plugify::EnumRef> {
	std::size_t operator()(const plugify::EnumRef& enumerator) const {
		union {
			plugify::EnumRef enumerate;
			std::uintptr_t ptr;
		} cast{ enumerator };
		return std::hash<std::uintptr_t>{}(cast.ptr);
	}
};

namespace py3lm {
	struct PythonMethodData {
		plugify::JitCallback jitCallback;
		PyObject* pythonFunction{};
	};

	enum class PyAbstractType : size_t {
		Type,
		BaseObject,
		Long,
		Bool,
		Ellipsis,
		None,
		NotImplemented,
		ByteArrayIter,
		ByteArray,
		BytesIter,
		Bytes,
		CFunction,
		CallIter,
		Capsule,
		Cell,
		ClassMethod,
		Complex,
		DictItems,
		DictIterItem,
		DictIterKey,
		DictIterValue,
		DictKeys,
		DictProxy,
		DictValues,
		Dict,
		Enum,
		Filter,
		Float,
		Frame,
		FrozenSet,
		Function,
		Gen,
		InstanceMethod,
		ListIter,
		ListRevIter,
		List,
		LongRangeIter,
		Map,
		MemoryView,
		Method,
		Module,
		Property,
		RangeIter,
		Range,
		SeqIter,
		SetIter,
		Set,
		Slice,
		StaticMethod,
		TraceBack,
		TupleIter,
		Tuple,
		UnicodeIter,
		Unicode,
		Zip,
		StdPrinter,
		Code,
		Reversed,
		ClassMethodDescr,
		GetSetDescr,
		WrapperDescr,
		MethodDescr,
		MemberDescr,
		Super,

		Vector2,
		Vector3,
		Vector4,
		Matrix4x4,

		Max = Matrix4x4,

		Invalid = static_cast<size_t>(-1)
	};

	constexpr auto MaxPyTypes = static_cast<size_t>(PyAbstractType::Max);

	struct PythonType {
		PyAbstractType type;
		const char* name;
	};

	using PythonInternalMap = std::unordered_map<PyObject*, void*>;
	using PythonExternalMap = std::unordered_map<void*, PyObject*>;
	using PythonTypeMap = std::unordered_map<PyTypeObject*, PythonType>;
	using PythonEnumMap = std::map<int64_t, PyObject*>;
	using PythonExternalEnumMap = std::unordered_map<plugify::EnumRef, std::shared_ptr<PythonEnumMap>>;
	using PythonInternalEnumMap = std::unordered_map<PyObject*, std::shared_ptr<PythonEnumMap>>;

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
		PyObject* GetEnumObject(plugify::EnumRef enumerator, int64_t value) const;
		void CreateEnumObject(plugify::EnumRef enumerator, PyObject* module);

		void LogFatal(std::string_view msg) const;
		void LogError() const;

	private:
		PyObject* FindPythonMethod(plugify::MemAddr addr) const;
		PyObject* CreateInternalModule(plugify::PluginRef plugin);
		PyObject* CreateExternalModule(plugify::PluginRef plugin);
		void TryCallPluginMethodNoArgs(plugify::PluginRef plugin, std::string_view name, std::string_view context);

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
		PyObject* _enumModule = nullptr;
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
		PythonExternalMap _externalMap;
		PythonInternalMap _internalMap;
		PythonTypeMap _typeMap;
		PythonExternalEnumMap _externalEnumMap;
		PythonInternalEnumMap _internalEnumMap;
	};
}
