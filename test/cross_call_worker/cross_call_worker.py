import sys
from plugify.plugin import Plugin, PluginInfo, Vector2, Vector3, Vector4, Matrix4x4

__plugin__ = PluginInfo('CrossCallWorker')


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
