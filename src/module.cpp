#include <plugify/function.h>
#include <plugify/plugify_provider.h>
#include <plugify/compat_format.h>
#include <plugify/log.h>
#include <plugify/language_module.h>
#include <plugify/module.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/plugin.h>
#include <module_export.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <dyncall/dyncall.h>
#include <asmjit/asmjit.h>
#include <cuchar>
#include <climits>
#include <unordered_map>
#include <array>
#include <memory>

using namespace plugify;
namespace fs = std::filesystem;

namespace py3lm {
	struct PythonMethodData {
		Function jitFunction;
		PyObject* pythonFunction{};
	};

	static PyObject* GetOrCreateFunctionObject(const Method& method, void* funcAddr);
	static std::optional<void*> GetOrCreateFunctionValue(const Method& method, PyObject* object);

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

		template<typename T>
		void* CreateValue(PyObject* pItem) {
			auto value = ValueFromObject<T>(pItem);
			if (value) {
				return new T(std::move(*value));
			}
			return nullptr;
		}

		void* CreateFunction(const Method& method, PyObject* pItem) {
			auto value = GetOrCreateFunctionValue(method, pItem);
			if (value) {
				return new uintptr_t(reinterpret_cast<uintptr_t>(*value));
			}
			return nullptr;
		}

		template<typename T>
		void* CreateArray(PyObject* pItem) {
			if constexpr (std::is_same_v<T, char>) {
				if (!PyUnicode_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "Tuple element must be an unicode.");
					return nullptr;
				}

				PyObject* const utf8Obj = PyUnicode_AsUTF8String(pItem);
				if (utf8Obj) {
					const char* utf8Value = PyBytes_AsString(utf8Obj);
					auto* array = new std::vector<char>(utf8Value, utf8Value + strlen(utf8Value));
					Py_DECREF(utf8Obj);
					return array;
				}
				else {
					PyErr_SetString(PyExc_TypeError, "Tuple element must be a string.");
					return nullptr;
				}
			}
			else if constexpr (std::is_same_v<T, char16_t>) {
				if (!PyUnicode_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "Tuple element must be an unicode.");
					return nullptr;
				}

				Py_ssize_t size = PyUnicode_GetLength(pItem);
				auto* array = new std::vector<char16_t>(static_cast<size_t>(size));
				PyUnicode_AsWideChar(pItem, reinterpret_cast<wchar_t*>(array->data()), size);
				return array;
			}
			else {
				if (!PyList_Check(pItem) && !PyTuple_Check(pItem)) {
					PyErr_SetString(PyExc_TypeError, "Tuple element must be an list or tuple.");
					return nullptr;
				}
				Py_ssize_t size = PySequence_Size(pItem);
				if (size == -1) {
					PyErr_SetString(PyExc_ValueError, "Sequence must have valid size.");
					return nullptr;
				}

				auto* array = new std::vector<T>(static_cast<size_t>(size));
				for (Py_ssize_t i = 0; i < size; ++i) {
					PyObject* const element = PySequence_GetItem(pItem, i);

					bool valid;
					if constexpr (std::is_same_v<T, char> || std::is_same_v<T, std::string>) {
						valid = PyUnicode_Check(pItem);
					}
					else if constexpr (std::is_same_v<T, bool>) {
						valid = PyBool_Check(element);
					}
					else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
						valid = PyFloat_Check(element);
					}
					else {
						valid = PyLong_Check(element);
					}

					if (valid) {
						if constexpr (std::is_same_v<T, uintptr_t>) {
							(*array)[i] = reinterpret_cast<T>(PyLong_AsVoidPtr(element));
						}
						else if constexpr (std::is_same_v<T, bool>) {
							(*array)[i] = (element == Py_True);
						}
						else if constexpr (std::is_same_v<T, float>) {
							(*array)[i] = static_cast<float>(PyFloat_AsDouble(element));
						}
						else if constexpr (std::is_same_v<T, double>) {
							(*array)[i] = PyFloat_AsDouble(element);
						}
						else if constexpr (std::is_same_v<T, int64_t>) {
							(*array)[i] = static_cast<T>(PyLong_AsLongLong(element));
						}
						else if constexpr (std::is_same_v<T, uint64_t>) {
							(*array)[i] = static_cast<T>(PyLong_AsUnsignedLongLong(element));
						}
						else if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint8_t>) {
							(*array)[i] = static_cast<T>(PyLong_AsUnsignedLong(element));
						}
						else {
							(*array)[i] = static_cast<T>(PyLong_AsLong(element));
						}
					}
					else {
						delete reinterpret_cast<std::vector<T>*>(array);
						Py_XDECREF(element);
						if constexpr (std::is_same_v<T, char> || std::is_same_v<T, char16_t> || std::is_same_v<T, std::string>) {
							PyErr_SetString(PyExc_TypeError, "List or tuple must be an unicode.");
						}
						else if constexpr (std::is_same_v<T, bool>) {
							PyErr_SetString(PyExc_TypeError, "List or tuple must be a bool.");
						}
						else if constexpr (std::is_same_v<T, float> || std::is_same_v<T, double>) {
							PyErr_SetString(PyExc_TypeError, "List or tuple must be a float.");
						}
						else {
							PyErr_SetString(PyExc_TypeError, "List or tuple must be a long.");
						}
						return nullptr;
					}
					Py_XDECREF(element);
				}
				return array;
			}
		}

		void* CreateStringArray(PyObject* pItem) {
			if (!PyList_Check(pItem) && !PyTuple_Check(pItem)) {
				PyErr_SetString(PyExc_TypeError, "Tuple element must be an list or tuple.");
				return nullptr;
			}
			Py_ssize_t size = PySequence_Size(pItem);
			auto* array = new std::vector<std::string>();
			array->reserve(static_cast<size_t>(size));
			for (Py_ssize_t i = 0; i < size; ++i) {
				PyObject* const element = PySequence_GetItem(pItem, i);
				if (PyUnicode_Check(element)) {
					PyObject* const utf8Obj = PyUnicode_AsUTF8String(pItem);
					if (utf8Obj) {
						array->emplace_back(PyBytes_AsString(utf8Obj));
						Py_DECREF(utf8Obj);
					}
					else {
						PyErr_SetString(PyExc_TypeError, "Tuple element must be a string.");
					}
				}
				else {
					delete reinterpret_cast<std::vector<std::string>*>(array);
					Py_XDECREF(element);
					PyErr_SetString(PyExc_TypeError, "List or tuple elements must be unicode");
					return nullptr;
				}
				Py_XDECREF(element);
			}
			return array;
		}

		void SetFallbackReturn(ValueType retType, const ReturnValue* ret, const Parameters* params) {
			switch (retType) {
			case ValueType::Void:
				break;
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
			case ValueType::Pointer:
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
			case ValueType::ArrayPointer:
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
			case ValueType::Pointer:
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
			case ValueType::ArrayPointer:
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
			default:
				// TODO: Log fail description
				std::terminate();
			}

			return false;
		}

		template<typename T>
		PyObject* CreatePyObject(T /*value*/) {
			static_assert(always_false_v<T>, "CreatePyObject specialization required");
			return nullptr;
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

		PyObject* ParamToObject(const Property& paramType, const Parameters* params, uint8_t index) {
			switch (paramType.type) {
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
			case ValueType::Pointer:
				return CreatePyObject(params->GetArgument<void*>(index));
			case ValueType::Float:
				return CreatePyObject(params->GetArgument<float>(index));
			case ValueType::Double:
				return CreatePyObject(params->GetArgument<double>(index));
			case ValueType::Function:
				return GetOrCreateFunctionObject(*(paramType.prototype.get()), params->GetArgument<void*>(index));
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
			case ValueType::ArrayPointer:
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
						PyObject* const arg = ParamToObject(method->paramTypes[index], params, params_start_index + index);
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

		std::tuple<bool, Function> CreateInternalCall(const std::shared_ptr<asmjit::JitRuntime>& jitRuntime, const Method& method, PyObject* func) {
			Function function(jitRuntime);
			void* const methodAddr = function.GetJitFunc(method, &InternalCall, reinterpret_cast<void*>(func));
			return { methodAddr != nullptr, std::move(function) };
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

			auto [result, function] = CreateInternalCall(jitRuntime, method, func);

			if (!result) {
				Py_DECREF(func);
				return MethodExportError{ std::format("{} (jit error: {})", method.name, function.GetError()) };
			}

			return MethodExportData{ std::move(function), func };
		}

		struct ArgsScope {
			DCCallVM* vm;
			std::vector<std::pair<void*, ValueType>> storage; // used to store array temp memory

			ArgsScope(uint8_t size) {
				vm = dcNewCallVM(4096);
				dcMode(vm, DC_CALL_C_DEFAULT);
				dcReset(vm);
				if (size) {
					storage.reserve(size);
				}
			}

			~ArgsScope() {
				for (auto& [ptr, type] : storage) {
					switch (type) {
					case ValueType::Invalid:
					case ValueType::Void:
						// Should not trigger!
						break;
					case ValueType::Bool: {
						delete reinterpret_cast<bool*>(ptr);
						break;
					}
					case ValueType::Char8: {
						delete reinterpret_cast<char*>(ptr);
						break;
					}
					case ValueType::Char16: {
						delete reinterpret_cast<char16_t*>(ptr);
						break;
					}
					case ValueType::Int8: {
						delete reinterpret_cast<int8_t*>(ptr);
						break;
					}
					case ValueType::Int16: {
						delete reinterpret_cast<int16_t*>(ptr);
						break;
					}
					case ValueType::Int32: {
						delete reinterpret_cast<int32_t*>(ptr);
						break;
					}
					case ValueType::Int64: {
						delete reinterpret_cast<int64_t*>(ptr);
						break;
					}
					case ValueType::UInt8: {
						delete reinterpret_cast<uint8_t*>(ptr);
						break;
					}
					case ValueType::UInt16: {
						delete reinterpret_cast<uint16_t*>(ptr);
						break;
					}
					case ValueType::UInt32: {
						delete reinterpret_cast<uint32_t*>(ptr);
						break;
					}
					case ValueType::UInt64: {
						delete reinterpret_cast<uint64_t*>(ptr);
						break;
					}
					case ValueType::Function:
					case ValueType::Pointer: {
						delete reinterpret_cast<uintptr_t*>(ptr);
						break;
					}
					case ValueType::Float: {
						delete reinterpret_cast<float*>(ptr);
						break;
					}
					case ValueType::Double: {
						delete reinterpret_cast<double*>(ptr);
						break;
					}
					case ValueType::String: {
						delete reinterpret_cast<std::string*>(ptr);
						break;
					}
					case ValueType::ArrayBool: {
						delete reinterpret_cast<std::vector<bool>*>(ptr);
						break;
					}
					case ValueType::ArrayChar8: {
						delete reinterpret_cast<std::vector<char>*>(ptr);
						break;
					}
					case ValueType::ArrayChar16: {
						delete reinterpret_cast<std::vector<char16_t>*>(ptr);
						break;
					}
					case ValueType::ArrayInt8: {
						delete reinterpret_cast<std::vector<int16_t>*>(ptr);
						break;
					}
					case ValueType::ArrayInt16: {
						delete reinterpret_cast<std::vector<int16_t>*>(ptr);
						break;
					}
					case ValueType::ArrayInt32: {
						delete reinterpret_cast<std::vector<int32_t>*>(ptr);
						break;
					}
					case ValueType::ArrayInt64: {
						delete reinterpret_cast<std::vector<int64_t>*>(ptr);
						break;
					}
					case ValueType::ArrayUInt8: {
						delete reinterpret_cast<std::vector<uint8_t>*>(ptr);
						break;
					}
					case ValueType::ArrayUInt16: {
						delete reinterpret_cast<std::vector<uint16_t>*>(ptr);
						break;
					}
					case ValueType::ArrayUInt32: {
						delete reinterpret_cast<std::vector<uint32_t>*>(ptr);
						break;
					}
					case ValueType::ArrayUInt64: {
						delete reinterpret_cast<std::vector<uint64_t>*>(ptr);
						break;
					}
					case ValueType::ArrayPointer: {
						delete reinterpret_cast<std::vector<uintptr_t>*>(ptr);
						break;
					}
					case ValueType::ArrayFloat: {
						delete reinterpret_cast<std::vector<float>*>(ptr);
						break;
					}
					case ValueType::ArrayDouble: {
						delete reinterpret_cast<std::vector<double>*>(ptr);
						break;
					}
					case ValueType::ArrayString: {
						delete reinterpret_cast<std::vector<std::string>*>(ptr);
						break;
					}
					default:
						puts("Unsupported types!");
						break;
					}
				}
				dcFree(vm);
			}
		};

		void ExternalCallNoArgs(const Method* method, void* addr, const Parameters* p, uint8_t count, const ReturnValue* ret) {
			const bool hasRet = method->retType.type > ValueType::LastPrimitive;

			ArgsScope a(hasRet);

			if (hasRet) {
				void* value;
				switch (method->retType.type) {
				case ValueType::String:
					value = new std::string();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayBool:
					value = new std::vector<bool>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayChar8:
					value = new std::vector<char>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayChar16:
					value = new std::vector<char16_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayInt8:
					value = new std::vector<int8_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayInt16:
					value = new std::vector<int16_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayInt32:
					value = new std::vector<int32_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayInt64:
					value = new std::vector<int64_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayUInt8:
					value = new std::vector<uint8_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayUInt16:
					value = new std::vector<uint16_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayUInt32:
					value = new std::vector<uint32_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayUInt64:
					value = new std::vector<uint64_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayPointer:
					value = new std::vector<uintptr_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayFloat:
					value = new std::vector<float>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayDouble:
					value = new std::vector<double>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayString:
					value = new std::vector<std::string>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				default:
					// Should not require storage
					break;
				}
			}

			switch (method->retType.type) {
			case ValueType::Invalid:
				break;
			case ValueType::Void:
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(Py_None);
				break;
			case ValueType::Bool: {
				bool val = dcCallBool(a.vm, addr);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Char8: {
				char val = dcCallChar(a.vm, addr);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Char16: {
				char16_t val = static_cast<char16_t>(dcCallShort(a.vm, addr));
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Int8: {
				int8_t val = dcCallChar(a.vm, addr);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Int16: {
				int16_t val = dcCallShort(a.vm, addr);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Int32: {
				int32_t val = dcCallInt(a.vm, addr);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Int64: {
				int64_t val = dcCallLongLong(a.vm, addr);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::UInt8: {
				uint8_t val = static_cast<uint8_t>(dcCallChar(a.vm, addr));
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::UInt16: {
				uint16_t val = static_cast<uint16_t>(dcCallShort(a.vm, addr));
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::UInt32: {
				uint32_t val = static_cast<uint32_t>(dcCallInt(a.vm, addr));
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::UInt64: {
				uint64_t val = static_cast<uint64_t>(dcCallLongLong(a.vm, addr));
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Function:
			case ValueType::Pointer: {
				uintptr_t val = reinterpret_cast<uintptr_t>(dcCallPointer(a.vm, addr));
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Float: {
				float val = dcCallFloat(a.vm, addr);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Double: {
				double val = dcCallDouble(a.vm, addr);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::String: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObject(*reinterpret_cast<std::string*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayBool: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<bool>(*reinterpret_cast<std::vector<bool>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayChar8: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<char>(*reinterpret_cast<std::vector<char>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayChar16: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<char16_t>(*reinterpret_cast<std::vector<char16_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayInt8: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<int8_t>(*reinterpret_cast<std::vector<int8_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayInt16: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<int16_t>(*reinterpret_cast<std::vector<int16_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayInt32: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<int32_t>(*reinterpret_cast<std::vector<int32_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayInt64: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<int64_t>(*reinterpret_cast<std::vector<int64_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayUInt8: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<uint8_t>(*reinterpret_cast<std::vector<uint8_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayUInt16: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<uint16_t>(*reinterpret_cast<std::vector<uint16_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayUInt32: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<uint32_t>(*reinterpret_cast<std::vector<uint32_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayUInt64: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<uint64_t>(*reinterpret_cast<std::vector<uint64_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayPointer: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<uintptr_t>(*reinterpret_cast<std::vector<uintptr_t>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayFloat: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<float>(*reinterpret_cast<std::vector<float>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayDouble: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<double>(*reinterpret_cast<std::vector<double>*>(std::get<0>(a.storage[0]))));
				break;
			}
			case ValueType::ArrayString: {
				dcCallVoid(a.vm, addr);
				ret->SetReturnPtr(CreatePyObjectList<std::string>(*reinterpret_cast<std::vector<std::string>*>(std::get<0>(a.storage[0]))));
				break;
			}
			default:
				puts("Unsupported types!");
				break;
			}
		}

		void ExternalCall(const Method* method, void* addr, const Parameters* p, uint8_t count, const ReturnValue* ret) {
			// PyObject* (MethodPyCall*)(PyObject* self, PyObject* args)
			const auto args = p->GetArgument<PyObject*>(1);

			if (!PyTuple_Check(args)) {
				std::string error(std::format("Function \"{}\" expects a tuple of arguments", method->funcName));
				PyErr_SetString(PyExc_TypeError, error.c_str());
				ret->SetReturnPtr(nullptr);
				return;
			}

			const auto paramCount = static_cast<uint8_t>(method->paramTypes.size());
			const Py_ssize_t size = PyTuple_Size(args);
			if (size != static_cast<Py_ssize_t>(paramCount)) {
				std::string error(std::format("Wrong number of parameters, {} when {} required.", size, paramCount));
				PyErr_SetString(PyExc_TypeError, error.c_str());
				ret->SetReturnPtr(nullptr);
				return;
			}

			const bool hasRet = method->retType.type > ValueType::LastPrimitive;

			ArgsScope a(hasRet ? paramCount + 1 : paramCount);

			/// prepare arguments

			void* value;
			Py_ssize_t refsCount = 0;

			if (hasRet) {
				switch (method->retType.type) {
				case ValueType::String:
					value = new std::string();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayBool:
					value = new std::vector<bool>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayChar8:
					value = new std::vector<char>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayChar16:
					value = new std::vector<char16_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayInt8:
					value = new std::vector<int8_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayInt16:
					value = new std::vector<int16_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayInt32:
					value = new std::vector<int32_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayInt64:
					value = new std::vector<int64_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayUInt8:
					value = new std::vector<uint8_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayUInt16:
					value = new std::vector<uint16_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayUInt32:
					value = new std::vector<uint32_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayUInt64:
					value = new std::vector<uint64_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayPointer:
					value = new std::vector<uintptr_t>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayFloat:
					value = new std::vector<float>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayDouble:
					value = new std::vector<double>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				case ValueType::ArrayString:
					value = new std::vector<std::string>();
					a.storage.emplace_back(value, method->retType.type);
					dcArgPointer(a.vm, value);
					break;
				default:
					// Should not require storage
					break;
				}
			}

			for (Py_ssize_t i = 0; i < size; ++i) {
				PyObject* pItem = PyTuple_GetItem(args, i);

				auto& param = method->paramTypes[i];
				// Pass by refs or values ?
				if (param.ref) {
					refsCount++;
					/// By references
					switch (param.type) {
					case ValueType::Invalid:
					case ValueType::Void:
						// Should not trigger!
						break;
					case ValueType::Bool: {
						value = CreateValue<bool>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Char8: {
						value = CreateValue<char>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Char16: {
						value = CreateValue<char16_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Int8: {
						value = CreateValue<int8_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Int16: {
						value = CreateValue<int16_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Int32: {
						value = CreateValue<int32_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Int64: {
						value = CreateValue<int64_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::UInt8: {
						value = CreateValue<uint8_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::UInt16: {
						value = CreateValue<uint16_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::UInt32: {
						value = CreateValue<uint32_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::UInt64: {
						value = CreateValue<uint64_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Pointer: {
						value = CreateValue<uintptr_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Float: {
						value = CreateValue<float>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Double: {
						value = CreateValue<double>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::String: {
						value = CreateValue<std::string>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Function: {
						value = CreateFunction(*(param.prototype.get()), pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayBool: {
						value = CreateArray<bool>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayChar8: {
						value = CreateArray<char>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayChar16: {
						value = CreateArray<char16_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayInt8: {
						value = CreateArray<int8_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayInt16: {
						value = CreateArray<int16_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayInt32: {
						value = CreateArray<int32_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayInt64: {
						value = CreateArray<int64_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayUInt8: {
						value = CreateArray<uint8_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayUInt16: {
						value = CreateArray<uint16_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayUInt32: {
						value = CreateArray<uint32_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayUInt64: {
						value = CreateArray<uint64_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayPointer: {
						value = CreateArray<uintptr_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayFloat: {
						value = CreateArray<float>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayDouble: {
						value = CreateArray<double>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayString: {
						value = CreateStringArray(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					default:
						puts("Unsupported types!");
						break;
					}
				}
				else {
					/// By values
					switch (param.type) {
					case ValueType::Invalid:
					case ValueType::Void:
						// Should not trigger!
						break;
					case ValueType::Bool: {
						auto boolVal = ValueFromObject<bool>(pItem);
						if (!boolVal.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgBool(a.vm, *boolVal);
						break;
					}
					case ValueType::Char8: {
						auto charVal = ValueFromObject<char>(pItem);
						if (!charVal.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgChar(a.vm, *charVal);
						break;
					}
					case ValueType::Char16: {
						auto wcharVal = ValueFromObject<char16_t>(pItem);
						if (!wcharVal.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgShort(a.vm, static_cast<short>(*wcharVal));
						break;
					}
					case ValueType::Int8: {
						auto int8Val = ValueFromObject<int8_t>(pItem);
						if (!int8Val.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgChar(a.vm, *int8Val);
						break;
					}
					case ValueType::Int16: {
						auto int16Val = ValueFromObject<int16_t>(pItem);
						if (!int16Val.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgShort(a.vm, *int16Val);
						break;
					}
					case ValueType::Int32: {
						auto int32Val = ValueFromObject<int32_t>(pItem);
						if (!int32Val.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgInt(a.vm, *int32Val);
						break;
					}
					case ValueType::Int64: {
						auto int64Val = ValueFromObject<int64_t>(pItem);
						if (!int64Val.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgLongLong(a.vm, *int64Val);
						break;
					}
					case ValueType::UInt8: {
						auto uint8Val = ValueFromObject<uint8_t>(pItem);
						if (!uint8Val.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgChar(a.vm, static_cast<int8_t>(*uint8Val));
						break;
					}
					case ValueType::UInt16: {
						auto uint16Val = ValueFromObject<uint16_t>(pItem);
						if (!uint16Val.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgShort(a.vm, static_cast<int16_t>(*uint16Val));
						break;
					}
					case ValueType::UInt32: {
						auto uint32Val = ValueFromObject<uint32_t>(pItem);
						if (!uint32Val.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgInt(a.vm, static_cast<int32_t>(*uint32Val));
						break;
					}
					case ValueType::UInt64: {
						auto uint64Val = ValueFromObject<uint64_t>(pItem);
						if (!uint64Val.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgLongLong(a.vm, static_cast<int64_t>(*uint64Val));
						break;
					}
					case ValueType::Pointer: {
						auto ptrVal = ValueFromObject<uintptr_t>(pItem);
						if (!ptrVal.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgPointer(a.vm, reinterpret_cast<void*>(*ptrVal));
						break;
					}
					case ValueType::Float: {
						auto floatVal = ValueFromObject<float>(pItem);
						if (!floatVal.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgFloat(a.vm, *floatVal);
						break;
					}
					case ValueType::Double: {
						auto doubleVal = ValueFromObject<double>(pItem);
						if (!doubleVal.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgDouble(a.vm, *doubleVal);
						break;
					}
					case ValueType::String: {
						value = CreateValue<std::string>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Function: {
						auto ptrVal = GetOrCreateFunctionValue(*(param.prototype.get()), pItem);
						if (!ptrVal.has_value()) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						dcArgPointer(a.vm, reinterpret_cast<void*>(*ptrVal));
						break;
					}
					case ValueType::ArrayBool: {
						value = CreateArray<bool>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayChar8: {
						value = CreateArray<char>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayChar16: {
						value = CreateArray<char16_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayInt8: {
						value = CreateArray<int8_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayInt16: {
						value = CreateArray<int16_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayInt32: {
						value = CreateArray<int32_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayInt64: {
						value = CreateArray<int64_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayUInt8: {
						value = CreateArray<uint8_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayUInt16: {
						value = CreateArray<uint16_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayUInt32: {
						value = CreateArray<uint32_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayUInt64: {
						value = CreateArray<uint64_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayPointer: {
						value = CreateArray<uintptr_t>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayFloat: {
						value = CreateArray<float>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayDouble: {
						value = CreateArray<double>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::ArrayString: {
						value = CreateStringArray(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					default:
						puts("Unsupported types!");
						break;
					}
				}
			}

			/// call function and set return

			PyObject* retObj = nullptr;

			switch (method->retType.type) {
			case ValueType::Invalid:
				break;
			case ValueType::Void:
				dcCallVoid(a.vm, addr);
				retObj = Py_None;
				break;
			case ValueType::Bool: {
				bool val = dcCallBool(a.vm, addr);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Char8: {
				char val = dcCallChar(a.vm, addr);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Char16: {
				char16_t val = static_cast<char16_t>(dcCallShort(a.vm, addr));
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Int8: {
				int8_t val = dcCallChar(a.vm, addr);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Int16: {
				int16_t val = dcCallShort(a.vm, addr);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Int32: {
				int32_t val = dcCallInt(a.vm, addr);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Int64: {
				int64_t val = dcCallLongLong(a.vm, addr);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::UInt8: {
				uint8_t val = static_cast<uint8_t>(dcCallChar(a.vm, addr));
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::UInt16: {
				uint16_t val = static_cast<uint16_t>(dcCallShort(a.vm, addr));
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::UInt32: {
				uint32_t val = static_cast<uint32_t>(dcCallInt(a.vm, addr));
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::UInt64: {
				uint64_t val = static_cast<uint64_t>(dcCallLongLong(a.vm, addr));
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Function:
			case ValueType::Pointer: {
				uintptr_t val = reinterpret_cast<uintptr_t>(dcCallPointer(a.vm, addr));
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Float: {
				float val = dcCallFloat(a.vm, addr);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Double: {
				double val = dcCallDouble(a.vm, addr);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::String: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObject(*reinterpret_cast<std::string*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayBool: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<bool>(*reinterpret_cast<std::vector<bool>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayChar8: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<char>(*reinterpret_cast<std::vector<char>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayChar16: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<char16_t>(*reinterpret_cast<std::vector<char16_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayInt8: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<int8_t>(*reinterpret_cast<std::vector<int8_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayInt16: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<int16_t>(*reinterpret_cast<std::vector<int16_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayInt32: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<int32_t>(*reinterpret_cast<std::vector<int32_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayInt64: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<int64_t>(*reinterpret_cast<std::vector<int64_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayUInt8: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<uint8_t>(*reinterpret_cast<std::vector<uint8_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayUInt16: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<uint16_t>(*reinterpret_cast<std::vector<uint16_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayUInt32: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<uint32_t>(*reinterpret_cast<std::vector<uint32_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayUInt64: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<uint64_t>(*reinterpret_cast<std::vector<uint64_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayPointer: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<uintptr_t>(*reinterpret_cast<std::vector<uintptr_t>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayFloat: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<float>(*reinterpret_cast<std::vector<float>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayDouble: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<double>(*reinterpret_cast<std::vector<double>*>(std::get<0>(a.storage[0])));
				break;
			}
			case ValueType::ArrayString: {
				dcCallVoid(a.vm, addr);
				retObj = CreatePyObjectList<std::string>(*reinterpret_cast<std::vector<std::string>*>(std::get<0>(a.storage[0])));
				break;
			}
			default:
				puts("Unsupported types!");
				break;
			}

			/// pull data from reference arguments back to python
			if (refsCount) {
				// return as tuple
				PyObject* retTuple = PyTuple_New(hasRet ? refsCount + 1 : refsCount);

				Py_ssize_t k = 0;

				if (hasRet) {
					PyTuple_SET_ITEM(retTuple, k++, retObj);
				}

				PyObject* pValue;
				for (Py_ssize_t i = 0, j = hasRet; i < size; ++i) {
					auto& param = method->paramTypes[i];
					if (param.ref) {
						switch (param.type) {
						case ValueType::Bool:
							pValue = CreatePyObject(*reinterpret_cast<bool*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Char8:
							pValue = CreatePyObject(*reinterpret_cast<char*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Char16:
							pValue = CreatePyObject(*reinterpret_cast<char16_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Int8:
							pValue = CreatePyObject(*reinterpret_cast<int8_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Int16:
							pValue = CreatePyObject(*reinterpret_cast<int16_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Int32:
							pValue = CreatePyObject(*reinterpret_cast<int32_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Int64:
							pValue = CreatePyObject(*reinterpret_cast<int64_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::UInt8:
							pValue = CreatePyObject(*reinterpret_cast<uint8_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::UInt16:
							pValue = CreatePyObject(*reinterpret_cast<uint16_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::UInt32:
							pValue = CreatePyObject(*reinterpret_cast<uint32_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::UInt64:
							pValue = CreatePyObject(*reinterpret_cast<uint64_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Float:
							pValue = CreatePyObject(*reinterpret_cast<float*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Double:
							pValue = CreatePyObject(*reinterpret_cast<double*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::String:
							pValue = CreatePyObject(*reinterpret_cast<std::string*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Function:
						case ValueType::Pointer:
							pValue = CreatePyObject(*reinterpret_cast<uintptr_t*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayBool:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<bool>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayChar8:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<char>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayChar16:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<char16_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayInt8:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<int8_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayInt16:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<int16_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayInt32:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<int32_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayInt64:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<int64_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayUInt8:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uint8_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayUInt16:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uint16_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayUInt32:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uint32_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayUInt64:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uint64_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayPointer:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uintptr_t>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayFloat:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<float>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayDouble:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<double>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayString:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<std::string>*>(std::get<0>(a.storage[j++])));
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						default:
							// TODO: Log fail description
							std::terminate();
						}
						if (k >= refsCount)
							break;
					}
				}

				ret->SetReturnPtr(retTuple);
			}
			else {
				// return as single object
				ret->SetReturnPtr(retObj);
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

			_ppsModule = PyImport_ImportModule("plugify.pps");
			if (!_ppsModule) {
				PyErr_Print();
				return ErrorData{ "Failed to import plugify.pps python module" };
			}

			return InitResultData{};
		}

		void Shutdown() override {
			if (Py_IsInitialized()) {
				if (_ppsModule) {
					Py_DECREF(_ppsModule);
					_ppsModule = nullptr;
				}

				for (const auto& data : _internalFunctions) {
					Py_DECREF(data.pythonFunction);
				}

				for (const auto& [_, object] : _externalMap) {
					Py_DECREF(object);
				}

				for (const auto& data : _pythonMethods) {
					Py_DECREF(data.pythonFunction);
				}

				for (const auto& [_, pluginData] : _pluginsMap) {
					Py_DECREF(pluginData._instance);
					Py_DECREF(pluginData._module);
				}

				Py_Finalize();
			}
			_internalMap.clear();
			_internalFunctions.clear();
			_externalMap.clear();
			_externalFunctions.clear();
			_moduleDefinitions.clear();
			_moduleMethods.clear();
			_moduleFunctions.clear();
			_pythonMethods.clear();
			_pluginsMap.clear();
			_jitRuntime.reset();
			_provider.reset();
		}

		void OnMethodExport(const IPlugin& plugin) override {
			if (_ppsModule) {
				PyObject* moduleObject = CreateInternalModule(plugin);
				if (!moduleObject) {
					moduleObject = CreateExternalModule(plugin);
				}
				if (moduleObject) {
					PyObject_SetAttrString(_ppsModule, plugin.GetName().c_str(), moduleObject);
					Py_DECREF(moduleObject);
					return;
				}
			}
			_provider->Log(std::format("[py3lm] Fail to export '{}' plugin methods", plugin.GetName()), Severity::Error);
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
				void* const methodAddr = methodData.jitFunction.GetFunction();
				methods.emplace_back(method.get().name, methodAddr);
				_internalMap.emplace(methodData.pythonFunction, methodAddr);
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

	public:
		PyObject* FindExternal(void* funcAddr) const {
			const auto it = _externalMap.find(funcAddr);
			if (it != _externalMap.end()) {
				return std::get<PyObject*>(*it);
			}
			return nullptr;
		}

		PyObject* GetOrCreateFunctionObject(const Method& method, void* funcAddr) {
			if (PyObject* const object = FindExternal(funcAddr)) {
				Py_INCREF(object);
				return object;
			}

			Function function(_jitRuntime);
			
			asmjit::FuncSignature sig(asmjit::CallConvId::kCDecl);
			sig.addArg(asmjit::TypeId::kUIntPtr);
			sig.addArg(asmjit::TypeId::kUIntPtr);
			sig.setRet(asmjit::TypeId::kUIntPtr);
			
			const bool noArgs = method.paramTypes.empty();
			
			void* const methodAddr = function.GetJitFunc(sig, method, noArgs ? &ExternalCallNoArgs : &ExternalCall, funcAddr);
			if (!methodAddr) {
				return nullptr;
			}
			
			auto defPtr = std::make_unique<PyMethodDef>();
			PyMethodDef& def = *(defPtr.get());
			def.ml_name = "PlugifyExternal";
			def.ml_meth = reinterpret_cast<PyCFunction>(methodAddr);
			def.ml_flags = noArgs ? METH_NOARGS : METH_VARARGS;
			def.ml_doc = nullptr;

			PyObject* const object = PyCFunction_New(defPtr.get(), nullptr);
			if (!object) {
				return nullptr;
			}

			_externalFunctions.emplace_back(std::move(function), std::move(defPtr));
			Py_INCREF(object);
			_externalMap.emplace(funcAddr, object);

			return object;
		}

		void* FindInternal(PyObject* object) const {
			const auto it = _internalMap.find(object);
			if (it != _internalMap.end()) {
				return std::get<void*>(*it);
			}
			return nullptr;
		}

		std::optional<void*> GetOrCreateFunctionValue(const Method& method, PyObject* object) {
			if (!PyFunction_Check(object)) {
				// TODO: set error
				return std::nullopt;
			}

			if (void* const funcAddr = FindInternal(object)) {
				return { funcAddr };
			}

			auto [result, function] = CreateInternalCall(_jitRuntime, method, object);
			if (!result) {
				// TODO: set error
				return std::nullopt;
			}

			void* const funcAddr = function.GetFunction();

			Py_INCREF(object);
			_internalFunctions.emplace_back(std::move(function), object);
			_internalMap.emplace(object, funcAddr);

			return { funcAddr };
		}

	private:
		PyObject* FindPythonMethod(void* addr) const {
			for (const auto& data : _pythonMethods) {
				if (data.jitFunction.GetFunction() == addr) {
					return data.pythonFunction;
				}
			}
			return nullptr;
		}

		PyObject* CreateInternalModule(const IPlugin& plugin) {
			if (!_pluginsMap.contains(plugin.GetName())) {
				return nullptr;
			}

			PyObject* moduleObject = PyModule_New(plugin.GetName().c_str());

			for (const auto& [name, addr] : plugin.GetMethods()) {
				for (const auto& method : plugin.GetDescriptor().exportedMethods) {
					if (name == method.name) {
						PyObject* const methodObject = FindPythonMethod(addr);
						if (!methodObject) {
							_provider->Log(std::format("[py3lm] Not found '{}' method while CreateInternalModule for '{}' plugin", name, plugin.GetName()), Severity::Fatal);
							std::terminate();
						}
						PyObject_SetAttrString(moduleObject, name.c_str(), methodObject);
						break;
					}
				}
			}
			
			return moduleObject;
		}

		PyObject* CreateExternalModule(const IPlugin& plugin) {
			auto& moduleMethods = _moduleMethods.emplace_back();

			for (const auto& [name, addr] : plugin.GetMethods()) {
				for (const auto& method : plugin.GetDescriptor().exportedMethods) {
					if (name == method.name) {
						Function function(_jitRuntime);

						asmjit::FuncSignature sig(asmjit::CallConvId::kCDecl);
						sig.addArg(asmjit::TypeId::kUIntPtr);
						sig.addArg(asmjit::TypeId::kUIntPtr);
						sig.setRet(asmjit::TypeId::kUIntPtr);

						const bool noArgs = method.paramTypes.empty();

						// Generate function --> PyObject* (MethodPyCall*)(PyObject* self, PyObject* args)
						void* const methodAddr = function.GetJitFunc(sig, method, noArgs ? &ExternalCallNoArgs : &ExternalCall, addr);
						if (!methodAddr)
							break;

						PyMethodDef& def = moduleMethods.emplace_back();
						def.ml_name = name.c_str();
						def.ml_meth = reinterpret_cast<PyCFunction>(methodAddr);
						def.ml_flags = noArgs ? METH_NOARGS : METH_VARARGS;
						def.ml_doc = nullptr;

						_moduleFunctions.emplace_back(std::move(function));
						break;
					}
				}
			}

			{
				PyMethodDef& def = moduleMethods.emplace_back();
				def.ml_name = nullptr;
				def.ml_meth = nullptr;
				def.ml_flags = 0;
				def.ml_doc = nullptr;
			}

			PyModuleDef& moduleDef = *(_moduleDefinitions.emplace_back(std::make_unique<PyModuleDef>()).get());
			moduleDef.m_base = PyModuleDef_HEAD_INIT;
			moduleDef.m_name = plugin.GetName().c_str();
			moduleDef.m_doc = nullptr;
			moduleDef.m_size = -1;
			moduleDef.m_methods = moduleMethods.data();
			moduleDef.m_slots = nullptr;
			moduleDef.m_traverse = nullptr;
			moduleDef.m_clear = nullptr;
			moduleDef.m_free = nullptr;

			return PyModule_Create(&moduleDef);
		}

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
		PyObject* _ppsModule = nullptr;
		std::vector<std::vector<PyMethodDef>> _moduleMethods;
		std::vector<std::unique_ptr<PyModuleDef>> _moduleDefinitions;
		std::vector<Function> _moduleFunctions;
		struct ExternalHolder {
			Function func;
			std::unique_ptr<PyMethodDef> def;
		};
		std::vector<ExternalHolder> _externalFunctions;
		std::unordered_map<void*, PyObject*> _externalMap;
		std::vector<PythonMethodData> _internalFunctions;
		std::unordered_map<PyObject*, void*> _internalMap;
	};

	Python3LanguageModule g_py3lm;

	extern "C"
	PY3LM_EXPORT ILanguageModule* GetLanguageModule() {
		return &g_py3lm;
	}

	static PyObject* GetOrCreateFunctionObject(const Method& method, void* funcAddr) {
		return g_py3lm.GetOrCreateFunctionObject(method, funcAddr);
	}

	static std::optional<void*> GetOrCreateFunctionValue(const Method& method, PyObject* object) {
		return g_py3lm.GetOrCreateFunctionValue(method, object);
	}
}
