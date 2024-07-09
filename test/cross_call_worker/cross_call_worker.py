import sys
from plugify.plugin import Plugin, Vector2, Vector3, Vector4, Matrix4x4
from plugify import pps


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


def ord_zero(ch: str):
    return 0 if len(ch) == 0 else ord(ch)


def strip_zero(fl: str):
    return f'{float(fl)}'


def reverse_no_param_return_void():
    pps.cross_call_master.NoParamReturnVoidCallback()


def reverse_no_param_return_bool():
    result = pps.cross_call_master.NoParamReturnBoolCallback()
    return 'true' if result is True else ('false' if result is False else '<wrong return>')


def reverse_no_param_return_char8():
    result = pps.cross_call_master.NoParamReturnChar8Callback()
    return f'{ord_zero(result)}'


def reverse_no_param_return_char16():
    result = pps.cross_call_master.NoParamReturnChar16Callback()
    return f'{ord_zero(result)}'


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
    return f'{result:#x}'


def reverse_no_param_return_float():
    result = pps.cross_call_master.NoParamReturnFloatCallback()
    return f'{result:.3f}'


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
    return f"{{{', '.join([f'{v}'.lower() for v in result])}}}"


def reverse_no_param_return_array_char8():
    result = pps.cross_call_master.NoParamReturnArrayChar8Callback()
    return f"{{{', '.join([f'{ord_zero(v)}' for v in result])}}}"


def reverse_no_param_return_array_char16():
    result = pps.cross_call_master.NoParamReturnArrayChar16Callback()
    return f"{{{', '.join([f'{ord_zero(v)}' for v in result])}}}"


def reverse_no_param_return_array_int8():
    result = pps.cross_call_master.NoParamReturnArrayInt8Callback()
    return f"{{{', '.join([f'{v}' for v in result])}}}"


def reverse_no_param_return_array_int16():
    result = pps.cross_call_master.NoParamReturnArrayInt16Callback()
    return f"{{{', '.join([f'{v}' for v in result])}}}"


def reverse_no_param_return_array_int32():
    result = pps.cross_call_master.NoParamReturnArrayInt32Callback()
    return f"{{{', '.join([f'{v}' for v in result])}}}"


def reverse_no_param_return_array_int64():
    result = pps.cross_call_master.NoParamReturnArrayInt64Callback()
    return f"{{{', '.join([f'{v}' for v in result])}}}"


def reverse_no_param_return_array_uint8():
    result = pps.cross_call_master.NoParamReturnArrayUInt8Callback()
    return f"{{{', '.join([f'{v}' for v in result])}}}"


def reverse_no_param_return_array_uint16():
    result = pps.cross_call_master.NoParamReturnArrayUInt16Callback()
    return f"{{{', '.join([f'{v}' for v in result])}}}"


def reverse_no_param_return_array_uint32():
    result = pps.cross_call_master.NoParamReturnArrayUInt32Callback()
    return f"{{{', '.join([f'{v}' for v in result])}}}"


def reverse_no_param_return_array_uint64():
    result = pps.cross_call_master.NoParamReturnArrayUInt64Callback()
    return f"{{{', '.join([f'{v}' for v in result])}}}"


def reverse_no_param_return_array_pointer():
    result = pps.cross_call_master.NoParamReturnArrayPointerCallback()
    return f"{{{', '.join([f'{v:#x}' for v in result])}}}"


def reverse_no_param_return_array_float():
    result = pps.cross_call_master.NoParamReturnArrayFloatCallback()
    return f"{{{', '.join([strip_zero(f'{v:.3f}') for v in result])}}}"


def reverse_no_param_return_array_double():
    result = pps.cross_call_master.NoParamReturnArrayDoubleCallback()
    return f"{{{', '.join([f'{v}' for v in result])}}}"


def reverse_no_param_return_array_string():
    result = pps.cross_call_master.NoParamReturnArrayStringCallback()
    result_formatted = ', '.join([f"'{v}'" for v in result])
    return f"{{{result_formatted}}}"


def reverse_no_param_return_vector2():
    result = pps.cross_call_master.NoParamReturnVector2Callback()
    return f'{{{result.x:.1f}, {result.y:.1f}}}'


def reverse_no_param_return_vector3():
    result = pps.cross_call_master.NoParamReturnVector3Callback()
    return f'{{{result.x:.1f}, {result.y:.1f}, {result.z:.1f}}}'


def reverse_no_param_return_vector4():
    result = pps.cross_call_master.NoParamReturnVector4Callback()
    return f'{{{result.x:.1f}, {result.y:.1f}, {result.z:.1f}, {result.w:.1f}}}'


def reverse_no_param_return_matrix4x4():
    result = pps.cross_call_master.NoParamReturnMatrix4x4Callback()
    formatted_rows = [f"{{{', '.join([f'{m:.1f}' for m in row])}}}" for row in result.elements]
    return f'{{{", ".join(formatted_rows)}}}'


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
    return f'{a}|{b:.1f}'


def reverse_param_ref3():
    _, a, b, c = pps.cross_call_master.ParamRef3Callback(0, 0.0, 0.0)
    return f'{a}|{b:.1f}|{c}'


def reverse_param_ref4():
    _, a, b, c, d = pps.cross_call_master.ParamRef4Callback(0, 0.0, 0.0, Vector4())
    return f'{a}|{b:.1f}|{c}|{{{d.x:.1f}, {d.y:.1f}, {d.z:.1f}, {d.w:.1f}}}'


def reverse_param_ref5():
    _, a, b, c, d, e = pps.cross_call_master.ParamRef5Callback(0, 0.0, 0.0, Vector4(), [])
    e_formatted = ', '.join([f'{v}' for v in e])
    return f'{a}|{b:.1f}|{c}|{{{d.x:.1f}, {d.y:.1f}, {d.z:.1f}, {d.w:.1f}}}|{{{e_formatted}}}'


def reverse_param_ref6():
    _, a, b, c, d, e, f = pps.cross_call_master.ParamRef6Callback(0, 0.0, 0.0, Vector4(), [], '')
    e_formatted = ', '.join([f'{v}' for v in e])
    return f'{a}|{b:.1f}|{c}|{{{d.x:.1f}, {d.y:.1f}, {d.z:.1f}, {d.w:.1f}}}|{{{e_formatted}}}|{ord_zero(f)}'


def reverse_param_ref7():
    _, a, b, c, d, e, f, g = pps.cross_call_master.ParamRef7Callback(0, 0.0, 0.0, Vector4(), [], '', '')
    e_formatted = ', '.join([f'{v}' for v in e])
    return f'{a}|{b:.1f}|{c}|{{{d.x:.1f}, {d.y:.1f}, {d.z:.1f}, {d.w:.1f}}}|{{{e_formatted}}}|{ord_zero(f)}|{g}'


def reverse_param_ref8():
    _, a, b, c, d, e, f, g, h = pps.cross_call_master.ParamRef8Callback(0, 0.0, 0.0, Vector4(), [], '', '', '')
    e_formatted = ', '.join([f'{v}' for v in e])
    return f'{a}|{b:.1f}|{c}|{{{d.x:.1f}, {d.y:.1f}, {d.z:.1f}, {d.w:.1f}}}|{{{e_formatted}}}|{ord_zero(f)}|{g}|{ord_zero(h)}'


def reverse_param_ref9():
    _, a, b, c, d, e, f, g, h, k = pps.cross_call_master.ParamRef9Callback(0, 0.0, 0.0, Vector4(), [], '', '', '', 0)
    e_formatted = ', '.join([f'{v}' for v in e])
    return f'{a}|{b:.1f}|{c}|{{{d.x:.1f}, {d.y:.1f}, {d.z:.1f}, {d.w:.1f}}}|{{{e_formatted}}}|{ord_zero(f)}|{g}|{ord_zero(h)}|{k}'


def reverse_param_ref10():
    _, a, b, c, d, e, f, g, h, k, l = pps.cross_call_master.ParamRef10Callback(0, 0.0, 0.0, Vector4(), [], '', '', '', 0, 0)
    e_formatted = ', '.join([f'{v}' for v in e])
    return f'{a}|{b:.1f}|{c}|{{{d.x:.1f}, {d.y:.1f}, {d.z:.1f}, {d.w:.1f}}}|{{{e_formatted}}}|{ord_zero(f)}|{g}|{ord_zero(h)}|{k}|{l:#x}'


def reverse_param_ref_vectors():
    _, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 = pps.cross_call_master.ParamRefVectorsCallback(
        [True], ['A'], ['A'], [-1], [-1], [-1], [-1], [0], [0], [0], [0], [0], [1.0], [1.0], ['Hi']
    )
    p1_formatted = ', '.join([f'{v}'.lower() for v in p1])
    p2_formatted = ', '.join([f'{ord_zero(v)}' for v in p2])
    p3_formatted = ', '.join([f'{ord_zero(v)}' for v in p3])
    p4_formatted = ', '.join([f'{v}' for v in p4])
    p5_formatted = ', '.join([f'{v}' for v in p5])
    p6_formatted = ', '.join([f'{v}' for v in p6])
    p7_formatted = ', '.join([f'{v}' for v in p7])
    p8_formatted = ', '.join([f'{v}' for v in p8])
    p9_formatted = ', '.join([f'{v}' for v in p9])
    p10_formatted = ', '.join([f'{v}' for v in p10])
    p11_formatted = ', '.join([f'{v}' for v in p11])
    p12_formatted = ', '.join([f'{v:#x}' for v in p12])
    p13_formatted = ', '.join([f'{v:.2f}' for v in p13])
    p14_formatted = ', '.join([f'{v}' for v in p14])
    p15_formatted = ', '.join([f"'{v}'" for v in p15])
    return f'{{{p1_formatted}}}|{{{p2_formatted}}}|{{{p3_formatted}}}|{{{p4_formatted}}}|{{{p5_formatted}}}|' \
           f'{{{p6_formatted}}}|{{{p7_formatted}}}|{{{p8_formatted}}}|{{{p9_formatted}}}|{{{p10_formatted}}}|' \
           f'{{{p11_formatted}}}|{{{p12_formatted}}}|{{{p13_formatted}}}|{{{p14_formatted}}}|{{{p15_formatted}}}'


def reverse_param_all_primitives():
    result = pps.cross_call_master.ParamAllPrimitivesCallback(True, '%', '☢', -1, -1000, -1000000, -1000000000000,
                                                              200, 50000, 3000000000, 9999999999, 0xfedcbaabcdef,
                                                              0.001, 987654.456789)
    return f'{result}'


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
}


def reverse_call(test):
    result = reverse_test[test]()
    if result is not None:
        pps.cross_call_master.ReverseReturn(result)
