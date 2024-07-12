#include "module.h"
#include <plugify/plugify_provider.h>
#include <plugify/compat_format.h>
#include <plugify/log.h>
#include <plugify/module.h>
#include <plugify/plugin_descriptor.h>
#include <plugify/plugin.h>
#include <plugify/math.h>
#include <module_export.h>
#include <dyncall/dyncall.h>
#include <cuchar>
#include <climits>
#include <array>

using namespace plugify;
namespace fs = std::filesystem;

namespace py3lm {
	extern Python3LanguageModule g_py3lm;

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
			PyErr_SetString(PyExc_TypeError, "Not boolean");
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
			PyErr_SetString(PyExc_TypeError, "Not string");
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
			PyErr_SetString(PyExc_TypeError, "Not string");
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
			PyErr_SetString(PyExc_TypeError, "Not integer");
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
			return ValueFromNumberObject<uint16_t, PyLong_AsUnsignedLong>(object);
		}

		template<>
		std::optional<uint32_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<uint32_t, PyLong_AsUnsignedLong>(object);
		}

		template<>
		std::optional<uint64_t> ValueFromObject(PyObject* object) {
			return ValueFromNumberObject<uint64_t, PyLong_AsUnsignedLongLong>(object);
		}

		template<>
		std::optional<void*> ValueFromObject(PyObject* object) {
			if (PyLong_Check(object)) {
				const auto result = PyLong_AsVoidPtr(object);
				if (!PyErr_Occurred()) {
					return result;
				}
			}
			PyErr_SetString(PyExc_TypeError, "Not integer");
			return std::nullopt;
		}

		template<>
		std::optional<float> ValueFromObject(PyObject* object) {
			if (PyFloat_Check(object)) {
				return static_cast<float>(PyFloat_AS_DOUBLE(object));
			}
			PyErr_SetString(PyExc_TypeError, "Not float");
			return std::nullopt;
		}

		template<>
		std::optional<double> ValueFromObject(PyObject* object) {
			if (PyFloat_Check(object)) {
				return PyFloat_AS_DOUBLE(object);
			}
			PyErr_SetString(PyExc_TypeError, "Not float");
			return std::nullopt;
		}

		template<>
		std::optional<std::string> ValueFromObject(PyObject* object) {
			if (PyUnicode_Check(object)) {
				Py_ssize_t size{};
				const char* const buffer = PyUnicode_AsUTF8AndSize(object, &size);
				return std::make_optional<std::string>(buffer, buffer + size);
			}
			PyErr_SetString(PyExc_TypeError, "Not string");
			return std::nullopt;
		}

		template<>
		std::optional<Vector2> ValueFromObject(PyObject* object) {
			return g_py3lm.Vector2ValueFromObject(object);
		}

		template<>
		std::optional<Vector3> ValueFromObject(PyObject* object) {
			return g_py3lm.Vector3ValueFromObject(object);
		}

		template<>
		std::optional<Vector4> ValueFromObject(PyObject* object) {
			return g_py3lm.Vector4ValueFromObject(object);
		}

		template<>
		std::optional<Matrix4x4> ValueFromObject(PyObject* object) {
			return g_py3lm.Matrix4x4ValueFromObject(object);
		}

		template<typename T>
		std::optional<std::vector<T>> ArrayFromObject(PyObject* arrayObject) {
			if (!PyList_Check(arrayObject)) {
				PyErr_SetString(PyExc_TypeError, "Not list");
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
			return array;
		}

		std::optional<void*> GetOrCreateFunctionValue(const Method& method, PyObject* object) {
			return g_py3lm.GetOrCreateFunctionValue(method, object);
		}

		template<typename T>
		void* CreateValue(PyObject* pItem) {
			auto value = ValueFromObject<T>(pItem);
			if (value) {
				return new T(std::move(*value));
			}
			return nullptr;
		}

		template<typename T>
		void* CreateArray(PyObject* pItem) {
			auto array = ArrayFromObject<T>(pItem);
			if (array) {
				return new std::vector<T>(std::move(*array));
			}
			return nullptr;
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
				ret->SetReturnPtr<void*>(nullptr);
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
			case ValueType::Vector2: {
				ret->SetReturnPtr<Vector2>({});
				break;
			}
#if PY3LM_PLATFORM_WINDOWS
			case ValueType::Vector3: {
				auto* const returnParam = params->GetArgument<Vector3*>(0);
				std::construct_at(returnParam);
				ret->SetReturnPtr<Vector3*>(returnParam);
				break;
			}
			case ValueType::Vector4: {
				auto* const returnParam = params->GetArgument<Vector4*>(0);
				std::construct_at(returnParam);
				ret->SetReturnPtr<Vector4*>(returnParam);
				break;
			}
#elif PY3LM_PLATFORM_LINUX || PY3LM_PLATFORM_APPLE
			case ValueType::Vector3: {
				ret->SetReturnPtr<Vector3>({});
				break;
			}
			case ValueType::Vector4: {
				ret->SetReturnPtr<Vector4>({});
				break;
			}
#endif // PY3LM_PLATFORM_WINDOWS
			case ValueType::Matrix4x4: {
				auto* const returnParam = params->GetArgument<Matrix4x4*>(0);
				std::construct_at(returnParam);
				ret->SetReturnPtr<Matrix4x4*>(returnParam);
				break;
			}
			default:
				// TODO: Log fail description
				std::terminate();
			}
		}

		bool SetReturn(PyObject* result, const Property& retType, const ReturnValue* ret, const Parameters* params) {
			switch (retType.type) {
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
				if (auto value = GetOrCreateFunctionValue(*(retType.prototype), result)) {
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
			case ValueType::Vector2:
				if (auto value = ValueFromObject<Vector2>(result)) {
					ret->SetReturnPtr<Vector2>(*value);
					return true;
				}
				break;
#if PY3LM_PLATFORM_WINDOWS
			case ValueType::Vector3:
				if (auto value = ValueFromObject<Vector3>(result)) {
					auto* const returnParam = params->GetArgument<Vector3*>(0);
					std::construct_at(returnParam, std::move(*value));
					ret->SetReturnPtr<Vector3*>(returnParam);
					return true;
				}
				break;
			case ValueType::Vector4:
				if (auto value = ValueFromObject<Vector4>(result)) {
					auto* const returnParam = params->GetArgument<Vector4*>(0);
					std::construct_at(returnParam, std::move(*value));
					ret->SetReturnPtr<Vector4*>(returnParam);
					return true;
				}
				break;
#elif PY3LM_PLATFORM_LINUX || PY3LM_PLATFORM_APPLE
			case ValueType::Vector3:
				if (auto value = ValueFromObject<Vector3>(result)) {
					ret->SetReturnPtr<Vector3>(*value);
					return true;
				}
				break;
			case ValueType::Vector4:
				if (auto value = ValueFromObject<Vector4>(result)) {
					ret->SetReturnPtr<Vector4>(*value);
					return true;
				}
				break;
#endif // PY3LM_PLATFORM_WINDOWS
			case ValueType::Matrix4x4:
				if (auto value = ValueFromObject<Matrix4x4>(result)) {
					auto* const returnParam = params->GetArgument<Matrix4x4*>(0);
					std::construct_at(returnParam, std::move(*value));
					ret->SetReturnPtr<Matrix4x4*>(returnParam);
					return true;
				}
				break;
			default:
				// TODO: Log fail description
				std::terminate();
			}

			return false;
		}

		bool SetRefParam(PyObject* object, const Property& paramType, const Parameters* params, uint8_t index) {
			switch (paramType.type) {
			case ValueType::Bool:
				if (auto value = ValueFromObject<bool>(object)) {
					auto* const param = params->GetArgument<bool*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Char8:
				if (auto value = ValueFromObject<char>(object)) {
					auto* const param = params->GetArgument<char*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Char16:
				if (auto value = ValueFromObject<char16_t>(object)) {
					auto* const param = params->GetArgument<char16_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Int8:
				if (auto value = ValueFromObject<int8_t>(object)) {
					auto* const param = params->GetArgument<int8_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Int16:
				if (auto value = ValueFromObject<int16_t>(object)) {
					auto* const param = params->GetArgument<int16_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Int32:
				if (auto value = ValueFromObject<int32_t>(object)) {
					auto* const param = params->GetArgument<int32_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Int64:
				if (auto value = ValueFromObject<int64_t>(object)) {
					auto* const param = params->GetArgument<int64_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::UInt8:
				if (auto value = ValueFromObject<uint8_t>(object)) {
					auto* const param = params->GetArgument<uint8_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::UInt16:
				if (auto value = ValueFromObject<uint16_t>(object)) {
					auto* const param = params->GetArgument<uint16_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::UInt32:
				if (auto value = ValueFromObject<uint32_t>(object)) {
					auto* const param = params->GetArgument<uint32_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::UInt64:
				if (auto value = ValueFromObject<uint64_t>(object)) {
					auto* const param = params->GetArgument<uint64_t*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Pointer:
				if (auto value = ValueFromObject<void*>(object)) {
					auto* const param = params->GetArgument<void**>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Float:
				if (auto value = ValueFromObject<float>(object)) {
					auto* const param = params->GetArgument<float*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Double:
				if (auto value = ValueFromObject<double>(object)) {
					auto* const param = params->GetArgument<double*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::String:
				if (auto value = ValueFromObject<std::string>(object)) {
					auto* const param = params->GetArgument<std::string*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayBool:
				if (auto value = ArrayFromObject<bool>(object)) {
					auto* const param = params->GetArgument<std::vector<bool>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayChar8:
				if (auto value = ArrayFromObject<char>(object)) {
					auto* const param = params->GetArgument<std::vector<char>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayChar16:
				if (auto value = ArrayFromObject<char16_t>(object)) {
					auto* const param = params->GetArgument<std::vector<char16_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayInt8:
				if (auto value = ArrayFromObject<int8_t>(object)) {
					auto* const param = params->GetArgument<std::vector<int8_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayInt16:
				if (auto value = ArrayFromObject<int16_t>(object)) {
					auto* const param = params->GetArgument<std::vector<int16_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayInt32:
				if (auto value = ArrayFromObject<int32_t>(object)) {
					auto* const param = params->GetArgument<std::vector<int32_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayInt64:
				if (auto value = ArrayFromObject<int64_t>(object)) {
					auto* const param = params->GetArgument<std::vector<int64_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayUInt8:
				if (auto value = ArrayFromObject<uint8_t>(object)) {
					auto* const param = params->GetArgument<std::vector<uint8_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayUInt16:
				if (auto value = ArrayFromObject<uint16_t>(object)) {
					auto* const param = params->GetArgument<std::vector<uint16_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayUInt32:
				if (auto value = ArrayFromObject<uint32_t>(object)) {
					auto* const param = params->GetArgument<std::vector<uint32_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayUInt64:
				if (auto value = ArrayFromObject<uint64_t>(object)) {
					auto* const param = params->GetArgument<std::vector<uint64_t>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayPointer:
				if (auto value = ArrayFromObject<void*>(object)) {
					auto* const param = params->GetArgument<std::vector<void*>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayFloat:
				if (auto value = ArrayFromObject<float>(object)) {
					auto* const param = params->GetArgument<std::vector<float>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayDouble:
				if (auto value = ArrayFromObject<double>(object)) {
					auto* const param = params->GetArgument<std::vector<double>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::ArrayString:
				if (auto value = ArrayFromObject<std::string>(object)) {
					auto* const param = params->GetArgument<std::vector<std::string>*>(index);
					*param = std::move(*value);
					return true;
				}
				break;
			case ValueType::Vector2:
				if (auto value = ValueFromObject<Vector2>(object)) {
					auto* const param = params->GetArgument<Vector2*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Vector3:
				if (auto value = ValueFromObject<Vector3>(object)) {
					auto* const param = params->GetArgument<Vector3*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Vector4:
				if (auto value = ValueFromObject<Vector4>(object)) {
					auto* const param = params->GetArgument<Vector4*>(index);
					*param = *value;
					return true;
				}
				break;
			case ValueType::Matrix4x4:
				if (auto value = ValueFromObject<Vector2>(object)) {
					auto* const param = params->GetArgument<Vector2*>(index);
					*param = *value;
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
			if (value == char{ 0 }) {
				return PyUnicode_FromStringAndSize(nullptr, Py_ssize_t{ 0 });
			}
			return PyUnicode_FromStringAndSize(&value, Py_ssize_t{ 1 });
		}

		template<>
		PyObject* CreatePyObject(char16_t value) {
			if (value == char16_t{ 0 }) {
				return PyUnicode_FromStringAndSize(nullptr, Py_ssize_t{ 0 });
			}
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

		template<>
		PyObject* CreatePyObject(Vector2 value) {
			return g_py3lm.CreateVector2Object(value);
		}

		template<>
		PyObject* CreatePyObject(Vector3 value) {
			return g_py3lm.CreateVector3Object(value);
		}

		template<>
		PyObject* CreatePyObject(Vector4 value) {
			return g_py3lm.CreateVector4Object(value);
		}

		template<>
		PyObject* CreatePyObject(Matrix4x4 value) {
			return g_py3lm.CreateMatrix4x4Object(value);
		}

		PyObject* GetOrCreateFunctionObject(const Method& method, void* funcAddr) {
			return g_py3lm.GetOrCreateFunctionObject(method, funcAddr);
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
			case ValueType::Vector2:
				return CreatePyObject(*(params->GetArgument<Vector2*>(index)));
			case ValueType::Vector3:
				return CreatePyObject(*(params->GetArgument<Vector3*>(index)));
			case ValueType::Vector4:
				return CreatePyObject(*(params->GetArgument<Vector4*>(index)));
			case ValueType::Matrix4x4:
				return CreatePyObject(*(params->GetArgument<Matrix4x4*>(index)));
			default:
				// TODO: Log fail description
				std::terminate();
				return nullptr;
			}
		}

		PyObject* ParamRefToObject(const Property& paramType, const Parameters* params, uint8_t index) {
			switch (paramType.type) {
			case ValueType::Bool:
				return CreatePyObject(*(params->GetArgument<bool*>(index)));
			case ValueType::Char8:
				return CreatePyObject(*(params->GetArgument<char*>(index)));
			case ValueType::Char16:
				return CreatePyObject(*(params->GetArgument<char16_t*>(index)));
			case ValueType::Int8:
				return CreatePyObject(*(params->GetArgument<int8_t*>(index)));
			case ValueType::Int16:
				return CreatePyObject(*(params->GetArgument<int16_t*>(index)));
			case ValueType::Int32:
				return CreatePyObject(*(params->GetArgument<int32_t*>(index)));
			case ValueType::Int64:
				return CreatePyObject(*(params->GetArgument<int64_t*>(index)));
			case ValueType::UInt8:
				return CreatePyObject(*(params->GetArgument<uint8_t*>(index)));
			case ValueType::UInt16:
				return CreatePyObject(*(params->GetArgument<uint16_t*>(index)));
			case ValueType::UInt32:
				return CreatePyObject(*(params->GetArgument<uint32_t*>(index)));
			case ValueType::UInt64:
				return CreatePyObject(*(params->GetArgument<uint64_t*>(index)));
			case ValueType::Pointer:
				return CreatePyObject(*(params->GetArgument<void**>(index)));
			case ValueType::Float:
				return CreatePyObject(*(params->GetArgument<float*>(index)));
			case ValueType::Double:
				return CreatePyObject(*(params->GetArgument<double*>(index)));
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
			case ValueType::Vector2:
				return CreatePyObject(*(params->GetArgument<Vector2*>(index)));
			case ValueType::Vector3:
				return CreatePyObject(*(params->GetArgument<Vector3*>(index)));
			case ValueType::Vector4:
				return CreatePyObject(*(params->GetArgument<Vector4*>(index)));
			case ValueType::Matrix4x4:
				return CreatePyObject(*(params->GetArgument<Matrix4x4*>(index)));
			default:
				// TODO: Log fail description
				std::terminate();
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

			uint8_t paramsCount = static_cast<uint8_t>(method->paramTypes.size());
			uint8_t refParamsCount = 0;
			uint8_t paramsStartIndex = ValueUtils::IsHiddenParam(method->retType.type) ? 1 : 0;

			PyObject* argTuple = nullptr;
			if (paramsCount) {
				argTuple = PyTuple_New(paramsCount);
				if (!argTuple) {
					processResult = ParamProcess::ErrorWithException;
					PyErr_SetString(PyExc_RuntimeError, "Fail to create arguments tuple");
				}
				else {
					for (uint8_t index = 0; index < paramsCount; ++index) {
						const auto& paramType = method->paramTypes[index];
						if (paramType.ref) {
							++refParamsCount;
						}
						using ParamConvertionFunc = PyObject* (*)(const Property&, const Parameters*, uint8_t);
						ParamConvertionFunc const convertFunc = paramType.ref ? &ParamRefToObject : &ParamToObject;
						PyObject* const arg = convertFunc(paramType, params, paramsStartIndex + index);
						if (!arg) {
							// convertFunc may set error
							processResult = PyErr_Occurred() ? ParamProcess::ErrorWithException : ParamProcess::Error;
							break;
						}
						if (PyTuple_SetItem(argTuple, index, arg) != 0) {
							Py_DECREF(arg);
							// PyTuple_SetItem set error
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

			const bool hasRefParams = refParamsCount != 0;

			PyObject* const result = PyObject_CallObject(func, argTuple);

			if (argTuple) {
				Py_DECREF(argTuple);
			}

			if (!result) {
				PyErr_Print();

				SetFallbackReturn(method->retType.type, ret, params);

				return;
			}

			if (hasRefParams) {
				if (!PyTuple_CheckExact(result)) {
					PyErr_SetString(PyExc_TypeError, "Returned value not tuple");

					PyErr_Print();

					Py_DECREF(result);

					SetFallbackReturn(method->retType.type, ret, params);

					return;
				}
				const Py_ssize_t tupleSize = PyTuple_Size(result);
				if (tupleSize != static_cast<Py_ssize_t>(1 + refParamsCount)) {
					const std::string error(std::format("Returned tuple wrong size {}, expected {}", tupleSize, static_cast<Py_ssize_t>(1 + refParamsCount)));
					PyErr_SetString(PyExc_TypeError, error.c_str());

					PyErr_Print();

					Py_DECREF(result);

					SetFallbackReturn(method->retType.type, ret, params);

					return;
				}
			}

			PyObject* const returnObject = hasRefParams ? PyTuple_GET_ITEM(result, Py_ssize_t{ 0 }) : result;

			if (hasRefParams) {
				for (uint8_t index = 0, k = 0; index < paramsCount; ++index) {
					const auto& paramType = method->paramTypes[index];
					if (!paramType.ref) {
						continue;
					}
					if (!SetRefParam(PyTuple_GET_ITEM(result, Py_ssize_t{ 1 + k }), paramType, params, paramsStartIndex + index)) {
						// SetRefParam may set error
						if (PyErr_Occurred()) {
							PyErr_Print();
						}
					}
					++k;
					if (k == refParamsCount) {
						break;
					}
				}
			}

			if (!SetReturn(returnObject, method->retType, ret, params)) {
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
			DCaggr* ag = nullptr;

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
					case ValueType::Vector2: {
						delete reinterpret_cast<Vector2*>(ptr);
						break;
					}
					case ValueType::Vector3: {
						delete reinterpret_cast<Vector3*>(ptr);
						break;
					}
					case ValueType::Vector4: {
						delete reinterpret_cast<Vector4*>(ptr);
						break;
					}
					case ValueType::Matrix4x4: {
						delete reinterpret_cast<Matrix4x4*>(ptr);
						break;
					}
					default:
						puts("Unsupported types!");
						std::terminate();
						break;
					}
				}
				if (ag) {
					dcFreeAggr(ag);
				}
				dcFree(vm);
			}
		};

		void ExternalCallNoArgs(const Method* method, void* addr, const Parameters* p, uint8_t count, const ReturnValue* ret) {
			ArgsScope a(1);

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
			case ValueType::Vector2:
				a.ag = dcNewAggr(2, sizeof(Vector2));
				for (int i = 0; i < 2; ++i) {
					dcAggrField(a.ag, DC_SIGCHAR_FLOAT, static_cast<int>(sizeof(float) * i), 1);
				}
				dcCloseAggr(a.ag);
				dcBeginCallAggr(a.vm, a.ag);
				break;
			case ValueType::Vector3:
				a.ag = dcNewAggr(3, sizeof(Vector3));
				for (int i = 0; i < 3; ++i) {
					dcAggrField(a.ag, DC_SIGCHAR_FLOAT, static_cast<int>(sizeof(float) * i), 1);
				}
				dcCloseAggr(a.ag);
				dcBeginCallAggr(a.vm, a.ag);
				break;
			case ValueType::Vector4:
				a.ag = dcNewAggr(4, sizeof(Vector4));
				for (int i = 0; i < 4; ++i) {
					dcAggrField(a.ag, DC_SIGCHAR_FLOAT, static_cast<int>(sizeof(float) * i), 1);
				}
				dcCloseAggr(a.ag);
				dcBeginCallAggr(a.vm, a.ag);
				break;
			case ValueType::Matrix4x4:
				a.ag = dcNewAggr(16, sizeof(Matrix4x4));
				for (int i = 0; i < 16; ++i) {
					dcAggrField(a.ag, DC_SIGCHAR_FLOAT, static_cast<int>(sizeof(float) * i), 1);
				}
				dcCloseAggr(a.ag);
				dcBeginCallAggr(a.vm, a.ag);
				break;
			default:
				// Should not require storage
				break;
			}

			switch (method->retType.type) {
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
			case ValueType::Function: {
				void* val = dcCallPointer(a.vm, addr);
				ret->SetReturnPtr(GetOrCreateFunctionObject(*(method->retType.prototype.get()), val));
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
			case ValueType::Vector2: {
				Vector2 val;
				dcCallAggr(a.vm, addr, a.ag, &val);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Vector3: {
				Vector3 val;
				dcCallAggr(a.vm, addr, a.ag, &val);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Vector4: {
				Vector4 val;
				dcCallAggr(a.vm, addr, a.ag, &val);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			case ValueType::Matrix4x4: {
				Matrix4x4 val;
				dcCallAggr(a.vm, addr, a.ag, &val);
				ret->SetReturnPtr(CreatePyObject(val));
				break;
			}
			default:
				const std::string error(std::format("Return unsupported type {:#x}", static_cast<uint8_t>(method->retType.type)));
				PyErr_SetString(PyExc_TypeError, error.c_str());
				ret->SetReturnPtr(nullptr);
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

			const Py_ssize_t paramsStartIndex = plugify::ValueUtils::IsObject(method->retType.type) ? 1 : 0;

			ArgsScope a(1 + paramCount);

			/// prepare arguments

			void* value;
			Py_ssize_t refsCount = 0;

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
			case ValueType::Vector2:
				a.ag = dcNewAggr(2, sizeof(Vector2));
				for (int i = 0; i < 2; ++i) {
					dcAggrField(a.ag, DC_SIGCHAR_FLOAT, static_cast<int>(sizeof(float) * i), 1);
				}
				dcCloseAggr(a.ag);
				dcBeginCallAggr(a.vm, a.ag);
				break;
			case ValueType::Vector3:
				a.ag = dcNewAggr(3, sizeof(Vector3));
				for (int i = 0; i < 3; ++i) {
					dcAggrField(a.ag, DC_SIGCHAR_FLOAT, static_cast<int>(sizeof(float) * i), 1);
				}
				dcCloseAggr(a.ag);
				dcBeginCallAggr(a.vm, a.ag);
				break;
			case ValueType::Vector4:
				a.ag = dcNewAggr(4, sizeof(Vector4));
				for (int i = 0; i < 4; ++i) {
					dcAggrField(a.ag, DC_SIGCHAR_FLOAT, static_cast<int>(sizeof(float) * i), 1);
				}
				dcCloseAggr(a.ag);
				dcBeginCallAggr(a.vm, a.ag);
				break;
			case ValueType::Matrix4x4:
				a.ag = dcNewAggr(16, sizeof(Matrix4x4));
				for (int i = 0; i < 16; ++i) {
					dcAggrField(a.ag, DC_SIGCHAR_FLOAT, static_cast<int>(sizeof(float) * i), 1);
				}
				dcCloseAggr(a.ag);
				dcBeginCallAggr(a.vm, a.ag);
				break;
			default:
				// Should not require storage
				break;
			}

			for (Py_ssize_t i = 0; i < size; ++i) {
				PyObject* pItem = PyTuple_GetItem(args, i);

				auto& param = method->paramTypes[i];
				// Pass by refs or values ?
				if (param.ref) {
					refsCount++;
					/// By references
					switch (param.type) {
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
						value = CreateArray<std::string>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Vector2: {
						value = CreateValue<Vector2>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Vector3: {
						value = CreateValue<Vector3>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Vector4: {
						value = CreateValue<Vector4>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Matrix4x4: {
						value = CreateValue<Matrix4x4>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					default: {
						const std::string error(std::format("Param {} unsupported type {:#x}", i + 1, static_cast<uint8_t>(param.type)));
						PyErr_SetString(PyExc_TypeError, error.c_str());
						ret->SetReturnPtr(nullptr);
						return;
					}
					}
				}
				else {
					/// By values
					switch (param.type) {
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
						value = CreateArray<std::string>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Vector2: {
						value = CreateValue<Vector2>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Vector3: {
						value = CreateValue<Vector3>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Vector4: {
						value = CreateValue<Vector4>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					case ValueType::Matrix4x4: {
						value = CreateValue<Matrix4x4>(pItem);
						if (!value) {
							ret->SetReturnPtr(nullptr);
							return;
						}
						a.storage.emplace_back(value, param.type);
						dcArgPointer(a.vm, value);
						break;
					}
					default: {
						const std::string error(std::format("Param {} unsupported type {:#x}", i + 1, static_cast<uint8_t>(param.type)));
						PyErr_SetString(PyExc_TypeError, error.c_str());
						ret->SetReturnPtr(nullptr);
						return;
					}
					}
				}
			}

			/// call function and set return

			PyObject* retObj = nullptr;

			switch (method->retType.type) {
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
			case ValueType::Function: {
				void* val = dcCallPointer(a.vm, addr);
				retObj = GetOrCreateFunctionObject(*(method->retType.prototype.get()), val);
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
			case ValueType::Vector2: {
				Vector2 val;
				dcCallAggr(a.vm, addr, a.ag, &val);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Vector3: {
				Vector3 val;
				dcCallAggr(a.vm, addr, a.ag, &val);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Vector4: {
				Vector4 val;
				dcCallAggr(a.vm, addr, a.ag, &val);
				retObj = CreatePyObject(val);
				break;
			}
			case ValueType::Matrix4x4: {
				Matrix4x4 val;
				dcCallAggr(a.vm, addr, a.ag, &val);
				retObj = CreatePyObject(val);
				break;
			}
			default: {
				const std::string error(std::format("Return unsupported type {:#x}", static_cast<uint8_t>(method->retType.type)));
				PyErr_SetString(PyExc_TypeError, error.c_str());
				ret->SetReturnPtr(nullptr);
				break;
			}
			}

			/// pull data from reference arguments back to python
			if (refsCount) {
				// return as tuple
				PyObject* retTuple = PyTuple_New(1 + refsCount);

				Py_ssize_t k = 0;

				PyTuple_SET_ITEM(retTuple, k++, retObj);

				PyObject* pValue;
				for (Py_ssize_t i = 0, j = paramsStartIndex; i < size; ++i) {
					auto& param = method->paramTypes[i];
					if (param.ref) {
						switch (param.type) {
						case ValueType::Bool:
							pValue = CreatePyObject(*reinterpret_cast<bool*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Char8:
							pValue = CreatePyObject(*reinterpret_cast<char*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Char16:
							pValue = CreatePyObject(*reinterpret_cast<char16_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Int8:
							pValue = CreatePyObject(*reinterpret_cast<int8_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Int16:
							pValue = CreatePyObject(*reinterpret_cast<int16_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Int32:
							pValue = CreatePyObject(*reinterpret_cast<int32_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Int64:
							pValue = CreatePyObject(*reinterpret_cast<int64_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::UInt8:
							pValue = CreatePyObject(*reinterpret_cast<uint8_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::UInt16:
							pValue = CreatePyObject(*reinterpret_cast<uint16_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::UInt32:
							pValue = CreatePyObject(*reinterpret_cast<uint32_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::UInt64:
							pValue = CreatePyObject(*reinterpret_cast<uint64_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Float:
							pValue = CreatePyObject(*reinterpret_cast<float*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Double:
							pValue = CreatePyObject(*reinterpret_cast<double*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::String:
							pValue = CreatePyObject(*reinterpret_cast<std::string*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Pointer:
							pValue = CreatePyObject(*reinterpret_cast<uintptr_t*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayBool:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<bool>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayChar8:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<char>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayChar16:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<char16_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayInt8:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<int8_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayInt16:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<int16_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayInt32:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<int32_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayInt64:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<int64_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayUInt8:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uint8_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayUInt16:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uint16_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayUInt32:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uint32_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayUInt64:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uint64_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayPointer:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<uintptr_t>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayFloat:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<float>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayDouble:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<double>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::ArrayString:
							pValue = CreatePyObjectList(*reinterpret_cast<std::vector<std::string>*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Vector2:
							pValue = CreatePyObject(*reinterpret_cast<Vector2*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Vector3:
							pValue = CreatePyObject(*reinterpret_cast<Vector3*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Vector4:
							pValue = CreatePyObject(*reinterpret_cast<Vector4*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						case ValueType::Matrix4x4:
							pValue = CreatePyObject(*reinterpret_cast<Matrix4x4*>(std::get<0>(a.storage[j++])));
							if (!pValue) {
								std::terminate();
							}
							PyTuple_SET_ITEM(retTuple, k++, pValue);
							break;
						default:
							// TODO: Log fail description
							std::terminate();
						}
						if (k >= refsCount + 1)
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

		template<typename T>
		std::optional<T> GetObjectAttrAsValue(PyObject* object, const char* attr_name) {
			PyObject* const attrObject = PyObject_GetAttrString(object, attr_name);
			if (!attrObject) {
				// PyObject_GetAttrString set error. e.g. AttributeError
				return std::nullopt;
			}
			const auto value = ValueFromObject<T>(attrObject);
			// ValueFromObject set error. e.g. TypeError
			Py_DECREF(attrObject);
			return value;
		}
	}

	Python3LanguageModule::Python3LanguageModule() = default;

	Python3LanguageModule::~Python3LanguageModule() = default;

	InitResult Python3LanguageModule::Initialize(std::weak_ptr<IPlugifyProvider> provider, const IModule& module) {
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

		_PluginTypeObject = PyObject_GetAttrString(plugifyPluginModule, "Plugin");
		if (!_PluginTypeObject) {
			Py_DECREF(plugifyPluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to find plugify.plugin.Plugin type" };
		}
		_PluginInfoTypeObject = PyObject_GetAttrString(plugifyPluginModule, "PluginInfo");
		if (!_PluginInfoTypeObject) {
			Py_DECREF(plugifyPluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to find plugify.plugin.PluginInfo type" };
		}

		_Vector2TypeObject = PyObject_GetAttrString(plugifyPluginModule, "Vector2");
		if (!_Vector2TypeObject) {
			Py_DECREF(plugifyPluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to find plugify.plugin.Vector2 type" };
		}
		_Vector3TypeObject = PyObject_GetAttrString(plugifyPluginModule, "Vector3");
		if (!_Vector3TypeObject) {
			Py_DECREF(plugifyPluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to find plugify.plugin.Vector3 type" };
		}
		_Vector4TypeObject = PyObject_GetAttrString(plugifyPluginModule, "Vector4");
		if (!_Vector4TypeObject) {
			Py_DECREF(plugifyPluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to find plugify.plugin.Vector4 type" };
		}
		_Matrix4x4TypeObject = PyObject_GetAttrString(plugifyPluginModule, "Matrix4x4");
		if (!_Matrix4x4TypeObject) {
			Py_DECREF(plugifyPluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to find plugify.plugin.Matrix4x4 type" };
		}

		Py_DECREF(plugifyPluginModule);

		_ppsModule = PyImport_ImportModule("plugify.pps");
		if (!_ppsModule) {
			PyErr_Print();
			return ErrorData{ "Failed to import plugify.pps python module" };
		}

		return InitResultData{};
	}

	void Python3LanguageModule::Shutdown() {
		if (Py_IsInitialized()) {
			if (_ppsModule) {
				Py_DECREF(_ppsModule);
			}

			if (_Vector2TypeObject) {
				Py_DECREF(_Vector2TypeObject);
			}

			if (_Vector3TypeObject) {
				Py_DECREF(_Vector3TypeObject);
			}

			if (_Vector4TypeObject) {
				Py_DECREF(_Vector4TypeObject);
			}

			if (_Matrix4x4TypeObject) {
				Py_DECREF(_Matrix4x4TypeObject);
			}

			if (_PluginTypeObject) {
				Py_DECREF(_PluginTypeObject);
			}

			if (_PluginInfoTypeObject) {
				Py_DECREF(_PluginInfoTypeObject);
			}

			for (const auto& data : _internalFunctions) {
				Py_DECREF(data.pythonFunction);
			}

			for (const auto& [_1, _2, object] : _externalFunctions) {
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
		_ppsModule = nullptr;
		_Vector2TypeObject = nullptr;
		_Vector3TypeObject = nullptr;
		_Vector4TypeObject = nullptr;
		_Matrix4x4TypeObject = nullptr;
		_PluginTypeObject = nullptr;
		_PluginInfoTypeObject = nullptr;
		_internalMap.clear();
		_externalMap.clear();
		_internalFunctions.clear();
		_externalFunctions.clear();
		_moduleDefinitions.clear();
		_moduleMethods.clear();
		_moduleFunctions.clear();
		_pythonMethods.clear();
		_pluginsMap.clear();
		_jitRuntime.reset();
		_provider.reset();
	}

	void Python3LanguageModule::OnMethodExport(const IPlugin& plugin) {
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

	LoadResult Python3LanguageModule::OnPluginLoad(const IPlugin& plugin) {
		const std::string& entryPoint = plugin.GetDescriptor().entryPoint;
		if (entryPoint.empty()) {
			return ErrorData{ "Incorrect entry point: empty" };
		}
		if (entryPoint.find_first_of("/\\") != std::string::npos) {
			return ErrorData{ "Incorrect entry point: contains '/' or '\\'" };
		}
		const std::string::size_type lastDotPos = entryPoint.find_last_of('.');
		if (lastDotPos == std::string::npos) {
			return ErrorData{ "Incorrect entry point: not have any dot '.' character" };
		}
		std::string_view className(entryPoint.begin() + (lastDotPos + 1), entryPoint.end());
		if (className.empty()) {
			return ErrorData{ "Incorrect entry point: empty class name part" };
		}
		std::string_view modulePathRel(entryPoint.begin(), entryPoint.begin() + lastDotPos);
		if (modulePathRel.empty()) {
			return ErrorData{ "Incorrect entry point: empty module path part" };
		}

		const fs::path& baseFolder = plugin.GetBaseDir();
		auto modulePath = std::string(modulePathRel);
		ReplaceAll(modulePath, ".", { static_cast<char>(fs::path::preferred_separator) });
		fs::path filePathRelative = modulePath;
		filePathRelative.replace_extension(".py");
		const fs::path filePath = baseFolder / filePathRelative;
		std::error_code ec;
		if (!fs::exists(filePath, ec) || !fs::is_regular_file(filePath, ec)) {
			return ErrorData{ std::format("Module file '{}' not exist", filePath.string()) };
		}
		const fs::path pluginsFolder = baseFolder.parent_path();
		filePathRelative = fs::relative(filePath, pluginsFolder, ec);
		filePathRelative.replace_extension();
		std::string moduleName = filePathRelative.generic_string();
		ReplaceAll(moduleName, "/", ".");

		_provider->Log(std::format("[py3lm] Load plugin module '{}'", moduleName), Severity::Verbose);

		PyObject* const pluginModule = PyImport_ImportModule(moduleName.c_str());
		if (!pluginModule) {
			PyErr_Print();
			return ErrorData{ std::format("Failed to import {} module", moduleName) };
		}

		PyObject* const classNameString = PyUnicode_FromStringAndSize(className.data(), static_cast<Py_ssize_t>(className.size()));
		if (!classNameString) {
			Py_DECREF(pluginModule);
			return ErrorData{ "Allocate class name string failed" };
		}

		PyObject* const pluginClass = PyObject_GetAttr(pluginModule, classNameString);
		if (!pluginClass) {
			Py_DECREF(classNameString);
			Py_DECREF(pluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to find plugin class" };
		}

		const int typeResult = PyObject_IsSubclass(pluginClass, _PluginTypeObject);
		if (typeResult != 1) {
			Py_DECREF(pluginClass);
			Py_DECREF(classNameString);
			Py_DECREF(pluginModule);
			PyErr_Print();
			return ErrorData{ std::format("Class '{}' not subclass of Plugin", className) };
		}

		PyObject* const pluginInstance = PyObject_CallNoArgs(pluginClass);
		Py_DECREF(pluginClass);
		if (!pluginInstance) {
			Py_DECREF(classNameString);
			Py_DECREF(pluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to create plugin instance" };
		}

		PyObject* const args = PyTuple_New(Py_ssize_t{ 2 });
		if (!args) {
			Py_DECREF(pluginInstance);
			Py_DECREF(classNameString);
			Py_DECREF(pluginModule);
			return ErrorData{ "Failed to save instance: arguments tuple is null" };
		}

		PyTuple_SET_ITEM(args, Py_ssize_t{ 0 }, classNameString); // classNameString ref taken by list
		Py_INCREF(pluginInstance);
		PyTuple_SET_ITEM(args, Py_ssize_t{ 1 }, pluginInstance); // pluginInstance ref taken by list

		PyObject* const pluginInfo = PyObject_CallObject(_PluginInfoTypeObject, args);
		Py_DECREF(args);
		if (!pluginInfo) {
			Py_DECREF(pluginInstance);
			Py_DECREF(pluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to save instance: plugin info not constructed" };
		}

		const int resultCode = PyObject_SetAttrString(pluginModule, "__plugin__", pluginInfo);
		Py_DECREF(pluginInfo);
		if (resultCode != 0) {
			Py_DECREF(pluginInstance);
			Py_DECREF(pluginModule);
			PyErr_Print();
			return ErrorData{ "Failed to save instance: assignment fail" };
		}

		if (_pluginsMap.contains(plugin.GetName())) {
			Py_DECREF(pluginInstance);
			Py_DECREF(pluginModule);
			return ErrorData{ "Plugin name duplicate" };
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
			AddToFunctionsMap(methodAddr, methodData.pythonFunction);
			_pythonMethods.emplace_back(std::move(methodData));
		}

		return LoadResultData{ std::move(methods) };
	}

	void Python3LanguageModule::OnPluginStart(const IPlugin& plugin) {
		TryCallPluginMethodNoArgs(plugin, "plugin_start", "OnPluginStart");
	}

	void Python3LanguageModule::OnPluginEnd(const IPlugin& plugin) {
		TryCallPluginMethodNoArgs(plugin, "plugin_end", "OnPluginEnd");
	}

	PyObject* Python3LanguageModule::FindExternal(void* funcAddr) const {
		const auto it = _externalMap.find(funcAddr);
		if (it != _externalMap.end()) {
			return std::get<PyObject*>(*it);
		}
		return nullptr;
	}

	void* Python3LanguageModule::FindInternal(PyObject* object) const {
		const auto it = _internalMap.find(object);
		if (it != _internalMap.end()) {
			return std::get<void*>(*it);
		}
		return nullptr;
	}

	void Python3LanguageModule::AddToFunctionsMap(void* funcAddr, PyObject* object) {
		_externalMap.emplace(funcAddr, object);
		_internalMap.emplace(object, funcAddr);
	}

	PyObject* Python3LanguageModule::GetOrCreateFunctionObject(const Method& method, void* funcAddr) {
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
			const std::string error(std::format("Lang module JIT failed to generate c++ PyCFunction wrapper '{}'", function.GetError()));
			PyErr_SetString(PyExc_RuntimeError, error.c_str());
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
			PyErr_SetString(PyExc_RuntimeError, "Fail to create function object from function pointer");
			return nullptr;
		}

		Py_INCREF(object);
		_externalFunctions.emplace_back(std::move(function), std::move(defPtr), object);
		AddToFunctionsMap(funcAddr, object);

		return object;
	}

	std::optional<void*> Python3LanguageModule::GetOrCreateFunctionValue(const Method& method, PyObject* object) {
		if (object == Py_None) {
			return { nullptr };
		}

		if (!PyFunction_Check(object)) {
			PyErr_SetString(PyExc_TypeError, "Not function");
			return std::nullopt;
		}

		if (void* const funcAddr = FindInternal(object)) {
			return { funcAddr };
		}

		auto [result, function] = CreateInternalCall(_jitRuntime, method, object);
		if (!result) {
			const std::string error(std::format("Lang module JIT failed to generate C++ wrapper from function object '{}'", function.GetError()));
			PyErr_SetString(PyExc_RuntimeError, error.c_str());
			return std::nullopt;
		}

		void* const funcAddr = function.GetFunction();

		Py_INCREF(object);
		_internalFunctions.emplace_back(std::move(function), object);
		AddToFunctionsMap(funcAddr, object);

		return { funcAddr };
	}

	PyObject* Python3LanguageModule::CreateVector2Object(const Vector2& vector) {
		PyObject* const args = PyTuple_New(Py_ssize_t{ 2 });
		if (!args) {
			PyErr_SetString(PyExc_RuntimeError, "Fail to create arguments tuple");
			return nullptr;
		}
		// CreatePyObject set error
		PyObject* const xObject = CreatePyObject(vector.x);
		if (!xObject) {
			Py_DECREF(args);
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 0 }, xObject); // xObject ref taken by tuple
		PyObject* const yObject = CreatePyObject(vector.y);
		if (!yObject) {
			Py_DECREF(args);
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 1 }, yObject); // yObject ref taken by tuple
		PyObject* const vectorObject = PyObject_CallObject(_Vector2TypeObject, args);
		Py_DECREF(args);
		return vectorObject;
	}

	std::optional<Vector2> Python3LanguageModule::Vector2ValueFromObject(PyObject* object) {
		const int typeResult = PyObject_IsInstance(object, _Vector2TypeObject);
		if (typeResult == -1) {
			// Python exception was set by PyObject_IsInstance
			return std::nullopt;
		}
		if (typeResult == 0) {
			PyErr_SetString(PyExc_TypeError, "Not Vector2");
			return std::nullopt;
		}
		auto xValue = GetObjectAttrAsValue<float>(object, "x");
		if (!xValue) {
			// GetObjectAttrAsValue set error. e.g. AttributeError, TypeError, ValueError
			return std::nullopt;
		}
		auto yValue = GetObjectAttrAsValue<float>(object, "y");
		if (!yValue) {
			// GetObjectAttrAsValue set error. e.g. AttributeError, TypeError, ValueError
			return std::nullopt;
		}
		return Vector2{ *xValue, *yValue };
	}

	PyObject* Python3LanguageModule::CreateVector3Object(const Vector3& vector) {
		PyObject* const args = PyTuple_New(Py_ssize_t{ 3 });
		if (!args) {
			PyErr_SetString(PyExc_RuntimeError, "Fail to create arguments tuple");
			return nullptr;
		}
		// CreatePyObject set error
		PyObject* const xObject = CreatePyObject(vector.x);
		if (!xObject) {
			Py_DECREF(args);
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 0 }, xObject); // xObject ref taken by tuple
		PyObject* const yObject = CreatePyObject(vector.y);
		if (!yObject) {
			Py_DECREF(args);
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 1 }, yObject); // yObject ref taken by tuple
		PyObject* const zObject = CreatePyObject(vector.z);
		if (!zObject) {
			Py_DECREF(args);
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 2 }, zObject); // zObject ref taken by tuple
		PyObject* const vectorObject = PyObject_CallObject(_Vector3TypeObject, args);
		Py_DECREF(args);
		return vectorObject;
	}

	std::optional<Vector3> Python3LanguageModule::Vector3ValueFromObject(PyObject* object) {
		const int typeResult = PyObject_IsInstance(object, _Vector3TypeObject);
		if (typeResult == -1) {
			// Python exception was set by PyObject_IsInstance
			return std::nullopt;
		}
		if (typeResult == 0) {
			PyErr_SetString(PyExc_TypeError, "Not Vector3");
			return std::nullopt;
		}
		auto xValue = GetObjectAttrAsValue<float>(object, "x");
		if (!xValue) {
			// GetObjectAttrAsValue set error. e.g. AttributeError, TypeError, ValueError
			return std::nullopt;
		}
		auto yValue = GetObjectAttrAsValue<float>(object, "y");
		if (!yValue) {
			// GetObjectAttrAsValue set error. e.g. AttributeError, TypeError, ValueError
			return std::nullopt;
		}
		auto zValue = GetObjectAttrAsValue<float>(object, "z");
		if (!zValue) {
			// GetObjectAttrAsValue set error. e.g. AttributeError, TypeError, ValueError
			return std::nullopt;
		}
		return Vector3{ *xValue, *yValue, *zValue };
	}

	PyObject* Python3LanguageModule::CreateVector4Object(const Vector4& vector) {
		PyObject* const args = PyTuple_New(Py_ssize_t{ 4 });
		if (!args) {
			PyErr_SetString(PyExc_RuntimeError, "Fail to create arguments tuple");
			return nullptr;
		}
		// CreatePyObject set error
		PyObject* const xObject = CreatePyObject(vector.x);
		if (!xObject) {
			Py_DECREF(args);
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 0 }, xObject); // xObject ref taken by tuple
		PyObject* const yObject = CreatePyObject(vector.y);
		if (!yObject) {
			Py_DECREF(args);
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 1 }, yObject); // yObject ref taken by tuple
		PyObject* const zObject = CreatePyObject(vector.z);
		if (!zObject) {
			Py_DECREF(args);
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 2 }, zObject); // zObject ref taken by tuple
		PyObject* const wObject = CreatePyObject(vector.w);
		if (!wObject) {
			Py_DECREF(args);
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 3 }, wObject); // wObject ref taken by tuple
		PyObject* const vectorObject = PyObject_CallObject(_Vector4TypeObject, args);
		Py_DECREF(args);
		return vectorObject;
	}

	std::optional<Vector4> Python3LanguageModule::Vector4ValueFromObject(PyObject* object) {
		const int typeResult = PyObject_IsInstance(object, _Vector4TypeObject);
		if (typeResult == -1) {
			// Python exception was set by PyObject_IsInstance
			return std::nullopt;
		}
		if (typeResult == 0) {
			PyErr_SetString(PyExc_TypeError, "Not Vector4");
			return std::nullopt;
		}
		auto xValue = GetObjectAttrAsValue<float>(object, "x");
		if (!xValue) {
			// GetObjectAttrAsValue set error. e.g. AttributeError, TypeError, ValueError
			return std::nullopt;
		}
		auto yValue = GetObjectAttrAsValue<float>(object, "y");
		if (!yValue) {
			// GetObjectAttrAsValue set error. e.g. AttributeError, TypeError, ValueError
			return std::nullopt;
		}
		auto zValue = GetObjectAttrAsValue<float>(object, "z");
		if (!zValue) {
			// GetObjectAttrAsValue set error. e.g. AttributeError, TypeError, ValueError
			return std::nullopt;
		}
		auto wValue = GetObjectAttrAsValue<float>(object, "w");
		if (!wValue) {
			// GetObjectAttrAsValue set error. e.g. AttributeError, TypeError, ValueError
			return std::nullopt;
		}
		return Vector4{ *xValue, *yValue, *zValue, *wValue };
	}

	PyObject* Python3LanguageModule::CreateMatrix4x4Object(const Matrix4x4& matrix) {
		PyObject* const elementsObject = PyList_New(Py_ssize_t{ 16 });
		if (!elementsObject) {
			PyErr_SetString(PyExc_RuntimeError, "Fail to create Matrix4x4 elements list");
			return nullptr;
		}
		// CreatePyObject set error
		PyObject* const m00Object = CreatePyObject(matrix.m00);
		if (!m00Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 0 }, m00Object); // m00Object ref taken by list
		PyObject* const m01Object = CreatePyObject(matrix.m01);
		if (!m01Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 1 }, m01Object); // m01Object ref taken by list
		PyObject* const m02Object = CreatePyObject(matrix.m02);
		if (!m02Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 2 }, m02Object); // m02Object ref taken by list
		PyObject* const m03Object = CreatePyObject(matrix.m03);
		if (!m03Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 3 }, m03Object); // m03Object ref taken by list
		PyObject* const m10Object = CreatePyObject(matrix.m10);
		if (!m10Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 4 }, m10Object); // m10Object ref taken by list
		PyObject* const m11Object = CreatePyObject(matrix.m11);
		if (!m11Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 5 }, m11Object); // m11Object ref taken by list
		PyObject* const m12Object = CreatePyObject(matrix.m12);
		if (!m12Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 6 }, m12Object); // m12Object ref taken by list
		PyObject* const m13Object = CreatePyObject(matrix.m13);
		if (!m13Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 7 }, m13Object); // m13Object ref taken by list
		PyObject* const m20Object = CreatePyObject(matrix.m20);
		if (!m20Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 8 }, m20Object); // m20Object ref taken by list
		PyObject* const m21Object = CreatePyObject(matrix.m21);
		if (!m21Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 9 }, m21Object); // m21Object ref taken by list
		PyObject* const m22Object = CreatePyObject(matrix.m22);
		if (!m22Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 10 }, m22Object); // m22Object ref taken by list
		PyObject* const m23Object = CreatePyObject(matrix.m23);
		if (!m23Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 11 }, m23Object); // m23Object ref taken by list
		PyObject* const m30Object = CreatePyObject(matrix.m30);
		if (!m30Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 12 }, m30Object); // m30Object ref taken by list
		PyObject* const m31Object = CreatePyObject(matrix.m31);
		if (!m31Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 13 }, m31Object); // m31Object ref taken by list
		PyObject* const m32Object = CreatePyObject(matrix.m32);
		if (!m32Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 14 }, m32Object); // m32Object ref taken by list
		PyObject* const m33Object = CreatePyObject(matrix.m33);
		if (!m33Object) {
			Py_DECREF(elementsObject);
			return nullptr;
		}
		PyList_SET_ITEM(elementsObject, Py_ssize_t{ 15 }, m33Object); // m33Object ref taken by list
		PyObject* const args = PyTuple_New(Py_ssize_t{ 1 });
		if (!args) {
			Py_DECREF(elementsObject);
			PyErr_SetString(PyExc_RuntimeError, "Fail to create arguments tuple");
			return nullptr;
		}
		PyTuple_SET_ITEM(args, Py_ssize_t{ 0 }, elementsObject); // elementsObject ref taken by tuple
		PyObject* const vectorObject = PyObject_CallObject(_Matrix4x4TypeObject, args);
		Py_DECREF(args);
		return vectorObject;
	}

	std::optional<Matrix4x4> Python3LanguageModule::Matrix4x4ValueFromObject(PyObject* object) {
		const int typeResult = PyObject_IsInstance(object, _Matrix4x4TypeObject);
		if (typeResult == -1) {
			// Python exception was set by PyObject_IsInstance
			return std::nullopt;
		}
		if (typeResult == 0) {
			PyErr_SetString(PyExc_TypeError, "Not Matrix4x4");
			return std::nullopt;
		}
		PyObject* const elementsListObject = PyObject_GetAttrString(object, "elements");
		if (!elementsListObject) {
			// PyObject_GetAttrString set error. e.g. AttributeError
			return std::nullopt;
		}
		if (!PyList_CheckExact(elementsListObject)) {
			Py_DECREF(elementsListObject);
			PyErr_SetString(PyExc_ValueError, "Elements must be a 4x4 list");
			return std::nullopt;
		}
		if (PyList_Size(elementsListObject) != Py_ssize_t{ 4 }) {
			Py_DECREF(elementsListObject);
			PyErr_SetString(PyExc_ValueError, "Elements must be a 4x4 list");
			return std::nullopt;
		}
		Matrix4x4 matrix{};
		for (Py_ssize_t i = 0; i < Py_ssize_t{ 4 }; ++i) {
			PyObject* const elementsRowListObject = PyList_GetItem(elementsListObject, i);
			if (!elementsRowListObject) [[unlikely]] {
				Py_DECREF(elementsListObject);
				// PyList_GetItem set error. e.g. IndexError
				return std::nullopt;
				}
				if (!PyList_CheckExact(elementsRowListObject)) {
					Py_DECREF(elementsListObject);
					PyErr_SetString(PyExc_ValueError, "Elements must be a 4x4 list");
					return std::nullopt;
				}
				if (PyList_Size(elementsRowListObject) != Py_ssize_t{ 4 }) {
					Py_DECREF(elementsListObject);
					PyErr_SetString(PyExc_ValueError, "Elements must be a 4x4 list");
					return std::nullopt;
				}
				for (Py_ssize_t j = 0; j < Py_ssize_t{ 4 }; ++j) {
					PyObject* const mObject = PyList_GetItem(elementsRowListObject, j);
					if (!mObject) [[unlikely]] {
						Py_DECREF(elementsListObject);
						/// PyList_GetItem set error. e.g. IndexError
						return std::nullopt;
						}
					const auto mValue = ValueFromObject<float>(mObject);
					if (!mValue) {
						Py_DECREF(elementsListObject);
						// ValueFromObject set error. e.g. TypeError
						return std::nullopt;
					}
					matrix.data[static_cast<size_t>(i * Py_ssize_t{ 4 } + j)] = *mValue;
				}
		}
		return { std::move(matrix) };
	}

	PyObject* Python3LanguageModule::FindPythonMethod(void* addr) const {
		for (const auto& data : _pythonMethods) {
			if (data.jitFunction.GetFunction() == addr) {
				return data.pythonFunction;
			}
		}
		return nullptr;
	}

	PyObject* Python3LanguageModule::CreateInternalModule(const IPlugin& plugin) {
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

	PyObject* Python3LanguageModule::CreateExternalModule(const IPlugin& plugin) {
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

	void Python3LanguageModule::TryCallPluginMethodNoArgs(const IPlugin& plugin, const std::string& name, const std::string& context) {
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

	Python3LanguageModule g_py3lm;

	extern "C"
	PY3LM_EXPORT ILanguageModule* GetLanguageModule() {
		return &g_py3lm;
	}
}
