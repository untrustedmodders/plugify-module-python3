import sys
from plugify.plugin import Plugin, Vector2, Vector3, Vector4, Matrix4x4
from plugify import pps


def bool_str(b):
    return f'{b}'.lower() if isinstance(b, bool) else '<wrong value>'


def ptr_str(v):
    return f'{v:#x}'


def ord_zero(ch: str):
    return 0 if len(ch) == 0 else ord(ch)


def strip_zero(fl: str):
    s = f'{float(fl)}'
    if s.endswith('.0'):
        s = s[:-2]
    return s


def float_str(v: float, aq: bool = True):
    return strip_zero(f'{v:.4f}' if aq else f'{v:.6f}')


def quote_str(s: str):
    return f"'{s}'"


def char_str(ch):
    return f'{ord_zero(ch)}'


def vector_to_string(array, f=None):
    if f is None:
        def f(v):
            return f'{v}'
    return f"{{{', '.join([f(v) for v in array])}}}"


def pod_to_string(pod):
    if isinstance(pod, Vector2):
        return f'{{{float_str(pod.x)}, {float_str(pod.y)}}}'
    if isinstance(pod, Vector3):
        return f'{{{float_str(pod.x)}, {float_str(pod.y)}, {float_str(pod.z)}}}'
    if isinstance(pod, Vector4):
        return f'{{{float_str(pod.x)}, {float_str(pod.y)}, {float_str(pod.z)}, {float_str(pod.w)}}}'
    if isinstance(pod, Matrix4x4):
        formatted_rows = [f"{{{', '.join([float_str(m) for m in row])}}}" for row in pod.elements]
        return f'{{{", ".join(formatted_rows)}}}'
    raise TypeError('Non POD type')


# <<< Test part >>>

class CrossCallWorker(Plugin):
    pass


def no_param_return_void():
    pass


def no_param_return_bool():
    return True


def no_param_return_char8():
    return '\x7f'


def no_param_return_char16():
    return u'\uffff'


def no_param_return_int8():
    return 0x7f


def no_param_return_int16():
    return 0x7fff


def no_param_return_int32():
    return 0x7fffffff


def no_param_return_int64():
    return 0x7fffffffffffffff


def no_param_return_uint8():
    return 0xff


def no_param_return_uint16():
    return 0xffff


def no_param_return_uint32():
    return 0xffffffff


def no_param_return_uint64():
    return 0xffffffffffffffff


def no_param_return_pointer():
    return 0x1


def no_param_return_float():
    return 3.4028235e38


def no_param_return_double():
    return sys.float_info.max


def no_param_return_function():
    return None


def no_param_return_string():
    return 'Hello World'


def no_param_return_array_bool():
    return [True, False]


def no_param_return_array_char8():
    return ['a', 'b', 'c', 'd']


def no_param_return_array_char16():
    return ['a', 'b', 'c', 'd']


def no_param_return_array_int8():
    return [-3, -2, -1, 0, 1]


def no_param_return_array_int16():
    return [-4, -3, -2, -1, 0, 1]


def no_param_return_array_int32():
    return [-5, -4, -3, -2, -1, 0, 1]


def no_param_return_array_int64():
    return [-6, -5, -4, -3, -2, -1, 0, 1]


def no_param_return_array_uint8():
    return [0, 1, 2, 3, 4, 5, 6, 7, 8]


def no_param_return_array_uint16():
    return [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]


def no_param_return_array_uint32():
    return [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]


def no_param_return_array_uint64():
    return [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]


def no_param_return_array_pointer():
    return [0, 1, 2, 3]


def no_param_return_array_float():
    return [-12.34, 0.0, 12.34]


def no_param_return_array_double():
    return [-12.345, 0.0, 12.345]


def no_param_return_array_string():
    return ['1st string', '2nd string', '3rd element string (Should be big enough to avoid small string optimization)']


def no_param_return_vector2():
    return Vector2(1.0, 2.0)


def no_param_return_vector3():
    return Vector3(1.0, 2.0, 3.0)


def no_param_return_vector4():
    return Vector4(1.0, 2.0, 3.0, 4.0)


def no_param_return_matrix4x4():
    return Matrix4x4([[1.0, 2.0, 3.0, 4.0], [5.0, 6.0, 7.0, 8.0], [9.0, 10.0, 11.0, 12.0], [13.0, 14.0, 15.0, 16.0]])


def param1(a):
    buffer = f'{a}'


def param2(a, b):
    buffer = f'{a}{b}'


def param3(a, b, c):
    buffer = f'{a}{b}{c}'


def param4(a, b, c, d):
    buffer = f'{a}{b}{c}{d}'


def param5(a, b, c, d, e):
    buffer = f'{a}{b}{c}{d}{e}'


def param6(a, b, c, d, e, f):
    buffer = f'{a}{b}{c}{d}{e}{f}'


def param7(a, b, c, d, e, f, g):
    buffer = f'{a}{b}{c}{d}{e}{f}{g}'


def param8(a, b, c, d, e, f, g, h):
    buffer = f'{a}{b}{c}{d}{e}{f}{g}{h}'


def param9(a, b, c, d, e, f, g, h, k):
    buffer = f'{a}{b}{c}{d}{e}{f}{g}{h}{k}'


def param10(a, b, c, d, e, f, g, h, k, l):
    buffer = f'{a}{b}{c}{d}{e}{f}{g}{h}{k}{l}'


def param_ref1(a):
    return None, 42


def param_ref2(a, b):
    return None, 10, 3.14


def param_ref3(a, b, c):
    return None, -20, 2.718, 3.14159


def param_ref4(a, b, c, d):
    return None, 100, -5.55, 1.618, Vector4(1.0, 2.0, 3.0, 4.0)


def param_ref5(a, b, c, d, e):
    return None, 500, -10.5, 2.71828, Vector4(-1.0, -2.0, -3.0, -4.0), [-6, -5, -4, -3, -2, -1, 0, 1]


def param_ref6(a, b, c, d, e, f):
    return None, 750, 20.0, 1.23456, Vector4(10.0, 20.0, 30.0, 40.0), [-6, -5, -4], 'Z'


def param_ref7(a, b, c, d, e, f, g):
    return (None, -1000, 3.0, -1.0, Vector4(100.0, 200.0, 300.0, 400.0), [-6, -5, -4, -3], 'Y',
            'Hello, World!')


def param_ref8(a, b, c, d, e, f, g, h):
    return (None, 999, -7.5, 0.123456, Vector4(-100.0, -200.0, -300.0, -400.0), [-6, -5, -4, -3, -2, -1], 'X',
            'Goodbye, World!', 'A')


def param_ref9(a, b, c, d, e, f, g, h, k):
    return (None, -1234, 123.45, -678.9, Vector4(987.65, 432.1, 123.456, 789.123),
            [-6, -5, -4, -3, -2, -1, 0, 1, 5, 9], 'W', 'Testing, 1 2 3', 'B', 42)


def param_ref10(a, b, c, d, e, f, g, h, k, l):
    return (None, 987, -0.123, 456.789, Vector4(-123.456, 0.987, 654.321, -789.123),
            [-6, -5, -4, -3, -2, -1, 0, 1, 5, 9, 4, -7], 'V', 'Another string', 'C', -444, 0x12345678)


def param_ref_vectors(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15):
    return (None,
            [True],
            ['a', 'b', 'c'],
            ['d', 'e', 'f'],
            [-3, -2, -1, 0, 1, 2, 3],
            [-4, -3, -2, -1, 0, 1, 2, 3, 4],
            [-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5],
            [-6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6],
            [0, 1, 2, 3, 4, 5, 6, 7],
            [0, 1, 2, 3, 4, 5, 6, 7, 8],
            [0, 1, 2, 3, 4, 5, 6, 7, 8, 9],
            [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
            [0, 1, 2],
            [-12.34, 0.0, 12.34],
            [-12.345, 0.0, 12.345],
            ['1', '12', '123', '1234', '12345', '123456'])


def param_all_primitives(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14):
    buffer = f'{p1}{p2}{p3}{p4}{p5}{p6}{p7}{p8}{p9}{p10}{p11}{p12}{p13}{p14}'
    return 56


def call_func_void(func):
    func()


def call_func_bool(func):
    return func()


def call_func_char8(func):
    return func()


def call_func_char16(func):
    return func()


def call_func_int8(func):
    return func()


def call_func_int16(func):
    return func()


def call_func_int32(func):
    return func()


def call_func_int64(func):
    return func()


def call_func_uint8(func):
    return func()


def call_func_uint16(func):
    return func()


def call_func_uint32(func):
    return func()


def call_func_uint64(func):
    return func()


def call_func_ptr(func):
    return func()


def call_func_float(func):
    return func()


def call_func_double(func):
    return func()


def call_func_function(func):
    return func()


def call_func_string(func):
    return func()


def call_func_bool_vector(func):
    return func()


def call_func_char8_vector(func):
    return func()


def call_func_char16_vector(func):
    return func()


def call_func_int8_vector(func):
    return func()


def call_func_int16_vector(func):
    return func()


def call_func_int32_vector(func):
    return func()


def call_func_int64_vector(func):
    return func()


def call_func_uint8_vector(func):
    return func()


def call_func_uint16_vector(func):
    return func()


def call_func_uint32_vector(func):
    return func()


def call_func_uint64_vector(func):
    return func()


def call_func_ptr_vector(func):
    return func()


def call_func_float_vector(func):
    return func()


def call_func_double_vector(func):
    return func()


def call_func_string_vector(func):
    return func()


def call_func_vec2(func):
    return func()


def call_func_vec3(func):
    return func()


def call_func_vec4(func):
    return func()


def call_func_mat4x4(func):
    return func()


def call_func1(func):
    vec = Vector3(4.5, 5.6, 6.7)
    return func(vec)


def call_func2(func):
    f = 2.71
    i64 = 200
    return func(f, i64)


def call_func3(func):
    ptr = 12345
    vec4 = Vector4(7.8, 8.9, 9.1, 10.2)
    str_value = 'RandomString'
    func(ptr, vec4, str_value)


def call_func4(func):
    b = False
    i32 = 42
    ch16 = 'B'
    mat = Matrix4x4.zero()
    return func(b, i32, ch16, mat)


def call_func5(func):
    i8 = 10
    vec2 = Vector2(3.4, 5.6)
    ptr = 67890
    d = 1.618
    vec64 = [4, 5, 6]
    return func(i8, vec2, ptr, d, vec64)


def call_func6(func):
    str_value = 'AnotherString'
    f = 4.56
    vec_f = [4.0, 5.0, 6.0]
    i16 = 30
    vec_u8 = [3, 4, 5]
    ptr = 24680
    return func(str_value, f, vec_f, i16, vec_u8, ptr)


def call_func7(func):
    vec_c = ['X', 'Y', 'Z']
    u16 = 20
    ch16 = 'C'
    vec_u32 = [4, 5, 6]
    vec4 = Vector4(4.5, 5.6, 6.7, 7.8)
    b = False
    u64 = 200
    return func(vec_c, u16, ch16, vec_u32, vec4, b, u64)


def call_func8(func):
    vec3 = Vector3(4.0, 5.0, 6.0)
    vec_u32 = [4, 5, 6]
    i16 = 30
    b = False
    vec4 = Vector4(4.5, 5.6, 6.7, 7.8)
    vec_c16 = ['D', 'E']
    ch16 = 'B'
    i32 = 50
    return func(vec3, vec_u32, i16, b, vec4, vec_c16, ch16, i32)


def call_func9(func):
    f = 2.71
    vec2 = Vector2(3.4, 5.6)
    vec_i8 = [4, 5, 6]
    u64 = 250
    b = False
    s = "Random"
    vec4 = Vector4(4.5, 5.6, 6.7, 7.8)
    i16 = 30
    ptr = 13579
    func(f, vec2, vec_i8, u64, b, s, vec4, i16, ptr)


def call_func10(func):
    vec4 = Vector4(5.6, 7.8, 8.9, 9.0)
    mat = Matrix4x4.zero()
    vec_u32 = [4, 5, 6]
    u64 = 150
    vec_c = ['X', 'Y', 'Z']
    i32 = 60
    b = False
    vec2 = Vector2(3.4, 5.6)
    i64 = 75
    d = 2.71
    return func(vec4, mat, vec_u32, u64, vec_c, i32, b, vec2, i64, d)


def call_func11(func):
    vec_b = [False, True, False]
    ch16 = 'C'
    u8 = 10
    d = 2.71
    vec3 = Vector3(4.0, 5.0, 6.0)
    vec_i8 = [3, 4, 5]
    i64 = 150
    u16 = 20
    f = 2.0
    vec2 = Vector2(4.5, 6.7)
    u32 = 30
    return func(vec_b, ch16, u8, d, vec3, vec_i8, i64, u16, f, vec2, u32)


def call_func12(func):
    ptr = 98765
    vec_d = [4.0, 5.0, 6.0]
    u32 = 30
    d = 1.41
    b = False
    i32 = 25
    i8 = 10
    u64 = 300
    f = 2.72
    vec_ptr = [2, 3, 4]
    i64 = 200
    ch = 'B'
    return func(ptr, vec_d, u32, d, b, i32, i8, u64, f, vec_ptr, i64, ch)


def call_func13(func):
    i64 = 75
    vec_c = ['D', 'E', 'F']
    u16 = 20
    f = 2.71
    vec_b = [False, True, False]
    vec4 = Vector4(5.6, 7.8, 9.0, 10.1)
    s = 'RandomString'
    i32 = 30
    vec3 = Vector3(4.0, 5.0, 6.0)
    ptr = 13579
    vec2 = Vector2(4.5, 6.7)
    vec_u8 = [2, 3, 4]
    i16 = 20
    return func(i64, vec_c, u16, f, vec_b, vec4, s, i32, vec3, ptr, vec2, vec_u8, i16)


def call_func14(func):
    vec_c = ['D', 'E', 'F']
    vec_u32 = [4, 5, 6]
    mat = Matrix4x4.zero()
    b = False
    ch16 = 'B'
    i32 = 25
    vec_f = [4.0, 5.0, 6.0]
    u16 = 30
    vec_u8 = [3, 4, 5]
    i8 = 10
    vec3 = Vector3(4.0, 5.0, 6.0)
    vec4 = Vector4(5.6, 7.8, 9.0, 10.1)
    d = 2.72
    ptr = 54321
    return func(vec_c, vec_u32, mat, b, ch16, i32, vec_f, u16, vec_u8, i8, vec3, vec4, d, ptr)


def call_func15(func):
    vec_i16 = [4, 5, 6]
    mat = Matrix4x4.zero()
    vec4 = Vector4(7.8, 8.9, 9.0, 10.1)
    ptr = 12345
    u64 = 200
    vec_u32 = [5, 6, 7]
    b = False
    f = 3.14
    vec_c16 = ['D', 'E']
    u8 = 6
    i32 = 25
    vec2 = Vector2(5.6, 7.8)
    u16 = 40
    d = 2.71
    vec_u8 = [1, 3, 5]
    return func(vec_i16, mat, vec4, ptr, u64, vec_u32, b, f, vec_c16, u8, i32, vec2, u16, d, vec_u8)


def call_func16(func):
    vec_b = [True, True, False]
    i16 = 20
    vec_i8 = [2, 3, 4]
    vec4 = Vector4(7.8, 8.9, 9.0, 10.1)
    mat = Matrix4x4.zero()
    vec2 = Vector2(5.6, 7.8)
    vec_u64 = [5, 6, 7]
    vec_c = ['D', 'E', 'F']
    s = 'DifferentString'
    i64 = 300
    vec_u32 = [6, 7, 8]
    vec3 = Vector3(5.0, 6.0, 7.0)
    f = 3.14
    d = 2.718
    i8 = 6
    u16 = 30
    return func(vec_b, i16, vec_i8, vec4, mat, vec2, vec_u64, vec_c, s, i64, vec_u32, vec3, f, d, i8, u16)


def call_func17(func):
    i32 = 42

    _, i32 = func(i32)

    return f'{i32}'


def call_func18(func):
    i8 = 9
    i16 = 25

    ret, i8, i16 = func(i8, i16)

    return f'{pod_to_string(ret)}|{i8}|{i16}'


def call_func19(func):
    u32 = 75
    vec3 = Vector3(4.0, 5.0, 6.0)
    vec_u32 = [4, 5, 6]

    _, u32, vec3, vec_u32 = func(u32, vec3, vec_u32)

    return f'{u32}|{pod_to_string(vec3)}|{vector_to_string(vec_u32)}'


def call_func20(func):
    ch16 = 'Z'
    vec4 = Vector4(5.0, 6.0, 7.0, 8.0)
    vec_u64 = [4, 5, 6]
    ch = 'X'

    ret, ch16, vec4, vec_u64, ch = func(ch16, vec4, vec_u64, ch)

    return f'{ret}|{ord_zero(ch16)}|{pod_to_string(vec4)}|{vector_to_string(vec_u64)}|{ch}'


def call_func21(func):
    mat = Matrix4x4.zero()
    vec_i32 = [4, 5, 6]
    vec2 = Vector2(3.0, 4.0)
    b = False
    d = 6.28

    ret, mat, vec_i32, vec2, b, d = func(mat, vec_i32, vec2, b, d)

    return f'{float_str(ret)}|{pod_to_string(mat)}|{vector_to_string(vec_i32)}|{pod_to_string(vec2)}|{bool_str(b)}|{float_str(d)}'


def call_func22(func):
    ptr = 1
    u32 = 20
    vec_d = [4.0, 5.0, 6.0]
    i16 = 15
    str_param = 'Updated Test'
    vec4 = Vector4(5.0, 6.0, 7.0, 8.0)

    ret, ptr, u32, vec_d, i16, str_param, vec4 = func(ptr, u32, vec_d, i16, str_param, vec4)

    return f'{ret}|{ptr_str(ptr)}|{u32}|{vector_to_string(vec_d)}|{i16}|{str_param}|{pod_to_string(vec4)}'


def call_func23(func):
    u64 = 200
    vec2 = Vector2(3.0, 4.0)
    vec_i16 = [4, 5, 6]
    ch16 = 'Y'
    f = 2.34
    i8 = 10
    vec_u8 = [3, 4, 5]

    _, u64, vec2, vec_i16, ch16, f, i8, vec_u8 = func(u64, vec2, vec_i16, ch16, f, i8, vec_u8)

    return f'{u64}|{pod_to_string(vec2)}|{vector_to_string(vec_i16)}|{ord_zero(ch16)}|{float_str(f)}|{i8}|{vector_to_string(vec_u8)}'


def call_func24(func):
    vec_c = ['D', 'E', 'F']
    i64 = 100
    vec_u8 = [3, 4, 5]
    vec4 = Vector4(5.0, 6.0, 7.0, 8.0)
    u64 = 200
    vec_ptr = [3, 4, 5]
    d = 6.28
    vec_ptr_2 = [4, 5, 6, 7]

    ret, vec_c, i64, vec_u8, vec4, u64, vec_ptr, d, vec_ptr_2 \
        = func(vec_c, i64, vec_u8, vec4, u64, vec_ptr, d, vec_ptr_2)

    return f'{pod_to_string(ret)}|{vector_to_string(vec_c, char_str)}|{i64}|{vector_to_string(vec_u8)}|{pod_to_string(vec4)}|{u64}|{vector_to_string(vec_ptr, ptr_str)}|{float_str(d)}|{vector_to_string(vec_ptr_2, ptr_str)}'


def call_func25(func):
    i32 = 50
    vec_ptr = [3, 4, 5]
    b = False
    u8 = 10
    str_val = 'Updated Test String'
    vec3 = Vector3(4.0, 5.0, 6.0)
    i64 = 100
    vec4 = Vector4(5.0, 6.0, 7.0, 8.0)
    u16 = 20

    ret, i32, vec_ptr, b, u8, str_val, vec3, i64, vec4, u16 = func(i32, vec_ptr, b, u8, str_val, vec3, i64, vec4, u16)

    return f'{float_str(ret)}|{i32}|{vector_to_string(vec_ptr, ptr_str)}|{bool_str(b)}|{u8}|{str_val}|{pod_to_string(vec3)}|{i64}|{pod_to_string(vec4)}|{u16}'


def call_func26(func):
    ch16 = 'B'
    vec2 = Vector2(3.0, 4.0)
    mat = Matrix4x4.zero()
    vec_f = [4.0, 5.0, 6.0]
    i16 = 20
    u64 = 200
    u32 = 20
    vec_u16 = [3, 4, 5]
    ptr = 0xDEADBEAFDEADBEAF
    b = False

    ret, ch16, vec2, mat,vec_f, i16, u64, u32, vec_u16, ptr, b \
        = func(ch16, vec2, mat,vec_f, i16, u64, u32, vec_u16, ptr, b)

    return f'{ret}|{ord_zero(ch16)}|{pod_to_string(vec2)}|{pod_to_string(mat)}|{vector_to_string(vec_f, float_str)}|{u64}|{u32}|{vector_to_string(vec_u16)}|{ptr_str(ptr)}|{bool_str(b)}'


def call_func27(func):
    f = 2.56
    vec3 = Vector3(4.0, 5.0, 6.0)
    ptr = 0
    vec2 = Vector2(3.0, 4.0)
    vec_i16 = [4, 5, 6]
    mat = Matrix4x4.zero()
    b = False
    vec4 = Vector4(5.0, 6.0, 7.0, 8.0)
    i8 = 10
    i32 = 40
    vec_u8 = [3, 4, 5]

    ret, f, vec3, ptr, vec2, vec_i16, mat, b, vec4, i8, i32, vec_u8 \
        = func(f, vec3, ptr, vec2, vec_i16, mat, b, vec4, i8, i32, vec_u8)

    return f'{ret}|{float_str(f)}|{pod_to_string(vec3)}|{ptr_str(ptr)}|{pod_to_string(vec2)}|{vector_to_string(vec_i16)}|{pod_to_string(mat)}|{bool_str(b)}|{pod_to_string(vec4)}|{i8}|{i32}|{vector_to_string(vec_u8)}'


def call_func28(func):
    ptr = 1
    u16 = 20
    vec_u32 = [4, 5, 6]
    mat = Matrix4x4.zero()
    f = 2.71
    vec4 = Vector4(5.0, 6.0, 7.0, 8.0)
    str_val = 'New example string'
    vec_u64 = [400, 500, 600]
    i64 = 987654321
    b = False
    vec3 = Vector3(4.0, 5.0, 6.0)
    vec_f = [4.0, 5.0, 6.0]

    ret, ptr, u16, vec_u32, mat, f, vec4, str_val, vec_u64, i64, b, vec3, vec_f \
        = func(ptr, u16, vec_u32, mat, f, vec4, str_val, vec_u64, i64, b, vec3, vec_f)

    return f'{ret}|{ptr_str(ptr)}|{u16}|{vector_to_string(vec_u32)}|{pod_to_string(mat)}|{float_str(f)}|{pod_to_string(vec4)}|{str_val}|{vector_to_string(vec_u64)}|{i64}|{bool_str(b)}|{pod_to_string(vec3)}|{vector_to_string(vec_f, float_str)}'


def call_func29(func):
    vec4 = Vector4(2.0, 3.0, 4.0, 5.0)
    i32 = 99
    vec_i8 = [4, 5, 6]
    d = 2.71
    b = False
    i8 = 10
    vec_u16 = [4, 5, 6]
    f = 3.21
    str_val = 'Yet another example string'
    mat = Matrix4x4.zero()
    u64 = 200
    vec3 = Vector3(5.0, 6.0, 7.0)
    vec_i64 = [2000, 3000, 4000]

    ret, vec4, i32, vec_i8, d, b, i8, vec_u16, f, str_val, mat, u64, vec3, vec_i64 \
        = func(vec4, i32, vec_i8, d, b, i8, vec_u16, f, str_val, mat, u64, vec3, vec_i64)

    return f'{vector_to_string(ret, quote_str)}|{pod_to_string(vec4)}|{i32}|{vector_to_string(vec_i8)}|{float_str(d)}|{bool_str(b)}|{i8}|{vector_to_string(vec_u16)}|{float_str(f)}|{str_val}|{pod_to_string(mat)}|{u64}|{pod_to_string(vec3)}|{vector_to_string(vec_i64)}'


def call_func30(func):
    ptr = 1
    vec4 = Vector4(2.0, 3.0, 4.0, 5.0)
    i64 = 987654321
    vec_u32 = [4, 5, 6]
    b = False
    str_val = 'Updated String for Func30'
    vec3 = Vector3(5.0, 6.0, 7.0)
    vec_u8 = [1, 2, 3]
    f = 5.67
    vec2 = Vector2(3.0, 4.0)
    mat = Matrix4x4.zero()
    i8 = 10
    vec_f = [4.0, 5.0, 6.0]
    d = 8.90

    ret, ptr, vec4, i64, vec_u32, b, str_val, vec3, vec_u8, f, vec2, mat, i8, vec_f, d \
        = func(ptr, vec4, i64, vec_u32, b, str_val, vec3, vec_u8, f, vec2, mat, i8, vec_f, d)

    return f'{ret}|{ptr_str(ptr)}|{pod_to_string(vec4)}|{i64}|{vector_to_string(vec_u32)}|{bool_str(b)}|{str_val}|{pod_to_string(vec3)}|{vector_to_string(vec_u8)}|{float_str(f)}|{pod_to_string(vec2)}|{pod_to_string(mat)}|{i8}|{vector_to_string(vec_f, float_str)}|{float_str(d, False)}'


def call_func31(func):
    ch = 'B'
    u32 = 200
    vec_u64 = [4, 5, 6]
    vec4 = Vector4(2.0, 3.0, 4.0, 5.0)
    str_val = 'Updated String for Func31'
    b = True
    i64 = 987654321
    vec2 = Vector2(3.0, 4.0)
    i8 = 10
    u16 = 20
    vec_i16 = [4, 5, 6]
    mat = Matrix4x4.zero()
    vec3 = Vector3(4.0, 5.0, 6.0)
    f = 5.67
    vec_d = [4.0, 5.0, 6.0]

    ret, ch, u32, vec_u64, vec4, str_val, b, i64, vec2, i8, u16, vec_i16, mat, vec3, f, vec_d \
        = func(ch, u32, vec_u64, vec4, str_val, b, i64, vec2, i8, u16, vec_i16, mat, vec3, f, vec_d)

    return f'{pod_to_string(ret)}|{ch}|{u32}|{vector_to_string(vec_u64)}|{pod_to_string(vec4)}|{str_val}|{bool_str(b)}|{i64}|{pod_to_string(vec2)}|{i8}|{u16}|{vector_to_string(vec_i16)}|{pod_to_string(mat)}|{pod_to_string(vec3)}|{float_str(f)}|{vector_to_string(vec_d)}'


def call_func32(func):
    i32 = 30
    u16 = 20
    vec_i8 = [4, 5, 6]
    vec4 = Vector4(2.0, 3.0, 4.0, 5.0)
    ptr = 1
    vec_u32 = [4, 5, 6]
    mat = Matrix4x4.zero()
    u64 = 200
    str_val = 'Updated String for Func32'
    i64 = 987654321
    vec2 = Vector2(3.0, 4.0)
    vec_i8_2 = [7, 8, 9]
    b = False
    vec3 = Vector3(4.0, 5.0, 6.0)
    u8 = 128
    vec_c16 = ['D', 'E', 'F']

    _, i32, u16, vec_i8, vec4, ptr, vec_u32, mat, u64, str_val, i64, vec2, vec_i8_2, b, vec3, u8, vec_c16 \
        = func(i32, u16, vec_i8, vec4, ptr, vec_u32, mat, u64, str_val, i64, vec2, vec_i8_2, b, vec3, u8, vec_c16)

    return f'{i32}|{u16}|{vector_to_string(vec_i8)}|{pod_to_string(vec4)}|{ptr_str(ptr)}|{vector_to_string(vec_u32)}|{pod_to_string(mat)}|{u64}|{str_val}|{i64}|{pod_to_string(vec2)}|{vector_to_string(vec_i8_2)}|{bool_str(b)}|{pod_to_string(vec3)}|{u8}|{vector_to_string(vec_c16, char_str)}'


def reverse_no_param_return_void():
    pps.cross_call_master.NoParamReturnVoidCallback()


def reverse_no_param_return_bool():
    result = pps.cross_call_master.NoParamReturnBoolCallback()
    return bool_str(result)


def reverse_no_param_return_char8():
    result = pps.cross_call_master.NoParamReturnChar8Callback()
    return char_str(result)


def reverse_no_param_return_char16():
    result = pps.cross_call_master.NoParamReturnChar16Callback()
    return char_str(result)


def reverse_no_param_return_int8():
    result = pps.cross_call_master.NoParamReturnInt8Callback()
    return f'{result}'


def reverse_no_param_return_int16():
    result = pps.cross_call_master.NoParamReturnInt16Callback()
    return f'{result}'


def reverse_no_param_return_int32():
    result = pps.cross_call_master.NoParamReturnInt32Callback()
    return f'{result}'


def reverse_no_param_return_int64():
    result = pps.cross_call_master.NoParamReturnInt64Callback()
    return f'{result}'


def reverse_no_param_return_uint8():
    result = pps.cross_call_master.NoParamReturnUInt8Callback()
    return f'{result}'


def reverse_no_param_return_uint16():
    result = pps.cross_call_master.NoParamReturnUInt16Callback()
    return f'{result}'


def reverse_no_param_return_uint32():
    result = pps.cross_call_master.NoParamReturnUInt32Callback()
    return f'{result}'


def reverse_no_param_return_uint64():
    result = pps.cross_call_master.NoParamReturnUInt64Callback()
    return f'{result}'


def reverse_no_param_return_pointer():
    result = pps.cross_call_master.NoParamReturnPointerCallback()
    return ptr_str(result)


def reverse_no_param_return_float():
    result = pps.cross_call_master.NoParamReturnFloatCallback()
    return float_str(result)


def reverse_no_param_return_double():
    result = pps.cross_call_master.NoParamReturnDoubleCallback()
    return f'{result}'


def reverse_no_param_return_function():
    result = pps.cross_call_master.NoParamReturnFunctionCallback()
    return f'{result()}' if result is not None else '<null function pointer>'


def reverse_no_param_return_string():
    result = pps.cross_call_master.NoParamReturnStringCallback()
    return result


def reverse_no_param_return_array_bool():
    result = pps.cross_call_master.NoParamReturnArrayBoolCallback()
    return vector_to_string(result, bool_str)


def reverse_no_param_return_array_char8():
    result = pps.cross_call_master.NoParamReturnArrayChar8Callback()
    return vector_to_string(result, char_str)


def reverse_no_param_return_array_char16():
    result = pps.cross_call_master.NoParamReturnArrayChar16Callback()
    return vector_to_string(result, char_str)


def reverse_no_param_return_array_int8():
    result = pps.cross_call_master.NoParamReturnArrayInt8Callback()
    return vector_to_string(result)


def reverse_no_param_return_array_int16():
    result = pps.cross_call_master.NoParamReturnArrayInt16Callback()
    return vector_to_string(result)


def reverse_no_param_return_array_int32():
    result = pps.cross_call_master.NoParamReturnArrayInt32Callback()
    return vector_to_string(result)


def reverse_no_param_return_array_int64():
    result = pps.cross_call_master.NoParamReturnArrayInt64Callback()
    return vector_to_string(result)


def reverse_no_param_return_array_uint8():
    result = pps.cross_call_master.NoParamReturnArrayUInt8Callback()
    return vector_to_string(result)


def reverse_no_param_return_array_uint16():
    result = pps.cross_call_master.NoParamReturnArrayUInt16Callback()
    return vector_to_string(result)


def reverse_no_param_return_array_uint32():
    result = pps.cross_call_master.NoParamReturnArrayUInt32Callback()
    return vector_to_string(result)


def reverse_no_param_return_array_uint64():
    result = pps.cross_call_master.NoParamReturnArrayUInt64Callback()
    return vector_to_string(result)


def reverse_no_param_return_array_pointer():
    result = pps.cross_call_master.NoParamReturnArrayPointerCallback()
    return vector_to_string(result, ptr_str)


def reverse_no_param_return_array_float():
    result = pps.cross_call_master.NoParamReturnArrayFloatCallback()
    return vector_to_string(result, float_str)


def reverse_no_param_return_array_double():
    result = pps.cross_call_master.NoParamReturnArrayDoubleCallback()
    return vector_to_string(result)


def reverse_no_param_return_array_string():
    result = pps.cross_call_master.NoParamReturnArrayStringCallback()
    return vector_to_string(result, quote_str)


def reverse_no_param_return_vector2():
    result = pps.cross_call_master.NoParamReturnVector2Callback()
    return pod_to_string(result)


def reverse_no_param_return_vector3():
    result = pps.cross_call_master.NoParamReturnVector3Callback()
    return pod_to_string(result)


def reverse_no_param_return_vector4():
    result = pps.cross_call_master.NoParamReturnVector4Callback()
    return pod_to_string(result)


def reverse_no_param_return_matrix4x4():
    result = pps.cross_call_master.NoParamReturnMatrix4x4Callback()
    return pod_to_string(result)


def reverse_param1():
    pps.cross_call_master.Param1Callback(999)


def reverse_param2():
    pps.cross_call_master.Param2Callback(888, 9.9)


def reverse_param3():
    pps.cross_call_master.Param3Callback(777, 8.8, 9.8765)


def reverse_param4():
    pps.cross_call_master.Param4Callback(666, 7.7, 8.7659, Vector4(100.1, 200.2, 300.3, 400.4))


def reverse_param5():
    pps.cross_call_master.Param5Callback(555, 6.6, 7.6598, Vector4(-105.1, -205.2, -305.3, -405.4), [])


def reverse_param6():
    pps.cross_call_master.Param6Callback(444, 5.5, 6.5987, Vector4(110.1, 210.2, 310.3, 410.4), [90000, -100, 20000], 'A')


def reverse_param7():
    pps.cross_call_master.Param7Callback(333, 4.4, 5.9876, Vector4(-115.1, -215.2, -315.3, -415.4), [800000, 30000, -4000000], 'B', 'red gold')


def reverse_param8():
    pps.cross_call_master.Param8Callback(222, 3.3, 1.2345, Vector4(120.1, 220.2, 320.3, 420.4), [7000000, 5000000, -600000000], 'C', 'blue ice', 'Z')


def reverse_param9():
    pps.cross_call_master.Param9Callback(111, 2.2, 5.1234, Vector4(-125.1, -225.2, -325.3, -425.4), [60000000, -700000000, 80000000000], 'D', 'pink metal', 'Y', -100)


def reverse_param10():
    pps.cross_call_master.Param10Callback(1234, 1.1, 4.5123, Vector4(130.1, 230.2, 330.3, 430.4), [500000000, 90000000000, 1000000000000], 'E', 'green wood', 'X', -200, 0xabeba)


def reverse_param_ref1():
    _, a = pps.cross_call_master.ParamRef1Callback(0)
    return f'{a}'


def reverse_param_ref2():
    _, a, b = pps.cross_call_master.ParamRef2Callback(0, 0.0)
    return f'{a}|{float_str(b)}'


def reverse_param_ref3():
    _, a, b, c = pps.cross_call_master.ParamRef3Callback(0, 0.0, 0.0)
    return f'{a}|{float_str(b)}|{c}'


def reverse_param_ref4():
    _, a, b, c, d = pps.cross_call_master.ParamRef4Callback(0, 0.0, 0.0, Vector4())
    return f'{a}|{float_str(b)}|{c}|{pod_to_string(d)}'


def reverse_param_ref5():
    _, a, b, c, d, e = pps.cross_call_master.ParamRef5Callback(0, 0.0, 0.0, Vector4(), [])
    return f'{a}|{float_str(b)}|{c}|{pod_to_string(d)}|{vector_to_string(e)}'


def reverse_param_ref6():
    _, a, b, c, d, e, f = pps.cross_call_master.ParamRef6Callback(0, 0.0, 0.0, Vector4(), [], '')
    return f'{a}|{float_str(b)}|{c}|{pod_to_string(d)}|{vector_to_string(e)}|{ord_zero(f)}'


def reverse_param_ref7():
    _, a, b, c, d, e, f, g = pps.cross_call_master.ParamRef7Callback(0, 0.0, 0.0, Vector4(), [], '', '')
    return f'{a}|{float_str(b)}|{c}|{pod_to_string(d)}|{vector_to_string(e)}|{ord_zero(f)}|{g}'


def reverse_param_ref8():
    _, a, b, c, d, e, f, g, h = pps.cross_call_master.ParamRef8Callback(0, 0.0, 0.0, Vector4(), [], '', '', '')
    return f'{a}|{float_str(b)}|{c}|{pod_to_string(d)}|{vector_to_string(e)}|{ord_zero(f)}|{g}|{ord_zero(h)}'


def reverse_param_ref9():
    _, a, b, c, d, e, f, g, h, k = pps.cross_call_master.ParamRef9Callback(0, 0.0, 0.0, Vector4(), [], '', '', '', 0)
    return f'{a}|{float_str(b)}|{c}|{pod_to_string(d)}|{vector_to_string(e)}|{ord_zero(f)}|{g}|{ord_zero(h)}|{k}'


def reverse_param_ref10():
    _, a, b, c, d, e, f, g, h, k, l = pps.cross_call_master.ParamRef10Callback(0, 0.0, 0.0, Vector4(), [], '', '', '', 0, 0)
    return f'{a}|{float_str(b)}|{c}|{pod_to_string(d)}|{vector_to_string(e)}|{ord_zero(f)}|{g}|{ord_zero(h)}|{k}|{ptr_str(l)}'


def reverse_param_ref_vectors():
    _, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 = pps.cross_call_master.ParamRefVectorsCallback(
        [True], ['A'], ['A'], [-1], [-1], [-1], [-1], [0], [0], [0], [0], [0], [1.0], [1.0], ['Hi']
    )
    p15_formatted = ', '.join([f"'{v}'" for v in p15])
    return f'{vector_to_string(p1, bool_str)}|{vector_to_string(p2, char_str)}|{vector_to_string(p3, char_str)}|' \
           f'{vector_to_string(p4)}|{vector_to_string(p5)}|{vector_to_string(p6)}|{vector_to_string(p7)}|' \
           f'{vector_to_string(p8)}|{vector_to_string(p9)}|{vector_to_string(p10)}|{vector_to_string(p11)}|' \
           f'{vector_to_string(p12, ptr_str)}|{vector_to_string(p13, float_str)}|{vector_to_string(p14)}|' \
           f'{vector_to_string(p15, quote_str)}'


def reverse_param_all_primitives():
    result = pps.cross_call_master.ParamAllPrimitivesCallback(True, '%', '☢', -1, -1000, -1000000, -1000000000000,
                                                              200, 50000, 3000000000, 9999999999, 0xfedcbaabcdef,
                                                              0.001, 987654.456789)
    return f'{result}'


class CallbackHolder:
    @staticmethod
    def mock_void():
        pass

    @staticmethod
    def mock_bool():
        return True

    @staticmethod
    def mock_char8():
        return 'A'

    @staticmethod
    def mock_char16():
        return 'Z'

    @staticmethod
    def mock_int8():
        return 10

    @staticmethod
    def mock_int16():
        return 100

    @staticmethod
    def mock_int32():
        return 1000

    @staticmethod
    def mock_int64():
        return 10000

    @staticmethod
    def mock_uint8():
        return 20

    @staticmethod
    def mock_uint16():
        return 200

    @staticmethod
    def mock_uint32():
        return 2000

    @staticmethod
    def mock_uint64():
        return 20000

    @staticmethod
    def mock_ptr():
        return 0

    @staticmethod
    def mock_float():
        return 3.14

    @staticmethod
    def mock_double():
        return 6.28

    @staticmethod
    def mock_function():
        return 2

    @staticmethod
    def mock_string():
        return 'Test string'

    @staticmethod
    def mock_bool_array():
        return [True, False]

    @staticmethod
    def mock_char8_array():
        return ['A', 'B']

    @staticmethod
    def mock_char16_array():
        return ['A', 'B']

    @staticmethod
    def mock_int8_array():
        return [10, 20]

    @staticmethod
    def mock_int16_array():
        return [100, 200]

    @staticmethod
    def mock_int32_array():
        return [1000, 2000]

    @staticmethod
    def mock_int64_array():
        return [10000, 20000]

    @staticmethod
    def mock_uint8_array():
        return [20, 30]

    @staticmethod
    def mock_uint16_array():
        return [200, 300]

    @staticmethod
    def mock_uint32_array():
        return [2000, 3000]

    @staticmethod
    def mock_uint64_array():
        return [20000, 30000]

    @staticmethod
    def mock_ptr_array():
        return [0, 1]

    @staticmethod
    def mock_float_array():
        return [1.1, 2.2]

    @staticmethod
    def mock_double_array():
        return [3.3, 4.4]

    @staticmethod
    def mock_string_array():
        return ['Hello', 'World']

    @staticmethod
    def mock_vec2():
        return Vector2(1.0, 2.0)

    @staticmethod
    def mock_vec3():
        return Vector3(1.0, 2.0, 3.0)

    @staticmethod
    def mock_vec4():
        return Vector4(1.0, 2.0, 3.0, 4.0)

    @staticmethod
    def mock_mat4x4():
        mat = Matrix4x4.zero()
        mat.elements[0][0] = 1.0
        return mat

    @staticmethod
    def mock_func1(vec3: Vector3):
        return int(vec3.x + vec3.y + vec3.z)

    @staticmethod
    def mock_func2(a, b):
        return '&'

    @staticmethod
    def mock_func3(p, v, s):
        pass

    @staticmethod
    def mock_func4(flag, u, c, m):
        return Vector4(1.0, 2.0, 3.0, 4.0)

    @staticmethod
    def mock_func5(i, v, p, d, vec):
        return True

    @staticmethod
    def mock_func6(s, f, vec, i, u_vec, p):
        return int(f + i)

    @staticmethod
    def mock_func7(vec, u, c, u_vec, v, flag, l):
        return 3.14

    @staticmethod
    def mock_func8(v, u_vec, i, flag, v4, c_vec, c, a):
        return Matrix4x4.zero()

    @staticmethod
    def mock_func9(f, v, i_vec, l, flag, s, v4, i, p):
        pass

    @staticmethod
    def mock_func10(v4, m, u_vec, l, c_vec, a, flag, v, i, d):
        return 42

    @staticmethod
    def mock_func11(b_vec, c, u, d, v3, i_vec, i, u16, f, v, u32):
        return 0

    @staticmethod
    def mock_func12(p, d_vec, u, d, flag, a, i, l, f, p_vec, i64, c):
        return False

    @staticmethod
    def mock_func13(i64, c_vec, u16, f, b_vec, v4, s, a, v3, p, v2, u8_vec, i16):
        return 'Dummy String'

    @staticmethod
    def mock_func14(c_vec, u_vec, m, flag, c, a, f_vec, u16, u8_vec, i8, v3, v4, d, p):
        return ['String1', 'String2']

    @staticmethod
    def mock_func15(i_vec, m, v4, p, l, u_vec, flag, f, c_vec, u, a, v2, u16, d, u8_vec):
        return 257

    @staticmethod
    def mock_func16(b_vec, i16, i_vec, v4, m, v2, u_vec, c_vec, s, i64, u32_vec, v3, f, d, i8, u16):
        return 0

    @staticmethod
    def mock_func17(ref_val):
        ref_val += 10
        return None, ref_val

    @staticmethod
    def mock_func18(i8, i16):
        i8 = 5
        i16 = 10
        return Vector2(5.0, 10.0), i8, i16

    @staticmethod
    def mock_func19(u32, v3, u_vec):
        u32 = 42
        v3 = Vector3(1.0, 2.0, 3.0)
        u_vec = [1, 2, 3]
        return None, u32, v3, u_vec

    @staticmethod
    def mock_func20(c, v4, u_vec, ch):
        c = 't'
        v4 = Vector4(1.0, 2.0, 3.0, 4.0)
        u_vec = [100, 200]
        ch = 'F'
        return 0, c, v4, u_vec, ch

    @staticmethod
    def mock_func21(m, i_vec, v2, flag, d):
        flag = True
        d = 3.14
        v2 = Vector2(1.0, 2.0)
        m = Matrix4x4([
            [1.3, 0.6, 0.8, 0.5],
            [0.7, 1.1, 0.2, 0.4],
            [0.9, 0.3, 1.2, 0.7],
            [0.2, 0.8, 0.5, 1.0]
        ])
        i_vec = [1, 2, 3]
        return 0.0, m, i_vec, v2, flag, d

    @staticmethod
    def mock_func22(p, u32, d_vec, i16, s, v4):
        p = 0
        u32 = 99
        i16 = 123
        s = 'Hello'
        v4 = Vector4(1.0, 2.0, 3.0, 4.0)
        d_vec = [1.1, 2.2, 3.3]
        return 0, p, u32, d_vec, i16, s, v4

    @staticmethod
    def mock_func23(u64, v2, i_vec, c, f, i8, u8_vec):
        u64 = 50
        f = 1.5
        i8 = -1
        v2 = Vector2(3.0, 4.0)
        u8_vec = [1, 2, 3]
        c = 'Ⅴ'
        i_vec = [1, 2, 3, 4]
        return 5, u64, v2, i_vec, c, f, i8, u8_vec

    @staticmethod
    def mock_func24(c_vec, i64, u8_vec, v4, u64, p_vec, d, v_vec):
        i64 = 64
        d = 2.71
        v4 = Vector4(1.0, 2.0, 3.0, 4.0)
        c_vec = ['a', 'b', 'c']
        u8_vec = [5, 6, 7]
        p_vec = [0]
        v_vec = [1, 1, 2, 2]
        u64 = 0xffffffff
        return Matrix4x4.zero(), c_vec, i64, u8_vec, v4, u64, p_vec, d, v_vec

    @staticmethod
    def mock_func25(i32, p_vec, flag, u8, s, v3, i64, v4, u16):
        flag = False
        i32 = 100
        u8 = 250
        v3 = Vector3(1.0, 2.0, 3.0)
        v4 = Vector4(4.0, 5.0, 6.0, 7.0)
        s = 'MockFunc25'
        p_vec = [0]
        i64 = 1337
        u16 = 64222
        return 0.0, i32, p_vec, flag, u8, s, v3, i64, v4, u16

    @staticmethod
    def mock_func26(c, v2, m, f_vec, i16, u64, u32, u16_vec, p, flag):
        c = 'Z'
        flag = True
        v2 = Vector2(2.0, 3.0)
        m = Matrix4x4([
            0.9, 0.2, 0.4, 0.8,
            0.1, 1.0, 0.6, 0.3,
            0.7, 0.5, 0.2, 0.9,
            0.3, 0.4, 1.5, 0.1
        ])
        f_vec = [1.1, 2.2]
        u64 = 64
        u32 = 32
        u16_vec = [100, 200]
        i16 = 332
        p = 0xDEADBEAFDEADBEAF
        return 'A', c, v2, m, f_vec, i16, u64, u32, u16_vec, p, flag

    @staticmethod
    def mock_func27(f, v3, p, v2, i16_vec, m, flag, v4, i8, i32, u8_vec):
        f = 1.0
        v3 = Vector3(-1.0, -2.0, -3.0)
        p = 0xDEADBEAFDEADBEAF
        v2 = Vector2(-111.0, 111.0)
        i16_vec = [1, 2, 3, 4]
        m = Matrix4x4([
            1.0, 0.5, 0.3, 0.7,
            0.8, 1.2, 0.6, 0.9,
            1.5, 1.1, 0.4, 0.2,
            0.3, 0.9, 0.7, 1.0
        ])
        flag = True
        v4 = Vector4(1.0, 2.0, 3.0, 4.0)
        i8 = 111
        i32 = 30
        u8_vec = [0, 0, 0, 0, 0, 0, 1, 0]
        return 0, f, v3, p, v2, i16_vec, m, flag, v4, i8, i32, u8_vec

    @staticmethod
    def mock_func28(ptr, u16, u32_vec, m, f, v4, s, u64_vec, i64, b, vec3, f_vec):
        ptr = 0
        u16 = 65500
        u32_vec = [1, 2, 3, 4, 5, 7]
        m = Matrix4x4([
            1.4, 0.7, 0.2, 0.5,
            0.3, 1.1, 0.6, 0.8,
            0.9, 0.4, 1.3, 0.1,
            0.6, 0.2, 0.7, 1.0
        ])
        f = 5.5
        v4 = Vector4(1.0, 2.0, 3.0, 4.0)
        u64_vec = [1, 2]
        i64 = 834748377834
        b = True
        vec3 = Vector3(10.0, 20.0, 30.0)
        s = 'MockFunc28'
        f_vec = [1.0, -1000.0, 2000.0]
        return s, ptr, u16, u32_vec, m, f, v4, s, u64_vec, i64, b, vec3, f_vec

    @staticmethod
    def mock_func29(v4, i32, i_vec, d, flag, i8, u16_vec, f, s, m, u64, v3, i64_vec):
        i32 = 30
        flag = True
        v4 = Vector4(1.0, 2.0, 3.0, 4.0)
        d = 3.14
        i8 = 8
        u16_vec = [100, 200]
        f = 1.5
        s = 'MockFunc29'
        m = Matrix4x4([
            0.4, 1.0, 0.6, 0.3,
            1.2, 0.8, 0.5, 0.9,
            0.7, 0.3, 1.4, 0.6,
            0.1, 0.9, 0.8, 1.3
        ])
        u64 = 64
        v3 = Vector3(1.0, 2.0, 3.0)
        i64_vec = [1, 2, 3]
        i_vec = [127, 126, 125]
        return ['Example', 'MockFunc29'], v4, i32, i_vec, d, flag, i8, u16_vec, f, s, m, u64, v3, i64_vec

    @staticmethod
    def mock_func30(p, v4, i64, u_vec, flag, s, v3, u8_vec, f, v2, m, i8, v_vec, d):
        flag = False
        f = 1.1
        i64 = 1000
        v2 = Vector2(3.0, 4.0)
        v4 = Vector4(1.0, 2.0, 3.0, 4.0)
        s = 'MockFunc30'
        p = 0
        u_vec = [100, 200]
        m = Matrix4x4([
            0.5, 0.3, 1.0, 0.7,
            1.1, 0.9, 0.6, 0.4,
            0.2, 0.8, 1.5, 0.3,
            0.7, 0.4, 0.9, 1.0
        ])
        i8 = 8
        v_vec = [1.0, 1.0, 2.0, 2.0]
        d = 2.718
        v3 = Vector3(1.0, 2.0, 3.0)
        u8_vec = [255, 0, 255, 200, 100, 200]
        return 42, p, v4, i64, u_vec, flag, s, v3, u8_vec, f, v2, m, i8, v_vec, d

    @staticmethod
    def mock_func31(c, u32, u_vec, v4, s, flag, i64, v2, i8, u16, i_vec, m, v3, f, v4_vec):
        u32 = 12345
        flag = True
        v3 = Vector3(1.0, 2.0, 3.0)
        s = 'MockFunc31'
        v2 = Vector2(5.0, 6.0)
        i8 = 7
        u16 = 255
        m = Matrix4x4([
            0.8, 0.5, 1.2, 0.3,
            1.0, 0.7, 0.4, 0.6,
            0.9, 0.2, 0.5, 1.4,
            0.6, 0.8, 1.1, 0.7
        ])
        i_vec = [1, 2]
        v4 = Vector4(1.0, 2.0, 3.0, 4.0)
        i64 = 123456789
        c = 'C'
        v4_vec = [1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0]
        u_vec = [1, 2, 3, 4, 5]
        f = -1.0
        return Vector3(1.0, 2.0, 3.0), c, u32, u_vec, v4, s, flag, i64, v2, i8, u16, i_vec, m, v3, f, v4_vec

    @staticmethod
    def mock_func32(i32, u16, i_vec, v4, p, u_vec, m, u64, s, i64, v2, u8_vec, flag, v3, u8, c_vec):
        i32 = 42
        u16 = 255
        flag = False
        v2 = Vector2(2.5, 3.5)
        u8_vec = [1, 2, 3, 4, 5, 9]
        v4 = Vector4(4.0, 5.0, 6.0, 7.0)
        s = 'MockFunc32'
        p = 0
        m = Matrix4x4([
            1.0, 0.4, 0.3, 0.9,
            0.7, 1.2, 0.5, 0.8,
            0.2, 0.6, 1.1, 0.4,
            0.9, 0.3, 0.8, 1.5
        ])
        u64 = 123456789
        u_vec = [100, 200]
        i64 = 1000
        v3 = Vector3(0.0, 0.0, 0.0)
        u8 = 8
        c_vec = ['a', 'b', 'c']
        i_vec = [0, 1]
        return 1.0, i32, u16, i_vec, v4, p, u_vec, m, u64, s, i64, v2, u8_vec, flag, v3, u8, c_vec


def reverse_call_func_void():
    pps.cross_call_master.CallFuncVoidCallback(CallbackHolder.mock_void)
    return ''


def reverse_call_func_bool():
    result = pps.cross_call_master.CallFuncBoolCallback(CallbackHolder.mock_bool)
    return bool_str(result)


def reverse_call_func_char8():
    result = pps.cross_call_master.CallFuncChar8Callback(CallbackHolder.mock_char8)
    return f'{ord_zero(result)}'


def reverse_call_func_char16():
    result = pps.cross_call_master.CallFuncChar16Callback(CallbackHolder.mock_char16)
    return f'{ord_zero(result)}'


def reverse_call_func_int8():
    result = pps.cross_call_master.CallFuncInt8Callback(CallbackHolder.mock_int8)
    return str(result)


def reverse_call_func_int16():
    result = pps.cross_call_master.CallFuncInt16Callback(CallbackHolder.mock_int16)
    return str(result)


def reverse_call_func_int32():
    result = pps.cross_call_master.CallFuncInt32Callback(CallbackHolder.mock_int32)
    return str(result)


def reverse_call_func_int64():
    result = pps.cross_call_master.CallFuncInt64Callback(CallbackHolder.mock_int64)
    return str(result)


def reverse_call_func_uint8():
    result = pps.cross_call_master.CallFuncUInt8Callback(CallbackHolder.mock_uint8)
    return str(result)


def reverse_call_func_uint16():
    result = pps.cross_call_master.CallFuncUInt16Callback(CallbackHolder.mock_uint16)
    return str(result)


def reverse_call_func_uint32():
    result = pps.cross_call_master.CallFuncUInt32Callback(CallbackHolder.mock_uint32)
    return str(result)


def reverse_call_func_uint64():
    result = pps.cross_call_master.CallFuncUInt64Callback(CallbackHolder.mock_uint64)
    return str(result)


def reverse_call_func_ptr():
    result = pps.cross_call_master.CallFuncPtrCallback(CallbackHolder.mock_ptr)
    return ptr_str(result)


def reverse_call_func_float():
    result = pps.cross_call_master.CallFuncFloatCallback(CallbackHolder.mock_float)
    return float_str(result)


def reverse_call_func_double():
    result = pps.cross_call_master.CallFuncDoubleCallback(CallbackHolder.mock_double)
    return str(result)


def reverse_call_func_string():
    result = pps.cross_call_master.CallFuncStringCallback(CallbackHolder.mock_string)
    return result


def reverse_call_func_bool_vector():
    result = pps.cross_call_master.CallFuncBoolVectorCallback(CallbackHolder.mock_bool_array)
    return vector_to_string(result, bool_str)


def reverse_call_func_char8_vector():
    result = pps.cross_call_master.CallFuncChar8VectorCallback(CallbackHolder.mock_char8_array)
    return vector_to_string(result, char_str)


def reverse_call_func_char16_vector():
    result = pps.cross_call_master.CallFuncChar16VectorCallback(CallbackHolder.mock_char16_array)
    return vector_to_string(result, char_str)


def reverse_call_func_int8_vector():
    result = pps.cross_call_master.CallFuncInt8VectorCallback(CallbackHolder.mock_int8_array)
    return vector_to_string(result)


def reverse_call_func_int16_vector():
    result = pps.cross_call_master.CallFuncInt16VectorCallback(CallbackHolder.mock_int16_array)
    return vector_to_string(result)


def reverse_call_func_int32_vector():
    result = pps.cross_call_master.CallFuncInt32VectorCallback(CallbackHolder.mock_int32_array)
    return vector_to_string(result)


def reverse_call_func_int64_vector():
    result = pps.cross_call_master.CallFuncInt64VectorCallback(CallbackHolder.mock_int64_array)
    return vector_to_string(result)


def reverse_call_func_uint8_vector():
    result = pps.cross_call_master.CallFuncUInt8VectorCallback(CallbackHolder.mock_uint8_array)
    return vector_to_string(result)


def reverse_call_func_uint16_vector():
    result = pps.cross_call_master.CallFuncUInt16VectorCallback(CallbackHolder.mock_uint16_array)
    return vector_to_string(result)


def reverse_call_func_uint32_vector():
    result = pps.cross_call_master.CallFuncUInt32VectorCallback(CallbackHolder.mock_uint32_array)
    return vector_to_string(result)


def reverse_call_func_uint64_vector():
    result = pps.cross_call_master.CallFuncUInt64VectorCallback(CallbackHolder.mock_uint64_array)
    return vector_to_string(result)


def reverse_call_func_ptr_vector():
    result = pps.cross_call_master.CallFuncPtrVectorCallback(CallbackHolder.mock_ptr_array)
    return vector_to_string(result, ptr_str)


def reverse_call_func_float_vector():
    result = pps.cross_call_master.CallFuncFloatVectorCallback(CallbackHolder.mock_float_array)
    return vector_to_string(result, float_str)


def reverse_call_func_double_vector():
    result = pps.cross_call_master.CallFuncDoubleVectorCallback(CallbackHolder.mock_double_array)
    return vector_to_string(result)


def reverse_call_func_string_vector():
    result = pps.cross_call_master.CallFuncStringVectorCallback(CallbackHolder.mock_string_array)
    return vector_to_string(result, quote_str)


def reverse_call_func_vec2():
    result = pps.cross_call_master.CallFuncVec2Callback(CallbackHolder.mock_vec2)
    return pod_to_string(result)


def reverse_call_func_vec3():
    result = pps.cross_call_master.CallFuncVec3Callback(CallbackHolder.mock_vec3)
    return pod_to_string(result)


def reverse_call_func_vec4():
    result = pps.cross_call_master.CallFuncVec4Callback(CallbackHolder.mock_vec4)
    return pod_to_string(result)


def reverse_call_func_mat4x4():
    result = pps.cross_call_master.CallFuncMat4x4Callback(CallbackHolder.mock_mat4x4)
    return pod_to_string(result)


def reverse_call_func1():
    result = pps.cross_call_master.CallFunc1Callback(CallbackHolder.mock_func1)
    return str(result)


def reverse_call_func2():
    result = pps.cross_call_master.CallFunc2Callback(CallbackHolder.mock_func2)
    return char_str(result)


def reverse_call_func3():
    pps.cross_call_master.CallFunc3Callback(CallbackHolder.mock_func3)
    return ''


def reverse_call_func4():
    result = pps.cross_call_master.CallFunc4Callback(CallbackHolder.mock_func4)
    return pod_to_string(result)


def reverse_call_func5():
    result = pps.cross_call_master.CallFunc5Callback(CallbackHolder.mock_func5)
    return bool_str(result)


def reverse_call_func6():
    result = pps.cross_call_master.CallFunc6Callback(CallbackHolder.mock_func6)
    return str(result)


def reverse_call_func7():
    result = pps.cross_call_master.CallFunc7Callback(CallbackHolder.mock_func7)
    return str(result)


def reverse_call_func8():
    result = pps.cross_call_master.CallFunc8Callback(CallbackHolder.mock_func8)
    return pod_to_string(result)


def reverse_call_func9():
    pps.cross_call_master.CallFunc9Callback(CallbackHolder.mock_func9)
    return ''


def reverse_call_func10():
    result = pps.cross_call_master.CallFunc10Callback(CallbackHolder.mock_func10)
    return str(result)


def reverse_call_func11():
    result = pps.cross_call_master.CallFunc11Callback(CallbackHolder.mock_func11)
    return ptr_str(result)


def reverse_call_func12():
    result = pps.cross_call_master.CallFunc12Callback(CallbackHolder.mock_func12)
    return bool_str(result)


def reverse_call_func13():
    result = pps.cross_call_master.CallFunc13Callback(CallbackHolder.mock_func13)
    return result


def reverse_call_func14():
    result = pps.cross_call_master.CallFunc14Callback(CallbackHolder.mock_func14)
    return vector_to_string(result, quote_str)


def reverse_call_func15():
    result = pps.cross_call_master.CallFunc15Callback(CallbackHolder.mock_func15)
    return str(result)


def reverse_call_func16():
    result = pps.cross_call_master.CallFunc16Callback(CallbackHolder.mock_func16)
    return ptr_str(result)


def reverse_call_func17():
    result = pps.cross_call_master.CallFunc17Callback(CallbackHolder.mock_func17)
    return result


def reverse_call_func18():
    result = pps.cross_call_master.CallFunc18Callback(CallbackHolder.mock_func18)
    return result


def reverse_call_func19():
    result = pps.cross_call_master.CallFunc19Callback(CallbackHolder.mock_func19)
    return result


def reverse_call_func20():
    result = pps.cross_call_master.CallFunc20Callback(CallbackHolder.mock_func20)
    return result


def reverse_call_func21():
    result = pps.cross_call_master.CallFunc21Callback(CallbackHolder.mock_func21)
    return result


def reverse_call_func22():
    result = pps.cross_call_master.CallFunc22Callback(CallbackHolder.mock_func22)
    return result


def reverse_call_func23():
    result = pps.cross_call_master.CallFunc23Callback(CallbackHolder.mock_func23)
    return result


def reverse_call_func24():
    result = pps.cross_call_master.CallFunc24Callback(CallbackHolder.mock_func24)
    return result


def reverse_call_func25():
    result = pps.cross_call_master.CallFunc25Callback(CallbackHolder.mock_func25)
    return result


def reverse_call_func26():
    result = pps.cross_call_master.CallFunc26Callback(CallbackHolder.mock_func26)
    return result


def reverse_call_func27():
    result = pps.cross_call_master.CallFunc27Callback(CallbackHolder.mock_func27)
    return result


def reverse_call_func28():
    result = pps.cross_call_master.CallFunc28Callback(CallbackHolder.mock_func28)
    return result


def reverse_call_func29():
    result = pps.cross_call_master.CallFunc29Callback(CallbackHolder.mock_func29)
    return result


def reverse_call_func30():
    result = pps.cross_call_master.CallFunc30Callback(CallbackHolder.mock_func30)
    return result


def reverse_call_func31():
    result = pps.cross_call_master.CallFunc31Callback(CallbackHolder.mock_func31)
    return result


def reverse_call_func32():
    result = pps.cross_call_master.CallFunc32Callback(CallbackHolder.mock_func32)
    return result


reverse_test = {
    'NoParamReturnVoid': reverse_no_param_return_void,
    'NoParamReturnBool': reverse_no_param_return_bool,
    'NoParamReturnChar8': reverse_no_param_return_char8,
    'NoParamReturnChar16': reverse_no_param_return_char16,
    'NoParamReturnInt8': reverse_no_param_return_int8,
    'NoParamReturnInt16': reverse_no_param_return_int16,
    'NoParamReturnInt32': reverse_no_param_return_int32,
    'NoParamReturnInt64': reverse_no_param_return_int64,
    'NoParamReturnUInt8': reverse_no_param_return_uint8,
    'NoParamReturnUInt16': reverse_no_param_return_uint16,
    'NoParamReturnUInt32': reverse_no_param_return_uint32,
    'NoParamReturnUInt64': reverse_no_param_return_uint64,
    'NoParamReturnPointer': reverse_no_param_return_pointer,
    'NoParamReturnFloat': reverse_no_param_return_float,
    'NoParamReturnDouble': reverse_no_param_return_double,
    'NoParamReturnFunction': reverse_no_param_return_function,
    'NoParamReturnString': reverse_no_param_return_string,
    'NoParamReturnArrayBool': reverse_no_param_return_array_bool,
    'NoParamReturnArrayChar8': reverse_no_param_return_array_char8,
    'NoParamReturnArrayChar16': reverse_no_param_return_array_char16,
    'NoParamReturnArrayInt8': reverse_no_param_return_array_int8,
    'NoParamReturnArrayInt16': reverse_no_param_return_array_int16,
    'NoParamReturnArrayInt32': reverse_no_param_return_array_int32,
    'NoParamReturnArrayInt64': reverse_no_param_return_array_int64,
    'NoParamReturnArrayUInt8': reverse_no_param_return_array_uint8,
    'NoParamReturnArrayUInt16': reverse_no_param_return_array_uint16,
    'NoParamReturnArrayUInt32': reverse_no_param_return_array_uint32,
    'NoParamReturnArrayUInt64': reverse_no_param_return_array_uint64,
    'NoParamReturnArrayPointer': reverse_no_param_return_array_pointer,
    'NoParamReturnArrayFloat': reverse_no_param_return_array_float,
    'NoParamReturnArrayDouble': reverse_no_param_return_array_double,
    'NoParamReturnArrayString': reverse_no_param_return_array_string,
    'NoParamReturnVector2': reverse_no_param_return_vector2,
    'NoParamReturnVector3': reverse_no_param_return_vector3,
    'NoParamReturnVector4': reverse_no_param_return_vector4,
    'NoParamReturnMatrix4x4': reverse_no_param_return_matrix4x4,
    'Param1': reverse_param1,
    'Param2': reverse_param2,
    'Param3': reverse_param3,
    'Param4': reverse_param4,
    'Param5': reverse_param5,
    'Param6': reverse_param6,
    'Param7': reverse_param7,
    'Param8': reverse_param8,
    'Param9': reverse_param9,
    'Param10': reverse_param10,
    'ParamRef1': reverse_param_ref1,
    'ParamRef2': reverse_param_ref2,
    'ParamRef3': reverse_param_ref3,
    'ParamRef4': reverse_param_ref4,
    'ParamRef5': reverse_param_ref5,
    'ParamRef6': reverse_param_ref6,
    'ParamRef7': reverse_param_ref7,
    'ParamRef8': reverse_param_ref8,
    'ParamRef9': reverse_param_ref9,
    'ParamRef10': reverse_param_ref10,
    'ParamRefArrays': reverse_param_ref_vectors,
    'ParamAllPrimitives': reverse_param_all_primitives,
    'CallFuncVoid': reverse_call_func_void,
    'CallFuncBool': reverse_call_func_bool,
    'CallFuncChar8': reverse_call_func_char8,
    'CallFuncChar16': reverse_call_func_char16,
    'CallFuncInt8': reverse_call_func_int8,
    'CallFuncInt16': reverse_call_func_int16,
    'CallFuncInt32': reverse_call_func_int32,
    'CallFuncInt64': reverse_call_func_int64,
    'CallFuncUInt8': reverse_call_func_uint8,
    'CallFuncUInt16': reverse_call_func_uint16,
    'CallFuncUInt32': reverse_call_func_uint32,
    'CallFuncUInt64': reverse_call_func_uint64,
    'CallFuncPtr': reverse_call_func_ptr,
    'CallFuncFloat': reverse_call_func_float,
    'CallFuncDouble': reverse_call_func_double,
    'CallFuncString': reverse_call_func_string,
    'CallFuncBoolVector': reverse_call_func_bool_vector,
    'CallFuncChar8Vector': reverse_call_func_char8_vector,
    'CallFuncChar16Vector': reverse_call_func_char16_vector,
    'CallFuncInt8Vector': reverse_call_func_int8_vector,
    'CallFuncInt16Vector': reverse_call_func_int16_vector,
    'CallFuncInt32Vector': reverse_call_func_int32_vector,
    'CallFuncInt64Vector': reverse_call_func_int64_vector,
    'CallFuncUInt8Vector': reverse_call_func_uint8_vector,
    'CallFuncUInt16Vector': reverse_call_func_uint16_vector,
    'CallFuncUInt32Vector': reverse_call_func_uint32_vector,
    'CallFuncUInt64Vector': reverse_call_func_uint64_vector,
    'CallFuncPtrVector': reverse_call_func_ptr_vector,
    'CallFuncFloatVector': reverse_call_func_float_vector,
    'CallFuncDoubleVector': reverse_call_func_double_vector,
    'CallFuncStringVector': reverse_call_func_string_vector,
    'CallFuncVec2': reverse_call_func_vec2,
    'CallFuncVec3': reverse_call_func_vec3,
    'CallFuncVec4': reverse_call_func_vec4,
    'CallFuncMat4x4': reverse_call_func_mat4x4,
    'CallFunc1': reverse_call_func1,
    'CallFunc2': reverse_call_func2,
    'CallFunc3': reverse_call_func3,
    'CallFunc4': reverse_call_func4,
    'CallFunc5': reverse_call_func5,
    'CallFunc6': reverse_call_func6,
    'CallFunc7': reverse_call_func7,
    'CallFunc8': reverse_call_func8,
    'CallFunc9': reverse_call_func9,
    'CallFunc10': reverse_call_func10,
    'CallFunc11': reverse_call_func11,
    'CallFunc12': reverse_call_func12,
    'CallFunc13': reverse_call_func13,
    'CallFunc14': reverse_call_func14,
    'CallFunc15': reverse_call_func15,
    'CallFunc16': reverse_call_func16,
    'CallFunc17': reverse_call_func17,
    'CallFunc18': reverse_call_func18,
    'CallFunc19': reverse_call_func19,
    'CallFunc20': reverse_call_func20,
    'CallFunc21': reverse_call_func21,
    'CallFunc22': reverse_call_func22,
    'CallFunc23': reverse_call_func23,
    'CallFunc24': reverse_call_func24,
    'CallFunc25': reverse_call_func25,
    'CallFunc26': reverse_call_func26,
    'CallFunc27': reverse_call_func27,
    'CallFunc28': reverse_call_func28,
    'CallFunc29': reverse_call_func29,
    'CallFunc30': reverse_call_func30,
    'CallFunc31': reverse_call_func31,
    'CallFunc32': reverse_call_func32,
}


def reverse_call(test):
    result = reverse_test[test]()
    if result is not None:
        pps.cross_call_master.ReverseReturn(result)
