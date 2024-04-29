#include <plugify/function.h>
#include <plugify/plugify_provider.h>
#include <plugify/log.h>
#include <plugify/language_module.h>
#include <plugify/module.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/plugin.h>
#include <module_export.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <asmjit/asmjit.h>
#include <cuchar>
#include <climits>
#include <unordered_map>
#include <array>

using namespace plugify;
namespace fs = std::filesystem;

namespace py3lm {
	struct PythonMethodData {
		Function jitFunction;
		PyObject* pythonFunction{};
	};

	namespace {
		void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
			size_t start_pos{};
			while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
				str.replace(start_pos, from.length(), to);
				start_pos += to.length();
			}
		}

		bool IsStaticMethod(PyObject* obj) {
			if (PyCFunction_Check(obj)) {
				PyCFunctionObject* cfunc = reinterpret_cast<PyCFunctionObject*>(obj);
				if (cfunc->m_ml && (cfunc->m_ml->ml_flags & METH_STATIC)) {
					return true;
				}
			}
			return false;
		}

		using MethodExportError = std::string;
		using MethodExportData = PythonMethodData;
		using MethodExportResult = std::variant<MethodExportError, MethodExportData>;

		template<class T>
		inline constexpr bool always_false_v = std::is_same_v<std::decay_t<T>, std::add_cv_t<std::decay_t<T>>>;

		template<typename T>
		std::optional<T> ValueFromObject(PyObject* /*object*/) {
			static_assert(always_false_v<T>, "ValueFromObject specialization required");
		}

		template<>
		std::optional<bool> ValueFromObject(PyObject* object) {
			if (PyBool_Check(object)) {
				return { object == Py_True };
			}
			return std::nullopt;
		}

		template<>
		std::optional<char> ValueFromObject(PyObject* object) {
			if (PyUnicode_Check(object)) {
				const Py_ssize_t length = PyUnicode_GetLength(object);
				if (length == 0) {
					return { 0 };
				}
				if (length == 1) {
					char ch = PyUnicode_AsUTF8(object)[0];
					if ((ch & 0x80) == 0) {
						return { ch };
					}
					// Can't pass multibyte character
					PyErr_SetNone(PyExc_ValueError);
				} else {
					PyErr_SetNone(PyExc_ValueError);
				}
			}
			return std::nullopt;
		}

		template<>
		std::optional<char16_t> ValueFromObject(PyObject* object) {
			if (PyUnicode_Check(object)) {
				const Py_ssize_t length = PyUnicode_GetLength(object);
				if (length == 0) {
					return { 0 };
				}
				if (length == 1) {
					Py_ssize_t size{};
					const char* const buffer = PyUnicode_AsUTF8AndSize(object, &size);
					char16_t ch{};
					std::mbstate_t state{};
					const std::size_t rc = std::mbrtoc16(&ch, buffer, static_cast<size_t>(size), &state);
					if (rc == 1 || rc == 2 || rc == 3) {
						return { ch };
					}
					// Can't pass surrogate pair
					PyErr_SetNone(PyExc_ValueError);
				} else {
					PyErr_SetNone(PyExc_ValueError);
				}
			}
			return std::nullopt;
		}

		template<class ValueType, class CType, CType (*ConvertFunc)(PyObject*)>
		std::optional<ValueType> ValueFromNumberObject(PyObject* object) {
			if (PyLong_Check(object)) {
				const CType castResult = ConvertFunc(object);
				if (!PyErr_Occurred()) {
					if (castResult <= static_cast<CType>(std::numeric_limits<ValueType>::max())
						&& castResult >= static_cast<CType>(std::numeric_limits<ValueType>::min())
					) {
						return { static_cast<ValueType>(castResult) };
					}
					PyErr_SetNone(PyExc_OverflowError);
				}
			}
			return std::nullopt;
		}

		template<class ValueType, auto ConvertFunc>
		auto ValueFromNumberObject(PyObject* object) {
			return ValueFromNumberObject<ValueType, std::invoke_result_t<decltype(ConvertFunc), PyObject*>, ConvertFunc>(object);
		}

		template<>
		std::optional<int8_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<int8_t, PyLong_AsLong>(object);
		}

		template<>
		std::optional<int16_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<int16_t, PyLong_AsLong>(object);
		}

		template<>
		std::optional<int32_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<int32_t, PyLong_AsLong>(object);
		}

		template<>
		std::optional<int64_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<int64_t, PyLong_AsLongLong>(object);
		}

		template<>
		std::optional<uint8_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<uint8_t, PyLong_AsUnsignedLong>(object);
		}

		template<>
		std::optional<uint16_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<uint8_t, PyLong_AsUnsignedLong>(object);
		}

		template<>
		std::optional<uint32_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<uint8_t, PyLong_AsUnsignedLong>(object);
		}

		template<>
		std::optional<uint64_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<uint8_t, PyLong_AsUnsignedLongLong>(object);
		}

		template<>
		std::optional<void*> ValueFromObject(PyObject* object) {
			if (PyLong_Check(object)) {
				const auto result = PyLong_AsVoidPtr(object);
				if (!PyErr_Occurred()) {
					return result;
				}
			}
			return std::nullopt;
		}

		template<>
		std::optional<float> ValueFromObject(PyObject* object) {
			if (PyFloat_Check(object)) {
				return static_cast<float>(PyFloat_AS_DOUBLE(object));
			}
			return std::nullopt;
		}

		template<>
		std::optional<double> ValueFromObject(PyObject* object) {
			if (PyFloat_Check(object)) {
				return PyFloat_AS_DOUBLE(object);
			}
			return std::nullopt;
		}

		template<>
		std::optional<std::string> ValueFromObject(PyObject* object) {
			if (PyUnicode_Check(object)) {
				Py_ssize_t size{};
				const char* const buffer = PyUnicode_AsUTF8AndSize(object, &size);
				return std::make_optional<std::string>(buffer, buffer + size);
			}
			return std::nullopt;
		}

		template<typename T>
		std::optional<std::vector<T>> ArrayFromObject(PyObject* arrayObject) {
			if (!PyList_Check(arrayObject)) {
				return std::nullopt;
			}
			const Py_ssize_t size = PyList_Size(arrayObject);
			std::vector<T> array(static_cast<size_t>(size));
			for (Py_ssize_t i = 0; i < size; ++i) {
				if (PyObject* const valueObject = PyList_GetItem(arrayObject, i)) {
					if (auto value = ValueFromObject<T>(valueObject)) {
						array[static_cast<size_t>(i)] = std::move(*value);
						continue;
					}
				}
				return std::nullopt;
			}
			return std::move(array);
		}

		void SetFallbackReturn(ValueType retType, const ReturnValue* ret, const Parameters* params) {
			switch (retType) {
			case ValueType::Bool:
			case ValueType::Char8:
			case ValueType::Char16:
			case ValueType::Int8:
			case ValueType::Int16:
			case ValueType::Int32:
			case ValueType::Int64:
			case ValueType::UInt8:
			case ValueType::UInt16:
			case ValueType::UInt32:
			case ValueType::UInt64:
			case ValueType::Ptr64:
			case ValueType::Float:
			case ValueType::Double:
				// HACK: Fill all 8 byte with 0
				ret->SetReturnPtr<uintptr_t>({});
				break;
			case ValueType::String: {
				auto* const returnParam = params->GetArgument<std::string*>(0);
				std::construct_at(returnParam);
				break;
			}
			case ValueType::Function:
				// TODO: Log fail description
				std::terminate();
				break;
			case ValueType::ArrayChar8:
			case ValueType::ArrayChar16:
			case ValueType::ArrayInt8:
			case ValueType::ArrayInt16:
			case ValueType::ArrayInt32:
			case ValueType::ArrayInt64:
			case ValueType::ArrayUInt8:
			case ValueType::ArrayUInt16:
			case ValueType::ArrayUInt32:
			case ValueType::ArrayUInt64:
			case ValueType::ArrayPtr64:
			case ValueType::ArrayFloat:
			case ValueType::ArrayDouble: {
				// HACK: Assume the same structure for empty array
				auto* const returnParam = params->GetArgument<std::vector<uintptr_t>*>(0);
				std::construct_at(returnParam);
				break;
			}
			case ValueType::ArrayString: {
				auto* const returnParam = params->GetArgument<std::vector<std::string>*>(0);
				std::construct_at(returnParam);
				break;
			}
			default:
				// TODO: Log fail description
				std::terminate();
			}
		}

		bool SetReturn(PyObject* result, ValueType retType, const ReturnValue* ret, const Parameters* params) {
			switch (retType) {
			case ValueType::Void:
				return true;
			case ValueType::Bool:
				if (auto value = ValueFromObject<bool>(result)) {
					ret->SetReturnPtr<bool>(*value);
					return true;
				}
				break;
			case ValueType::Char8:
				if (auto value = ValueFromObject<char>(result)) {
					ret->SetReturnPtr<char>(*value);
					return true;
				}
				break;
			case ValueType::Char16:
				if (auto value = ValueFromObject<char16_t>(result)) {
					ret->SetReturnPtr<char16_t>(*value);
					return true;
				}
				break;
			case ValueType::Int8:
				if (auto value = ValueFromObject<int8_t>(result)) {
					ret->SetReturnPtr<int8_t>(*value);
					return true;
				}
				break;
			case ValueType::Int16:
				if (auto value = ValueFromObject<int16_t>(result)) {
					ret->SetReturnPtr<int16_t>(*value);
					return true;
				}
				break;
			case ValueType::Int32:
				if (auto value = ValueFromObject<int32_t>(result)) {
					ret->SetReturnPtr<int32_t>(*value);
					return true;
				}
				break;
			case ValueType::Int64:
				if (auto value = ValueFromObject<int64_t>(result)) {
					ret->SetReturnPtr<int64_t>(*value);
					return true;
				}
				break;
			case ValueType::UInt8:
				if (auto value = ValueFromObject<uint8_t>(result)) {
					ret->SetReturnPtr<uint8_t>(*value);
					return true;
				}
				break;
			case ValueType::UInt16:
				if (auto value = ValueFromObject<uint16_t>(result)) {
					ret->SetReturnPtr<uint16_t>(*value);
					return true;
				}
				break;
			case ValueType::UInt32:
				if (auto value = ValueFromObject<uint32_t>(result)) {
					ret->SetReturnPtr<uint32_t>(*value);
					return true;
				}
				break;
			case ValueType::UInt64:
				if (auto value = ValueFromObject<uint64_t>(result)) {
					ret->SetReturnPtr<uint64_t>(*value);
					return true;
				}
				break;
			case ValueType::Ptr64:
				if (auto value = ValueFromObject<void*>(result)) {
					ret->SetReturnPtr<void*>(*value);
					return true;
				}
				break;
			case ValueType::Float:
				if (auto value = ValueFromObject<float>(result)) {
					ret->SetReturnPtr<float>(*value);
					return true;
				}
				break;
			case ValueType::Double:
				if (auto value = ValueFromObject<double>(result)) {
					ret->SetReturnPtr<double>(*value);
					return true;
				}
				break;
			case ValueType::Function:
				if (auto value = ValueFromObject<void*>(result)) {
					ret->SetReturnPtr<void*>(*value);
					return true;
				}
				break;
			case ValueType::String:
				if (auto value = ValueFromObject<std::string>(result)) {
					auto* const returnParam = params->GetArgument<std::string*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayBool:
				if (auto value = ArrayFromObject<bool>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<bool>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayChar8:
				if (auto value = ArrayFromObject<char>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<char>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayChar16:
				if (auto value = ArrayFromObject<char16_t>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<char16_t>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayInt8:
				if (auto value = ArrayFromObject<int8_t>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<int8_t>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayInt16:
				if (auto value = ArrayFromObject<int16_t>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<int16_t>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayInt32:
				if (auto value = ArrayFromObject<int32_t>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<int32_t>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayInt64:
				if (auto value = ArrayFromObject<int64_t>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<int64_t>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayUInt8:
				if (auto value = ArrayFromObject<uint8_t>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<uint8_t>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayUInt16:
				if (auto value = ArrayFromObject<uint16_t>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<uint16_t>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayUInt32:
				if (auto value = ArrayFromObject<uint32_t>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<uint32_t>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayUInt64:
				if (auto value = ArrayFromObject<uint64_t>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<uint64_t>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayPtr64:
				if (auto value = ArrayFromObject<void*>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<void*>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayFloat:
				if (auto value = ArrayFromObject<float>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<float>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayDouble:
				if (auto value = ArrayFromObject<double>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<double>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			case ValueType::ArrayString:
				if (auto value = ArrayFromObject<std::string>(result)) {
					auto* const returnParam = params->GetArgument<std::vector<std::string>*>(0);
					std::construct_at(returnParam, std::move(*value));
					return true;
				}
				break;
			}

			return false;
		}

		template<typename T>
		PyObject* CreatePyObject(T /*value*/) {
			static_assert(always_false_v<T>, "CreatePyObject specialization required");
		}

		template<>
		PyObject* CreatePyObject(bool value) {
			return PyBool_FromLong(value);
		}

		template<>
		PyObject* CreatePyObject(char value) {
			return PyUnicode_FromStringAndSize(&value, static_cast<Py_ssize_t>(1));
		}

		template<>
		PyObject* CreatePyObject(char16_t value) {
			std::mbstate_t state{};
			std::array<char, MB_LEN_MAX> out{};
			std::size_t rc = std::c16rtomb(out.data(), value, &state);
			if (rc == static_cast<size_t>(-1)) {
				return nullptr;
			}
			return PyUnicode_FromStringAndSize(out.data(), static_cast<Py_ssize_t>(rc));
		}

		template<>
		PyObject* CreatePyObject(int8_t value) {
			return PyLong_FromLong(value);
		}

		template<>
		PyObject* CreatePyObject(int16_t value) {
			return PyLong_FromLong(value);
		}

		template<>
		PyObject* CreatePyObject(int32_t value) {
			return PyLong_FromLong(value);
		}

		template<>
		PyObject* CreatePyObject(int64_t value) {
			return PyLong_FromLongLong(value);
		}

		template<>
		PyObject* CreatePyObject(uint8_t value) {
			return PyLong_FromUnsignedLong(value);
		}

		template<>
		PyObject* CreatePyObject(uint16_t value) {
			return PyLong_FromUnsignedLong(value);
		}

		template<>
		PyObject* CreatePyObject(uint32_t value) {
			return PyLong_FromUnsignedLong(value);
		}

		template<>
		PyObject* CreatePyObject(uint64_t value) {
			return PyLong_FromUnsignedLongLong(value);
		}

		template<>
		PyObject* CreatePyObject(void* value) {
			return PyLong_FromVoidPtr(value);
		}

		template<>
		PyObject* CreatePyObject(float value) {
			return PyFloat_FromDouble(static_cast<double>(value));
		}

		template<>
		PyObject* CreatePyObject(double value) {
			return PyFloat_FromDouble(value);
		}

		template<>
		PyObject* CreatePyObject(std::string value) {
			return PyUnicode_FromStringAndSize(value.data(), static_cast<Py_ssize_t>(value.size()));
		}

		template<typename T>
		PyObject* CreatePyObjectList(const std::vector<T>& arrayArg) {
			const auto size = static_cast<Py_ssize_t>(arrayArg.size());
			PyObject* const arrayObject = PyList_New(size);
			if (arrayObject) {
				for (Py_ssize_t i = 0; i < size; ++i) {
					PyObject* const valueObject = CreatePyObject(arrayArg[i]);
					if (!valueObject) {
						Py_DECREF(arrayObject);
						return nullptr;
					}
					PyList_SET_ITEM(arrayObject, i, valueObject);
				}
			}
			return arrayObject;
		}

		PyObject* ParamToObject(ValueType type, const Parameters* params, uint8_t index) {
			switch (type) {
			case ValueType::Bool:
				return CreatePyObject(params->GetArgument<bool>(index));
			case ValueType::Char8:
				return CreatePyObject(params->GetArgument<char>(index));
			case ValueType::Char16:
				return CreatePyObject(params->GetArgument<char16_t>(index));
			case ValueType::Int8:
				return CreatePyObject(params->GetArgument<int8_t>(index));
			case ValueType::Int16:
				return CreatePyObject(params->GetArgument<int16_t>(index));
			case ValueType::Int32:
				return CreatePyObject(params->GetArgument<int32_t>(index));
			case ValueType::Int64:
				return CreatePyObject(params->GetArgument<int64_t>(index));
			case ValueType::UInt8:
				return CreatePyObject(params->GetArgument<uint8_t>(index));
			case ValueType::UInt16:
				return CreatePyObject(params->GetArgument<uint16_t>(index));
			case ValueType::UInt32:
				return CreatePyObject(params->GetArgument<uint32_t>(index));
			case ValueType::UInt64:
				return CreatePyObject(params->GetArgument<uint64_t>(index));
			case ValueType::Ptr64:
				return CreatePyObject(params->GetArgument<void*>(index));
			case ValueType::Float:
				return CreatePyObject(params->GetArgument<float>(index));
			case ValueType::Double:
				return CreatePyObject(params->GetArgument<double>(index));
			case ValueType::Function:
				// TODO: Generate External call
				// TODO: if address is wrapped External call, get origial python object
				return CreatePyObject(params->GetArgument<void*>(index));
			case ValueType::String:
				return CreatePyObject(*(params->GetArgument<const std::string*>(index)));
			case ValueType::ArrayBool:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<bool>*>(index)));
			case ValueType::ArrayChar8:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<char>*>(index)));
			case ValueType::ArrayChar16:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<char16_t>*>(index)));
			case ValueType::ArrayInt8:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<int8_t>*>(index)));
			case ValueType::ArrayInt16:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<int16_t>*>(index)));
			case ValueType::ArrayInt32:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<int32_t>*>(index)));
			case ValueType::ArrayInt64:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<int64_t>*>(index)));
			case ValueType::ArrayUInt8:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<uint8_t>*>(index)));
			case ValueType::ArrayUInt16:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<uint16_t>*>(index)));
			case ValueType::ArrayUInt32:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<uint32_t>*>(index)));
			case ValueType::ArrayUInt64:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<uint64_t>*>(index)));
			case ValueType::ArrayPtr64:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<void*>*>(index)));
			case ValueType::ArrayFloat:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<float>*>(index)));
			case ValueType::ArrayDouble:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<double>*>(index)));
			case ValueType::ArrayString:
				return CreatePyObjectList(*(params->GetArgument<const std::vector<std::string>*>(index)));
			default:
				return nullptr;
			}
		}

		void InternalCall(const Method* method, void* data, const Parameters* params, const uint8_t count, const ReturnValue* ret) {
			PyObject* const func = reinterpret_cast<PyObject*>(data);

			enum class ParamProcess {
				NoError,
				Error,
				ErrorWithException
			};
			ParamProcess processResult = ParamProcess::NoError;

			uint8_t params_count = static_cast<uint8_t>(method->paramTypes.size());
			uint8_t params_start_index = method->retType.type > ValueType::LastPrimitive ? 1 : 0;

			PyObject* argTuple = nullptr;
			if (params_count) {
				argTuple = PyTuple_New(params_count);
				if (!argTuple) {
					processResult = ParamProcess::Error;
				}
				else {
					for (uint8_t index = 0; index < params_count; ++index) {
						PyObject* const arg = ParamToObject(method->paramTypes[index].type, params, params_start_index + index);
						if (!arg) {
							processResult = ParamProcess::Error;
							break;
						}
						if (PyTuple_SetItem(argTuple, index, arg) != 0) {
							Py_DECREF(arg);
							processResult = ParamProcess::ErrorWithException;
							break;
						}
						// arg reference "stolen" by tuple set, no need to handle
					}
				}
			}

			if (processResult != ParamProcess::NoError) {
				if (argTuple) {
					Py_DECREF(argTuple);
				}
				if (processResult == ParamProcess::ErrorWithException) {
					PyErr_Print();
				}

				SetFallbackReturn(method->retType.type, ret, params);

				return;
			}

			PyObject* const result = PyObject_CallObject(func, argTuple);

			if (argTuple) {
				Py_DECREF(argTuple);
			}

			if (!result) {
				PyErr_Print();

				SetFallbackReturn(method->retType.type, ret, params);

				return;
			}

			if (!SetReturn(result, method->retType.type, ret, params)) {
				if (PyErr_Occurred()) {
					PyErr_Print();
				}

				SetFallbackReturn(method->retType.type, ret, params);
			}

			Py_DECREF(result);
		}

		MethodExportResult GenerateMethodExport(const Method& method, const std::shared_ptr<asmjit::JitRuntime>& jitRuntime, PyObject* pluginModule, PyObject* pluginInstance) {
			PyObject* func{};

			std::string className, methodName;
			{
				const auto& funcName = method.funcName;
				if (const auto pos = funcName.find('.'); pos != std::string::npos) {
					className = funcName.substr(0, pos);
					methodName = std::string(funcName.begin() + (pos + 1), funcName.end());
				}
				else {
					methodName = funcName;
				}
			}

			const bool funcIsMethod = !className.empty();

			if (funcIsMethod) {
				PyObject* const classType = PyObject_GetAttrString(pluginModule, className.c_str());
				if (classType) {
					func = PyObject_GetAttrString(classType, methodName.c_str());
					Py_DECREF(classType);
				}
			}
			else {
				func = PyObject_GetAttrString(pluginModule, methodName.c_str());
			}

			if (!func) {
				return MethodExportError{ std::format("{} (Not found '{}' in module)", method.name, method.funcName) };
			}

			if (!PyFunction_Check(func)) {
				Py_DECREF(func);
				return MethodExportError{ std::format("{} ('{}' not function type)", method.name, method.funcName) };
			}

			if (funcIsMethod && !IsStaticMethod(func)) {
				PyObject* const bind = PyMethod_New(func, pluginInstance);
				Py_DECREF(func);
				if (!bind) {
					return MethodExportError{ std::format("{} (instance bind fail)", method.name) };
				}
				func = bind;
			}

			Function function(jitRuntime);

			void* const methodAddr = function.GetJitFunc(method, &InternalCall, reinterpret_cast<void*>(func));

			if (!methodAddr) {
				Py_DECREF(func);
				return MethodExportError{ std::format("{} (jit error: {})", method.name, function.GetError()) };
			}

			return MethodExportData{ std::move(function), func };
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

			_jitRuntime = std::make_shared<asmjit::JitRuntime>();

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
				for (const auto& data : _pythonMethods) {
					Py_DECREF(data.pythonFunction);
				}

				for (const auto& [_, pluginData] : _pluginsMap) {
					Py_DECREF(pluginData._instance);
					Py_DECREF(pluginData._module);
				}

				Py_Finalize();
			}
			_pythonMethods.clear();
			_pluginsMap.clear();
			_jitRuntime.reset();
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
			bool exportResult = true;
			std::vector<std::string> exportErrors;
			std::vector<std::tuple<std::reference_wrapper<const Method>, PythonMethodData>> methodsHolders;

			if (!exportedMethods.empty()) {
				for (const auto& method : exportedMethods) {
					MethodExportResult generateResult = GenerateMethodExport(method, _jitRuntime, pluginModule, pluginInstance);
					if (auto* data = std::get_if<MethodExportError>(&generateResult)) {
						exportResult = false;
						exportErrors.emplace_back(std::move(*data));
						continue;
					}
					methodsHolders.emplace_back(std::cref(method), std::move(std::get<MethodExportData>(generateResult)));
				}
			}

			if (!exportResult) {
				Py_DECREF(pluginInstance);
				Py_DECREF(pluginModule);
				std::string errorString = "Methods export error(s): " + exportErrors[0];
				for (auto it = std::next(exportErrors.begin()); it != exportErrors.end(); ++it) {
					std::format_to(std::back_inserter(errorString), ", {}", *it);
				}
				return ErrorData{ std::move(errorString) };
			}

			const auto [_, result] = _pluginsMap.try_emplace(plugin.GetName(), pluginModule, pluginInstance);
			if (!result) {
				Py_DECREF(pluginInstance);
				Py_DECREF(pluginModule);
				return ErrorData{ std::format("Save plugin data to map unsuccessful") };
			}

			std::vector<MethodData> methods;
			methods.reserve(methodsHolders.size());
			_pythonMethods.reserve(methodsHolders.size());

			for (auto& [method, methodData] : methodsHolders) {
				methods.emplace_back(method.get().name, methodData.jitFunction.GetFunction());
				_pythonMethods.emplace_back(std::move(methodData));
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
		std::shared_ptr<asmjit::JitRuntime> _jitRuntime;
		struct PluginData {
			PyObject* _module = nullptr;
			PyObject* _instance = nullptr;
		};
		std::unordered_map<std::string, PluginData> _pluginsMap;
		std::vector<PythonMethodData> _pythonMethods;
	};

	Python3LanguageModule g_py3lm;

	extern "C"
	PY3LM_EXPORT ILanguageModule* GetLanguageModule() {
		return &g_py3lm;
	}
}
