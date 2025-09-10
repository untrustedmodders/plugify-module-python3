#pragma once

#include <plugify/callback.hpp>
#include <plugify/call.hpp>
#include <plugify/language_module.hpp>
#include <plugify/extension.hpp>
#include <plg/numerics.hpp>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <memory>
#include <optional>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>

using namespace plugify;

namespace py3lm {
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
	using PythonExternalEnumMap = std::unordered_map<const EnumObject*, std::shared_ptr<PythonEnumMap>>;
	using PythonInternalEnumMap = std::unordered_map<PyObject*, std::shared_ptr<PythonEnumMap>>;

	struct PythonMethodData {
		JitCallback jitCallback;
		PyObject* pythonFunction{};
	};

	class Python3LanguageModule final : public ILanguageModule {
	public:
		Python3LanguageModule();
		~Python3LanguageModule();

		// ILanguageModule
		Result<InitData> Initialize(const Provider& provider, const Extension& module) override;
		void Shutdown() override;
		void OnUpdate(std::chrono::milliseconds dt) override {};
		void OnMethodExport(const Extension& plugin) override;
		Result<LoadData> OnPluginLoad(const Extension& plugin) override;
		void OnPluginStart(const Extension& plugin) override;
		void OnPluginUpdate(const Extension& plugin, std::chrono::milliseconds dt) override;
		void OnPluginEnd(const Extension& plugin) override;
		bool IsDebugBuild() override;

	private:
		PyObject* FindExternal(void* funcAddr) const;
		void* FindInternal(PyObject* object) const;
		void AddToFunctionsMap(void* funcAddr, PyObject* object);

	public:
		PyObject* GetOrCreateFunctionObject(const Method& method, void* funcAddr);
		std::optional<void*> GetOrCreateFunctionValue(const Method& method, PyObject* object);
		PyObject* CreateVector2Object(const plg::vec2& vector);
		std::optional<plg::vec2> Vector2ValueFromObject(PyObject* object);
		PyObject* CreateVector3Object(const plg::vec3& vector);
		std::optional<plg::vec3> Vector3ValueFromObject(PyObject* object);
		PyObject* CreateVector4Object(const plg::vec4& vector);
		std::optional<plg::vec4> Vector4ValueFromObject(PyObject* object);
		PyObject* CreateMatrix4x4Object(const plg::mat4x4& matrix);
		std::optional<plg::mat4x4> Matrix4x4ValueFromObject(PyObject* object);
		PythonType GetObjectType(PyObject* type) const;
		PyObject* GetEnumObject(const EnumObject& enumerator, int64_t value) const;
		void CreateEnumObject(const EnumObject& enumerator, PyObject* moduleDict);
		void ResolveRequiredModule(std::string_view moduleName);
		std::vector<std::string> ExtractRequiredModules(const std::string& modulePath);

		const std::unique_ptr<Provider>& GetProvider() const { return _provider; }
		void LogFatal(std::string_view msg) const;
		void LogError() const;

	private:
		PyObject* FindPythonMethod(MemAddr addr) const;
		PyObject* CreateInternalModule(const Extension& plugin, PyObject* moduleObject = nullptr);
		PyObject* CreateExternalModule(const Extension& plugin, PyObject* moduleObject = nullptr);
		void TryCreateModule(const Extension& plugin, bool empty);

	private:
		std::unique_ptr<Provider> _provider;
		struct PluginData {
			PyObject* module = nullptr;
			PyObject* instance = nullptr;
			PyObject* update = nullptr;
			PyObject* start = nullptr;
			PyObject* end = nullptr;
		};
		std::unordered_map<UniqueId, PluginData> _pluginsMap;
		std::vector<PythonMethodData> _pythonMethods;
		PyObject* _PluginTypeObject = nullptr;
		PyObject* _PluginInfoTypeObject = nullptr;
		PyObject* _Vector2TypeObject = nullptr;
		PyObject* _Vector3TypeObject = nullptr;
		PyObject* _Vector4TypeObject = nullptr;
		PyObject* _Matrix4x4TypeObject = nullptr;
		PyObject* _ExtractRequiredModulesObject = nullptr;
		PyObject* _ppsModule = nullptr;
		PyObject* _enumModule = nullptr;
		PyObject* _formatException = nullptr;
		std::vector<std::vector<PyMethodDef>> _moduleMethods;
		struct JitHolder {
			JitCallback jitCallback;
			JitCall jitCall;
		};
		std::vector<JitHolder> _moduleFunctions;
		struct ExternalHolder {
			JitCallback jitCallback;
			JitCall jitCall;
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
