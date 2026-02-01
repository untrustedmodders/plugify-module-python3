from collections.abc import Callable
from enum import IntEnum
from plugify.plugin import Vector2, Vector3, Vector4, Matrix4x4

# Generated from cross_call_master.pplugin

class Example(IntEnum):
    First = 1
    Second = 2
    Third = 3
    Forth = 4


AliasBool = bool

AliasChar8 = str

AliasChar16 = str

AliasInt8 = int

AliasInt16 = int

AliasInt32 = int

AliasInt64 = int

AliasPtr = int

AliasFloat = float

AliasDouble = float

AliasString = str

AliasAny = object

AliasVec2 = Vector2

AliasVec3 = Vector3

AliasVec4 = Vector4

AliasMat4x4 = Matrix4x4

AliasBoolVector = list[bool]

AliasChar8Vector = list[str]

AliasChar16Vector = list[str]

AliasInt8Vector = list[int]

AliasInt16Vector = list[int]

AliasInt32Vector = list[int]

AliasInt64Vector = list[int]

AliasPtrVector = list[int]

AliasFloatVector = list[float]

AliasDoubleVector = list[float]

AliasStringVector = list[str]

AliasAnyVector = list[object]

AliasVec2Vector = list[Vector2]

AliasVec3Vector = list[Vector3]

AliasVec4Vector = list[Vector4]

AliasUInt8 = int

AliasUInt16 = int

AliasUInt32 = int

AliasUInt64 = int

AliasFunction = int

AliasUInt8Vector = list[int]

AliasUInt16Vector = list[int]

AliasUInt32Vector = list[int]

AliasUInt64Vector = list[int]

AliasMat4x4Vector = list[Matrix4x4]


def ReverseReturn(returnString: str) -> None:
    """

    Args:
        returnString (string):
    """
    ...

def NoParamReturnVoidCallback() -> None:
    """
    """
    ...

def NoParamReturnBoolCallback() -> bool:
    """

    Returns:
        bool:
    """
    ...

def NoParamReturnChar8Callback() -> str:
    """

    Returns:
        char8:
    """
    ...

def NoParamReturnChar16Callback() -> str:
    """

    Returns:
        char16:
    """
    ...

def NoParamReturnInt8Callback() -> int:
    """

    Returns:
        int8:
    """
    ...

def NoParamReturnInt16Callback() -> int:
    """

    Returns:
        int16:
    """
    ...

def NoParamReturnInt32Callback() -> int:
    """

    Returns:
        int32:
    """
    ...

def NoParamReturnInt64Callback() -> int:
    """

    Returns:
        int64:
    """
    ...

def NoParamReturnUInt8Callback() -> int:
    """

    Returns:
        uint8:
    """
    ...

def NoParamReturnUInt16Callback() -> int:
    """

    Returns:
        uint16:
    """
    ...

def NoParamReturnUInt32Callback() -> int:
    """

    Returns:
        uint32:
    """
    ...

def NoParamReturnUInt64Callback() -> int:
    """

    Returns:
        uint64:
    """
    ...

def NoParamReturnPointerCallback() -> int:
    """

    Returns:
        ptr64:
    """
    ...

def NoParamReturnFloatCallback() -> float:
    """

    Returns:
        float:
    """
    ...

def NoParamReturnDoubleCallback() -> float:
    """

    Returns:
        double:
    """
    ...

def NoParamReturnFunctionCallback() -> Callable[[], int]:
    """

    Returns:
        function:
    """
    ...

def NoParamReturnStringCallback() -> str:
    """

    Returns:
        string:
    """
    ...

def NoParamReturnAnyCallback() -> object:
    """

    Returns:
        any:
    """
    ...

def NoParamReturnArrayBoolCallback() -> list[bool]:
    """

    Returns:
        bool[]:
    """
    ...

def NoParamReturnArrayChar8Callback() -> list[str]:
    """

    Returns:
        char8[]:
    """
    ...

def NoParamReturnArrayChar16Callback() -> list[str]:
    """

    Returns:
        char16[]:
    """
    ...

def NoParamReturnArrayInt8Callback() -> list[int]:
    """

    Returns:
        int8[]:
    """
    ...

def NoParamReturnArrayInt16Callback() -> list[int]:
    """

    Returns:
        int16[]:
    """
    ...

def NoParamReturnArrayInt32Callback() -> list[int]:
    """

    Returns:
        int32[]:
    """
    ...

def NoParamReturnArrayInt64Callback() -> list[int]:
    """

    Returns:
        int64[]:
    """
    ...

def NoParamReturnArrayUInt8Callback() -> list[int]:
    """

    Returns:
        uint8[]:
    """
    ...

def NoParamReturnArrayUInt16Callback() -> list[int]:
    """

    Returns:
        uint16[]:
    """
    ...

def NoParamReturnArrayUInt32Callback() -> list[int]:
    """

    Returns:
        uint32[]:
    """
    ...

def NoParamReturnArrayUInt64Callback() -> list[int]:
    """

    Returns:
        uint64[]:
    """
    ...

def NoParamReturnArrayPointerCallback() -> list[int]:
    """

    Returns:
        ptr64[]:
    """
    ...

def NoParamReturnArrayFloatCallback() -> list[float]:
    """

    Returns:
        float[]:
    """
    ...

def NoParamReturnArrayDoubleCallback() -> list[float]:
    """

    Returns:
        double[]:
    """
    ...

def NoParamReturnArrayStringCallback() -> list[str]:
    """

    Returns:
        string[]:
    """
    ...

def NoParamReturnArrayAnyCallback() -> list[object]:
    """

    Returns:
        any[]:
    """
    ...

def NoParamReturnArrayVector2Callback() -> list[Vector2]:
    """

    Returns:
        vec2[]:
    """
    ...

def NoParamReturnArrayVector3Callback() -> list[Vector3]:
    """

    Returns:
        vec3[]:
    """
    ...

def NoParamReturnArrayVector4Callback() -> list[Vector4]:
    """

    Returns:
        vec4[]:
    """
    ...

def NoParamReturnArrayMatrix4x4Callback() -> list[Matrix4x4]:
    """

    Returns:
        mat4x4[]:
    """
    ...

def NoParamReturnVector2Callback() -> Vector2:
    """

    Returns:
        vec2:
    """
    ...

def NoParamReturnVector3Callback() -> Vector3:
    """

    Returns:
        vec3:
    """
    ...

def NoParamReturnVector4Callback() -> Vector4:
    """

    Returns:
        vec4:
    """
    ...

def NoParamReturnMatrix4x4Callback() -> Matrix4x4:
    """

    Returns:
        mat4x4:
    """
    ...

def Param1Callback(a: int) -> None:
    """

    Args:
        a (int32):
    """
    ...

def Param2Callback(a: int, b: float) -> None:
    """

    Args:
        a (int32):
        b (float):
    """
    ...

def Param3Callback(a: int, b: float, c: float) -> None:
    """

    Args:
        a (int32):
        b (float):
        c (double):
    """
    ...

def Param4Callback(a: int, b: float, c: float, d: Vector4) -> None:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
    """
    ...

def Param5Callback(a: int, b: float, c: float, d: Vector4, e: list[int]) -> None:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
    """
    ...

def Param6Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str) -> None:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
    """
    ...

def Param7Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str, g: str) -> None:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
        g (string):
    """
    ...

def Param8Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str, g: str, h: str) -> None:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
        g (string):
        h (char16):
    """
    ...

def Param9Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str, g: str, h: str, k: int) -> None:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
        g (string):
        h (char16):
        k (int16):
    """
    ...

def Param10Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str, g: str, h: str, k: int, l: int) -> None:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
        g (string):
        h (char16):
        k (int16):
        l (ptr64):
    """
    ...

def ParamRef1Callback(a: int) -> tuple[None, int]:
    """

    Args:
        a (int32):
    """
    ...

def ParamRef2Callback(a: int, b: float) -> tuple[None, int, float]:
    """

    Args:
        a (int32):
        b (float):
    """
    ...

def ParamRef3Callback(a: int, b: float, c: float) -> tuple[None, int, float, float]:
    """

    Args:
        a (int32):
        b (float):
        c (double):
    """
    ...

def ParamRef4Callback(a: int, b: float, c: float, d: Vector4) -> tuple[None, int, float, float, Vector4]:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
    """
    ...

def ParamRef5Callback(a: int, b: float, c: float, d: Vector4, e: list[int]) -> tuple[None, int, float, float, Vector4, list[int]]:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
    """
    ...

def ParamRef6Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str) -> tuple[None, int, float, float, Vector4, list[int], str]:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
    """
    ...

def ParamRef7Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str, g: str) -> tuple[None, int, float, float, Vector4, list[int], str, str]:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
        g (string):
    """
    ...

def ParamRef8Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str, g: str, h: str) -> tuple[None, int, float, float, Vector4, list[int], str, str, str]:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
        g (string):
        h (char16):
    """
    ...

def ParamRef9Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str, g: str, h: str, k: int) -> tuple[None, int, float, float, Vector4, list[int], str, str, str, int]:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
        g (string):
        h (char16):
        k (int16):
    """
    ...

def ParamRef10Callback(a: int, b: float, c: float, d: Vector4, e: list[int], f: str, g: str, h: str, k: int, l: int) -> tuple[None, int, float, float, Vector4, list[int], str, str, str, int, int]:
    """

    Args:
        a (int32):
        b (float):
        c (double):
        d (vec4):
        e (int64[]):
        f (char8):
        g (string):
        h (char16):
        k (int16):
        l (ptr64):
    """
    ...

def ParamRefVectorsCallback(p1: list[bool], p2: list[str], p3: list[str], p4: list[int], p5: list[int], p6: list[int], p7: list[int], p8: list[int], p9: list[int], p10: list[int], p11: list[int], p12: list[int], p13: list[float], p14: list[float], p15: list[str]) -> tuple[None, list[bool], list[str], list[str], list[int], list[int], list[int], list[int], list[int], list[int], list[int], list[int], list[int], list[float], list[float], list[str]]:
    """

    Args:
        p1 (bool[]):
        p2 (char8[]):
        p3 (char16[]):
        p4 (int8[]):
        p5 (int16[]):
        p6 (int32[]):
        p7 (int64[]):
        p8 (uint8[]):
        p9 (uint16[]):
        p10 (uint32[]):
        p11 (uint64[]):
        p12 (ptr64[]):
        p13 (float[]):
        p14 (double[]):
        p15 (string[]):
    """
    ...

def ParamAllPrimitivesCallback(p1: bool, p2: str, p3: str, p4: int, p5: int, p6: int, p7: int, p8: int, p9: int, p10: int, p11: int, p12: int, p13: float, p14: float) -> int:
    """

    Args:
        p1 (bool):
        p2 (char8):
        p3 (char16):
        p4 (int8):
        p5 (int16):
        p6 (int32):
        p7 (int64):
        p8 (uint8):
        p9 (uint16):
        p10 (uint32):
        p11 (uint64):
        p12 (ptr64):
        p13 (float):
        p14 (double):

    Returns:
        int64:
    """
    ...

def ParamAllAliasesCallback(aBool: AliasBool, aChar8: AliasChar8, aChar16: AliasChar16, aInt8: AliasInt8, aInt16: AliasInt16, aInt32: AliasInt32, aInt64: AliasInt64, aPtr: AliasPtr, aFloat: AliasFloat, aDouble: AliasDouble, aString: AliasString, aAny: AliasAny, aVec2: AliasVec2, aVec3: AliasVec3, aVec4: AliasVec4, aMat4x4: AliasMat4x4, aBoolVec: AliasBoolVector, aChar8Vec: AliasChar8Vector, aChar16Vec: AliasChar16Vector, aInt8Vec: AliasInt8Vector, aInt16Vec: AliasInt16Vector, aInt32Vec: AliasInt32Vector, aInt64Vec: AliasInt64Vector, aPtrVec: AliasPtrVector, aFloatVec: AliasFloatVector, aDoubleVec: AliasDoubleVector, aStringVec: AliasStringVector, aAnyVec: AliasAnyVector, aVec2Vec: AliasVec2Vector, aVec3Vec: AliasVec3Vector, aVec4Vec: AliasVec4Vector) -> int:
    """

    Args:
        aBool (bool):
        aChar8 (char8):
        aChar16 (char16):
        aInt8 (int8):
        aInt16 (int16):
        aInt32 (int32):
        aInt64 (int64):
        aPtr (ptr64):
        aFloat (float):
        aDouble (double):
        aString (string):
        aAny (any):
        aVec2 (vec2):
        aVec3 (vec3):
        aVec4 (vec4):
        aMat4x4 (mat4x4):
        aBoolVec (bool[]):
        aChar8Vec (char8[]):
        aChar16Vec (char16[]):
        aInt8Vec (int8[]):
        aInt16Vec (int16[]):
        aInt32Vec (int32[]):
        aInt64Vec (int64[]):
        aPtrVec (ptr64[]):
        aFloatVec (float[]):
        aDoubleVec (double[]):
        aStringVec (string[]):
        aAnyVec (any[]):
        aVec2Vec (vec2[]):
        aVec3Vec (vec3[]):
        aVec4Vec (vec4[]):

    Returns:
        int32:
    """
    ...

def ParamAllRefAliasesCallback(aBool: AliasBool, aChar8: AliasChar8, aChar16: AliasChar16, aInt8: AliasInt8, aInt16: AliasInt16, aInt32: AliasInt32, aInt64: AliasInt64, aPtr: AliasPtr, aFloat: AliasFloat, aDouble: AliasDouble, aString: AliasString, aAny: AliasAny, aVec2: AliasVec2, aVec3: AliasVec3, aVec4: AliasVec4, aMat4x4: AliasMat4x4, aBoolVec: AliasBoolVector, aChar8Vec: AliasChar8Vector, aChar16Vec: AliasChar16Vector, aInt8Vec: AliasInt8Vector, aInt16Vec: AliasInt16Vector, aInt32Vec: AliasInt32Vector, aInt64Vec: AliasInt64Vector, aPtrVec: AliasPtrVector, aFloatVec: AliasFloatVector, aDoubleVec: AliasDoubleVector, aStringVec: AliasStringVector, aAnyVec: AliasAnyVector, aVec2Vec: AliasVec2Vector, aVec3Vec: AliasVec3Vector, aVec4Vec: AliasVec4Vector) -> tuple[int, AliasBool, AliasChar8, AliasChar16, AliasInt8, AliasInt16, AliasInt32, AliasInt64, AliasPtr, AliasFloat, AliasDouble, AliasString, AliasAny, AliasVec2, AliasVec3, AliasVec4, AliasMat4x4, AliasBoolVector, AliasChar8Vector, AliasChar16Vector, AliasInt8Vector, AliasInt16Vector, AliasInt32Vector, AliasInt64Vector, AliasPtrVector, AliasFloatVector, AliasDoubleVector, AliasStringVector, AliasAnyVector, AliasVec2Vector, AliasVec3Vector, AliasVec4Vector]:
    """

    Args:
        aBool (bool):
        aChar8 (char8):
        aChar16 (char16):
        aInt8 (int8):
        aInt16 (int16):
        aInt32 (int32):
        aInt64 (int64):
        aPtr (ptr64):
        aFloat (float):
        aDouble (double):
        aString (string):
        aAny (any):
        aVec2 (vec2):
        aVec3 (vec3):
        aVec4 (vec4):
        aMat4x4 (mat4x4):
        aBoolVec (bool[]):
        aChar8Vec (char8[]):
        aChar16Vec (char16[]):
        aInt8Vec (int8[]):
        aInt16Vec (int16[]):
        aInt32Vec (int32[]):
        aInt64Vec (int64[]):
        aPtrVec (ptr64[]):
        aFloatVec (float[]):
        aDoubleVec (double[]):
        aStringVec (string[]):
        aAnyVec (any[]):
        aVec2Vec (vec2[]):
        aVec3Vec (vec3[]):
        aVec4Vec (vec4[]):

    Returns:
        int64:
    """
    ...

def ParamEnumCallback(p1: Example, p2: list[Example]) -> int:
    """

    Args:
        p1 (int32):
        p2 (int32[]):

    Returns:
        int32:
    """
    ...

def ParamEnumRefCallback(p1: Example, p2: list[Example]) -> tuple[int, Example, list[Example]]:
    """

    Args:
        p1 (int32):
        p2 (int32[]):

    Returns:
        int32:
    """
    ...

def ParamVariantCallback(p1: object, p2: list[object]) -> None:
    """

    Args:
        p1 (any):
        p2 (any[]):
    """
    ...

def ParamVariantRefCallback(p1: object, p2: list[object]) -> tuple[None, object, list[object]]:
    """

    Args:
        p1 (any):
        p2 (any[]):
    """
    ...

def CallFuncVoidCallback(func: Callable[[], None]) -> None:
    """

    Args:
        func (function):

    Callback Prototype (FuncVoid):
    """
    ...

def CallFuncBoolCallback(func: Callable[[], bool]) -> bool:
    """

    Args:
        func (function):

    Returns:
        bool:

    Callback Prototype (FuncBool):

        Returns:
            bool:
    """
    ...

def CallFuncChar8Callback(func: Callable[[], str]) -> str:
    """

    Args:
        func (function):

    Returns:
        char8:

    Callback Prototype (FuncChar8):

        Returns:
            char8:
    """
    ...

def CallFuncChar16Callback(func: Callable[[], str]) -> str:
    """

    Args:
        func (function):

    Returns:
        char16:

    Callback Prototype (FuncChar16):

        Returns:
            char16:
    """
    ...

def CallFuncInt8Callback(func: Callable[[], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        int8:

    Callback Prototype (FuncInt8):

        Returns:
            int8:
    """
    ...

def CallFuncInt16Callback(func: Callable[[], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        int16:

    Callback Prototype (FuncInt16):

        Returns:
            int16:
    """
    ...

def CallFuncInt32Callback(func: Callable[[], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        int32:

    Callback Prototype (FuncInt32):

        Returns:
            int32:
    """
    ...

def CallFuncInt64Callback(func: Callable[[], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        int64:

    Callback Prototype (FuncInt64):

        Returns:
            int64:
    """
    ...

def CallFuncUInt8Callback(func: Callable[[], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        uint8:

    Callback Prototype (FuncUInt8):

        Returns:
            uint8:
    """
    ...

def CallFuncUInt16Callback(func: Callable[[], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        uint16:

    Callback Prototype (FuncUInt16):

        Returns:
            uint16:
    """
    ...

def CallFuncUInt32Callback(func: Callable[[], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        uint32:

    Callback Prototype (FuncUInt32):

        Returns:
            uint32:
    """
    ...

def CallFuncUInt64Callback(func: Callable[[], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        uint64:

    Callback Prototype (FuncUInt64):

        Returns:
            uint64:
    """
    ...

def CallFuncPtrCallback(func: Callable[[], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        ptr64:

    Callback Prototype (FuncPtr):

        Returns:
            ptr64:
    """
    ...

def CallFuncFloatCallback(func: Callable[[], float]) -> float:
    """

    Args:
        func (function):

    Returns:
        float:

    Callback Prototype (FuncFloat):

        Returns:
            float:
    """
    ...

def CallFuncDoubleCallback(func: Callable[[], float]) -> float:
    """

    Args:
        func (function):

    Returns:
        double:

    Callback Prototype (FuncDouble):

        Returns:
            double:
    """
    ...

def CallFuncStringCallback(func: Callable[[], str]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (FuncString):

        Returns:
            string:
    """
    ...

def CallFuncAnyCallback(func: Callable[[], object]) -> object:
    """

    Args:
        func (function):

    Returns:
        any:

    Callback Prototype (FuncAny):

        Returns:
            any:
    """
    ...

def CallFuncFunctionCallback(func: Callable[[], Callable[[], None]]) -> int:
    """

    Args:
        func (function):

    Returns:
        ptr64:

    Callback Prototype (FuncFunction):

        Returns:
            function:
    """
    ...

def CallFuncBoolVectorCallback(func: Callable[[], list[bool]]) -> list[bool]:
    """

    Args:
        func (function):

    Returns:
        bool[]:

    Callback Prototype (FuncBoolVector):

        Returns:
            bool[]:
    """
    ...

def CallFuncChar8VectorCallback(func: Callable[[], list[str]]) -> list[str]:
    """

    Args:
        func (function):

    Returns:
        char8[]:

    Callback Prototype (FuncChar8Vector):

        Returns:
            char8[]:
    """
    ...

def CallFuncChar16VectorCallback(func: Callable[[], list[str]]) -> list[str]:
    """

    Args:
        func (function):

    Returns:
        char16[]:

    Callback Prototype (FuncChar16Vector):

        Returns:
            char16[]:
    """
    ...

def CallFuncInt8VectorCallback(func: Callable[[], list[int]]) -> list[int]:
    """

    Args:
        func (function):

    Returns:
        int8[]:

    Callback Prototype (FuncInt8Vector):

        Returns:
            int8[]:
    """
    ...

def CallFuncInt16VectorCallback(func: Callable[[], list[int]]) -> list[int]:
    """

    Args:
        func (function):

    Returns:
        int16[]:

    Callback Prototype (FuncInt16Vector):

        Returns:
            int16[]:
    """
    ...

def CallFuncInt32VectorCallback(func: Callable[[], list[int]]) -> list[int]:
    """

    Args:
        func (function):

    Returns:
        int32[]:

    Callback Prototype (FuncInt32Vector):

        Returns:
            int32[]:
    """
    ...

def CallFuncInt64VectorCallback(func: Callable[[], list[int]]) -> list[int]:
    """

    Args:
        func (function):

    Returns:
        int64[]:

    Callback Prototype (FuncInt64Vector):

        Returns:
            int64[]:
    """
    ...

def CallFuncUInt8VectorCallback(func: Callable[[], list[int]]) -> list[int]:
    """

    Args:
        func (function):

    Returns:
        uint8[]:

    Callback Prototype (FuncUInt8Vector):

        Returns:
            uint8[]:
    """
    ...

def CallFuncUInt16VectorCallback(func: Callable[[], list[int]]) -> list[int]:
    """

    Args:
        func (function):

    Returns:
        uint16[]:

    Callback Prototype (FuncUInt16Vector):

        Returns:
            uint16[]:
    """
    ...

def CallFuncUInt32VectorCallback(func: Callable[[], list[int]]) -> list[int]:
    """

    Args:
        func (function):

    Returns:
        uint32[]:

    Callback Prototype (FuncUInt32Vector):

        Returns:
            uint32[]:
    """
    ...

def CallFuncUInt64VectorCallback(func: Callable[[], list[int]]) -> list[int]:
    """

    Args:
        func (function):

    Returns:
        uint64[]:

    Callback Prototype (FuncUInt64Vector):

        Returns:
            uint64[]:
    """
    ...

def CallFuncPtrVectorCallback(func: Callable[[], list[int]]) -> list[int]:
    """

    Args:
        func (function):

    Returns:
        ptr64[]:

    Callback Prototype (FuncPtrVector):

        Returns:
            ptr64[]:
    """
    ...

def CallFuncFloatVectorCallback(func: Callable[[], list[float]]) -> list[float]:
    """

    Args:
        func (function):

    Returns:
        float[]:

    Callback Prototype (FuncFloatVector):

        Returns:
            float[]:
    """
    ...

def CallFuncDoubleVectorCallback(func: Callable[[], list[float]]) -> list[float]:
    """

    Args:
        func (function):

    Returns:
        double[]:

    Callback Prototype (FuncDoubleVector):

        Returns:
            double[]:
    """
    ...

def CallFuncStringVectorCallback(func: Callable[[], list[str]]) -> list[str]:
    """

    Args:
        func (function):

    Returns:
        string[]:

    Callback Prototype (FuncStringVector):

        Returns:
            string[]:
    """
    ...

def CallFuncAnyVectorCallback(func: Callable[[], list[object]]) -> list[object]:
    """

    Args:
        func (function):

    Returns:
        any[]:

    Callback Prototype (FuncAnyVector):

        Returns:
            any[]:
    """
    ...

def CallFuncVec2VectorCallback(func: Callable[[], list[Vector2]]) -> list[Vector2]:
    """

    Args:
        func (function):

    Returns:
        vec2[]:

    Callback Prototype (FuncVec2Vector):

        Returns:
            vec2[]:
    """
    ...

def CallFuncVec3VectorCallback(func: Callable[[], list[Vector3]]) -> list[Vector3]:
    """

    Args:
        func (function):

    Returns:
        vec3[]:

    Callback Prototype (FuncVec3Vector):

        Returns:
            vec3[]:
    """
    ...

def CallFuncVec4VectorCallback(func: Callable[[], list[Vector4]]) -> list[Vector4]:
    """

    Args:
        func (function):

    Returns:
        vec4[]:

    Callback Prototype (FuncVec4Vector):

        Returns:
            vec4[]:
    """
    ...

def CallFuncMat4x4VectorCallback(func: Callable[[], list[Matrix4x4]]) -> list[Matrix4x4]:
    """

    Args:
        func (function):

    Returns:
        mat4x4[]:

    Callback Prototype (FuncMat4x4Vector):

        Returns:
            mat4x4[]:
    """
    ...

def CallFuncVec2Callback(func: Callable[[], Vector2]) -> Vector2:
    """

    Args:
        func (function):

    Returns:
        vec2:

    Callback Prototype (FuncVec2):

        Returns:
            vec2:
    """
    ...

def CallFuncVec3Callback(func: Callable[[], Vector3]) -> Vector3:
    """

    Args:
        func (function):

    Returns:
        vec3:

    Callback Prototype (FuncVec3):

        Returns:
            vec3:
    """
    ...

def CallFuncVec4Callback(func: Callable[[], Vector4]) -> Vector4:
    """

    Args:
        func (function):

    Returns:
        vec4:

    Callback Prototype (FuncVec4):

        Returns:
            vec4:
    """
    ...

def CallFuncMat4x4Callback(func: Callable[[], Matrix4x4]) -> Matrix4x4:
    """

    Args:
        func (function):

    Returns:
        mat4x4:

    Callback Prototype (FuncMat4x4):

        Returns:
            mat4x4:
    """
    ...

def CallFuncAliasBoolCallback(func: Callable[[], AliasBool]) -> AliasBool:
    """

    Args:
        func (function):

    Returns:
        bool:

    Callback Prototype (FuncAliasBool):

        Returns:
            bool:
    """
    ...

def CallFuncAliasChar8Callback(func: Callable[[], AliasChar8]) -> AliasChar8:
    """

    Args:
        func (function):

    Returns:
        char8:

    Callback Prototype (FuncAliasChar8):

        Returns:
            char8:
    """
    ...

def CallFuncAliasChar16Callback(func: Callable[[], AliasChar16]) -> AliasChar16:
    """

    Args:
        func (function):

    Returns:
        char16:

    Callback Prototype (FuncAliasChar16):

        Returns:
            char16:
    """
    ...

def CallFuncAliasInt8Callback(func: Callable[[], AliasInt8]) -> AliasInt8:
    """

    Args:
        func (function):

    Returns:
        int8:

    Callback Prototype (FuncAliasInt8):

        Returns:
            int8:
    """
    ...

def CallFuncAliasInt16Callback(func: Callable[[], AliasInt16]) -> AliasInt16:
    """

    Args:
        func (function):

    Returns:
        int16:

    Callback Prototype (FuncAliasInt16):

        Returns:
            int16:
    """
    ...

def CallFuncAliasInt32Callback(func: Callable[[], AliasInt32]) -> AliasInt32:
    """

    Args:
        func (function):

    Returns:
        int32:

    Callback Prototype (FuncAliasInt32):

        Returns:
            int32:
    """
    ...

def CallFuncAliasInt64Callback(func: Callable[[], AliasInt64]) -> AliasInt64:
    """

    Args:
        func (function):

    Returns:
        int64:

    Callback Prototype (FuncAliasInt64):

        Returns:
            int64:
    """
    ...

def CallFuncAliasUInt8Callback(func: Callable[[], AliasUInt8]) -> AliasUInt8:
    """

    Args:
        func (function):

    Returns:
        uint8:

    Callback Prototype (FuncAliasUInt8):

        Returns:
            uint8:
    """
    ...

def CallFuncAliasUInt16Callback(func: Callable[[], AliasUInt16]) -> AliasUInt16:
    """

    Args:
        func (function):

    Returns:
        uint16:

    Callback Prototype (FuncAliasUInt16):

        Returns:
            uint16:
    """
    ...

def CallFuncAliasUInt32Callback(func: Callable[[], AliasUInt32]) -> AliasUInt32:
    """

    Args:
        func (function):

    Returns:
        uint32:

    Callback Prototype (FuncAliasUInt32):

        Returns:
            uint32:
    """
    ...

def CallFuncAliasUInt64Callback(func: Callable[[], AliasUInt64]) -> AliasUInt64:
    """

    Args:
        func (function):

    Returns:
        uint64:

    Callback Prototype (FuncAliasUInt64):

        Returns:
            uint64:
    """
    ...

def CallFuncAliasPtrCallback(func: Callable[[], AliasPtr]) -> AliasPtr:
    """

    Args:
        func (function):

    Returns:
        ptr64:

    Callback Prototype (FuncAliasPtr):

        Returns:
            ptr64:
    """
    ...

def CallFuncAliasFloatCallback(func: Callable[[], AliasFloat]) -> AliasFloat:
    """

    Args:
        func (function):

    Returns:
        float:

    Callback Prototype (FuncAliasFloat):

        Returns:
            float:
    """
    ...

def CallFuncAliasDoubleCallback(func: Callable[[], AliasDouble]) -> AliasDouble:
    """

    Args:
        func (function):

    Returns:
        double:

    Callback Prototype (FuncAliasDouble):

        Returns:
            double:
    """
    ...

def CallFuncAliasStringCallback(func: Callable[[], AliasString]) -> AliasString:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (FuncAliasString):

        Returns:
            string:
    """
    ...

def CallFuncAliasAnyCallback(func: Callable[[], AliasAny]) -> AliasAny:
    """

    Args:
        func (function):

    Returns:
        any:

    Callback Prototype (FuncAliasAny):

        Returns:
            any:
    """
    ...

def CallFuncAliasFunctionCallback(func: Callable[[], AliasFunction]) -> AliasFunction:
    """

    Args:
        func (function):

    Returns:
        ptr64:

    Callback Prototype (FuncAliasFunction):

        Returns:
            function:
    """
    ...

def CallFuncAliasBoolVectorCallback(func: Callable[[], AliasBoolVector]) -> AliasBoolVector:
    """

    Args:
        func (function):

    Returns:
        bool[]:

    Callback Prototype (FuncAliasBoolVector):

        Returns:
            bool[]:
    """
    ...

def CallFuncAliasChar8VectorCallback(func: Callable[[], AliasChar8Vector]) -> AliasChar8Vector:
    """

    Args:
        func (function):

    Returns:
        char8[]:

    Callback Prototype (FuncAliasChar8Vector):

        Returns:
            char8[]:
    """
    ...

def CallFuncAliasChar16VectorCallback(func: Callable[[], AliasChar16Vector]) -> AliasChar16Vector:
    """

    Args:
        func (function):

    Returns:
        char16[]:

    Callback Prototype (FuncAliasChar16Vector):

        Returns:
            char16[]:
    """
    ...

def CallFuncAliasInt8VectorCallback(func: Callable[[], AliasInt8Vector]) -> AliasInt8Vector:
    """

    Args:
        func (function):

    Returns:
        int8[]:

    Callback Prototype (FuncAliasInt8Vector):

        Returns:
            int8[]:
    """
    ...

def CallFuncAliasInt16VectorCallback(func: Callable[[], AliasInt16Vector]) -> AliasInt16Vector:
    """

    Args:
        func (function):

    Returns:
        int16[]:

    Callback Prototype (FuncAliasInt16Vector):

        Returns:
            int16[]:
    """
    ...

def CallFuncAliasInt32VectorCallback(func: Callable[[], AliasInt32Vector]) -> AliasInt32Vector:
    """

    Args:
        func (function):

    Returns:
        int32[]:

    Callback Prototype (FuncAliasInt32Vector):

        Returns:
            int32[]:
    """
    ...

def CallFuncAliasInt64VectorCallback(func: Callable[[], AliasInt64Vector]) -> AliasInt64Vector:
    """

    Args:
        func (function):

    Returns:
        int64[]:

    Callback Prototype (FuncAliasInt64Vector):

        Returns:
            int64[]:
    """
    ...

def CallFuncAliasUInt8VectorCallback(func: Callable[[], AliasUInt8Vector]) -> AliasUInt8Vector:
    """

    Args:
        func (function):

    Returns:
        uint8[]:

    Callback Prototype (FuncAliasUInt8Vector):

        Returns:
            uint8[]:
    """
    ...

def CallFuncAliasUInt16VectorCallback(func: Callable[[], AliasUInt16Vector]) -> AliasUInt16Vector:
    """

    Args:
        func (function):

    Returns:
        uint16[]:

    Callback Prototype (FuncAliasUInt16Vector):

        Returns:
            uint16[]:
    """
    ...

def CallFuncAliasUInt32VectorCallback(func: Callable[[], AliasUInt32Vector]) -> AliasUInt32Vector:
    """

    Args:
        func (function):

    Returns:
        uint32[]:

    Callback Prototype (FuncAliasUInt32Vector):

        Returns:
            uint32[]:
    """
    ...

def CallFuncAliasUInt64VectorCallback(func: Callable[[], AliasUInt64Vector]) -> AliasUInt64Vector:
    """

    Args:
        func (function):

    Returns:
        uint64[]:

    Callback Prototype (FuncAliasUInt64Vector):

        Returns:
            uint64[]:
    """
    ...

def CallFuncAliasPtrVectorCallback(func: Callable[[], AliasPtrVector]) -> AliasPtrVector:
    """

    Args:
        func (function):

    Returns:
        ptr64[]:

    Callback Prototype (FuncAliasPtrVector):

        Returns:
            ptr64[]:
    """
    ...

def CallFuncAliasFloatVectorCallback(func: Callable[[], AliasFloatVector]) -> AliasFloatVector:
    """

    Args:
        func (function):

    Returns:
        float[]:

    Callback Prototype (FuncAliasFloatVector):

        Returns:
            float[]:
    """
    ...

def CallFuncAliasDoubleVectorCallback(func: Callable[[], AliasDoubleVector]) -> AliasDoubleVector:
    """

    Args:
        func (function):

    Returns:
        double[]:

    Callback Prototype (FuncAliasDoubleVector):

        Returns:
            double[]:
    """
    ...

def CallFuncAliasStringVectorCallback(func: Callable[[], AliasStringVector]) -> AliasStringVector:
    """

    Args:
        func (function):

    Returns:
        string[]:

    Callback Prototype (FuncAliasStringVector):

        Returns:
            string[]:
    """
    ...

def CallFuncAliasAnyVectorCallback(func: Callable[[], AliasAnyVector]) -> AliasAnyVector:
    """

    Args:
        func (function):

    Returns:
        any[]:

    Callback Prototype (FuncAliasAnyVector):

        Returns:
            any[]:
    """
    ...

def CallFuncAliasVec2VectorCallback(func: Callable[[], AliasVec2Vector]) -> AliasVec2Vector:
    """

    Args:
        func (function):

    Returns:
        vec2[]:

    Callback Prototype (FuncAliasVec2Vector):

        Returns:
            vec2[]:
    """
    ...

def CallFuncAliasVec3VectorCallback(func: Callable[[], AliasVec3Vector]) -> AliasVec3Vector:
    """

    Args:
        func (function):

    Returns:
        vec3[]:

    Callback Prototype (FuncAliasVec3Vector):

        Returns:
            vec3[]:
    """
    ...

def CallFuncAliasVec4VectorCallback(func: Callable[[], AliasVec4Vector]) -> AliasVec4Vector:
    """

    Args:
        func (function):

    Returns:
        vec4[]:

    Callback Prototype (FuncAliasVec4Vector):

        Returns:
            vec4[]:
    """
    ...

def CallFuncAliasMat4x4VectorCallback(func: Callable[[], AliasMat4x4Vector]) -> AliasMat4x4Vector:
    """

    Args:
        func (function):

    Returns:
        mat4x4[]:

    Callback Prototype (FuncAliasMat4x4Vector):

        Returns:
            mat4x4[]:
    """
    ...

def CallFuncAliasVec2Callback(func: Callable[[], AliasVec2]) -> AliasVec2:
    """

    Args:
        func (function):

    Returns:
        vec2:

    Callback Prototype (FuncAliasVec2):

        Returns:
            vec2:
    """
    ...

def CallFuncAliasVec3Callback(func: Callable[[], AliasVec3]) -> AliasVec3:
    """

    Args:
        func (function):

    Returns:
        vec3:

    Callback Prototype (FuncAliasVec3):

        Returns:
            vec3:
    """
    ...

def CallFuncAliasVec4Callback(func: Callable[[], AliasVec4]) -> AliasVec4:
    """

    Args:
        func (function):

    Returns:
        vec4:

    Callback Prototype (FuncAliasVec4):

        Returns:
            vec4:
    """
    ...

def CallFuncAliasMat4x4Callback(func: Callable[[], AliasMat4x4]) -> AliasMat4x4:
    """

    Args:
        func (function):

    Returns:
        mat4x4:

    Callback Prototype (FuncAliasMat4x4):

        Returns:
            mat4x4:
    """
    ...

def CallFuncAliasAllCallback(func: Callable[[AliasBool, AliasChar8, AliasChar16, AliasInt8, AliasInt16, AliasInt32, AliasInt64, AliasPtr, AliasFloat, AliasDouble, AliasString, AliasAny, AliasVec2, AliasVec3, AliasVec4, AliasMat4x4, AliasBoolVector, AliasChar8Vector, AliasChar16Vector, AliasInt8Vector, AliasInt16Vector, AliasInt32Vector, AliasInt64Vector, AliasPtrVector, AliasFloatVector, AliasDoubleVector, AliasStringVector, AliasAnyVector, AliasVec2Vector, AliasVec3Vector, AliasVec4Vector], str]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (FuncAliasAll):
        Args:
            aBool (bool):
            aChar8 (char8):
            aChar16 (char16):
            aInt8 (int8):
            aInt16 (int16):
            aInt32 (int32):
            aInt64 (int64):
            aPtr (ptr64):
            aFloat (float):
            aDouble (double):
            aString (string):
            aAny (any):
            aVec2 (vec2):
            aVec3 (vec3):
            aVec4 (vec4):
            aMat4x4 (mat4x4):
            aBoolVec (bool[]):
            aChar8Vec (char8[]):
            aChar16Vec (char16[]):
            aInt8Vec (int8[]):
            aInt16Vec (int16[]):
            aInt32Vec (int32[]):
            aInt64Vec (int64[]):
            aPtrVec (ptr64[]):
            aFloatVec (float[]):
            aDoubleVec (double[]):
            aStringVec (string[]):
            aAnyVec (any[]):
            aVec2Vec (vec2[]):
            aVec3Vec (vec3[]):
            aVec4Vec (vec4[]):

        Returns:
            string:
    """
    ...

def CallFunc1Callback(func: Callable[[Vector3], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        int32:

    Callback Prototype (Func1):
        Args:
            a (vec3):

        Returns:
            int32:
    """
    ...

def CallFunc2Callback(func: Callable[[float, int], str]) -> str:
    """

    Args:
        func (function):

    Returns:
        char8:

    Callback Prototype (Func2):
        Args:
            a (float):
            b (int64):

        Returns:
            char8:
    """
    ...

def CallFunc3Callback(func: Callable[[int, Vector4, str], None]) -> None:
    """

    Args:
        func (function):

    Callback Prototype (Func3):
        Args:
            a (ptr64):
            b (vec4):
            c (string):
    """
    ...

def CallFunc4Callback(func: Callable[[bool, int, str, Matrix4x4], Vector4]) -> Vector4:
    """

    Args:
        func (function):

    Returns:
        vec4:

    Callback Prototype (Func4):
        Args:
            a (bool):
            b (int32):
            c (char16):
            d (mat4x4):

        Returns:
            vec4:
    """
    ...

def CallFunc5Callback(func: Callable[[int, Vector2, int, float, list[int]], bool]) -> bool:
    """

    Args:
        func (function):

    Returns:
        bool:

    Callback Prototype (Func5):
        Args:
            a (int8):
            b (vec2):
            c (ptr64):
            d (double):
            e (uint64[]):

        Returns:
            bool:
    """
    ...

def CallFunc6Callback(func: Callable[[str, float, list[float], int, list[int], int], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        int64:

    Callback Prototype (Func6):
        Args:
            a (string):
            b (float):
            c (float[]):
            d (int16):
            e (uint8[]):
            f (ptr64):

        Returns:
            int64:
    """
    ...

def CallFunc7Callback(func: Callable[[list[str], int, str, list[int], Vector4, bool, int], float]) -> float:
    """

    Args:
        func (function):

    Returns:
        double:

    Callback Prototype (Func7):
        Args:
            vecC (char8[]):
            u16 (uint16):
            ch16 (char16):
            vecU32 (uint32[]):
            vec4 (vec4):
            b (bool):
            u64 (uint64):

        Returns:
            double:
    """
    ...

def CallFunc8Callback(func: Callable[[Vector3, list[int], int, bool, Vector4, list[str], str, int], Matrix4x4]) -> Matrix4x4:
    """

    Args:
        func (function):

    Returns:
        mat4x4:

    Callback Prototype (Func8):
        Args:
            vec3 (vec3):
            vecU32 (uint32[]):
            i16 (int16):
            b (bool):
            vec4 (vec4):
            vecC16 (char16[]):
            ch16 (char16):
            i32 (int32):

        Returns:
            mat4x4:
    """
    ...

def CallFunc9Callback(func: Callable[[float, Vector2, list[int], int, bool, str, Vector4, int, int], None]) -> None:
    """

    Args:
        func (function):

    Callback Prototype (Func9):
        Args:
            f (float):
            vec2 (vec2):
            vecI8 (int8[]):
            u64 (uint64):
            b (bool):
            str (string):
            vec4 (vec4):
            i16 (int16):
            ptr (ptr64):
    """
    ...

def CallFunc10Callback(func: Callable[[Vector4, Matrix4x4, list[int], int, list[str], int, bool, Vector2, int, float], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        uint32:

    Callback Prototype (Func10):
        Args:
            vec4 (vec4):
            mat (mat4x4):
            vecU32 (uint32[]):
            u64 (uint64):
            vecC (char8[]):
            i32 (int32):
            b (bool):
            vec2 (vec2):
            i64 (int64):
            d (double):

        Returns:
            uint32:
    """
    ...

def CallFunc11Callback(func: Callable[[list[bool], str, int, float, Vector3, list[int], int, int, float, Vector2, int], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        ptr64:

    Callback Prototype (Func11):
        Args:
            vecB (bool[]):
            ch16 (char16):
            u8 (uint8):
            d (double):
            vec3 (vec3):
            vecI8 (int8[]):
            i64 (int64):
            u16 (uint16):
            f (float):
            vec2 (vec2):
            u32 (uint32):

        Returns:
            ptr64:
    """
    ...

def CallFunc12Callback(func: Callable[[int, list[float], int, float, bool, int, int, int, float, list[int], int, str], bool]) -> bool:
    """

    Args:
        func (function):

    Returns:
        bool:

    Callback Prototype (Func12):
        Args:
            ptr (ptr64):
            vecD (double[]):
            u32 (uint32):
            d (double):
            b (bool):
            i32 (int32):
            i8 (int8):
            u64 (uint64):
            f (float):
            vecPtr (ptr64[]):
            i64 (int64):
            ch (char8):

        Returns:
            bool:
    """
    ...

def CallFunc13Callback(func: Callable[[int, list[str], int, float, list[bool], Vector4, str, int, Vector3, int, Vector2, list[int], int], str]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func13):
        Args:
            i64 (int64):
            vecC (char8[]):
            d (uint16):
            f (float):
            b (bool[]):
            vec4 (vec4):
            str (string):
            int32 (int32):
            vec3 (vec3):
            ptr (ptr64):
            vec2 (vec2):
            arr (uint8[]):
            i16 (int16):

        Returns:
            string:
    """
    ...

def CallFunc14Callback(func: Callable[[list[str], list[int], Matrix4x4, bool, str, int, list[float], int, list[int], int, Vector3, Vector4, float, int], list[str]]) -> list[str]:
    """

    Args:
        func (function):

    Returns:
        string[]:

    Callback Prototype (Func14):
        Args:
            vecC (char8[]):
            vecU32 (uint32[]):
            mat (mat4x4):
            b (bool):
            ch16 (char16):
            i32 (int32):
            vecF (float[]):
            u16 (uint16):
            vecU8 (uint8[]):
            i8 (int8):
            vec3 (vec3):
            vec4 (vec4):
            d (double):
            ptr (ptr64):

        Returns:
            string[]:
    """
    ...

def CallFunc15Callback(func: Callable[[list[int], Matrix4x4, Vector4, int, int, list[int], bool, float, list[str], int, int, Vector2, int, float, list[int]], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        int16:

    Callback Prototype (Func15):
        Args:
            vecI16 (int16[]):
            mat (mat4x4):
            vec4 (vec4):
            ptr (ptr64):
            u64 (uint64):
            vecU32 (uint32[]):
            b (bool):
            f (float):
            vecC16 (char16[]):
            u8 (uint8):
            i32 (int32):
            vec2 (vec2):
            u16 (uint16):
            d (double):
            vecU8 (uint8[]):

        Returns:
            int16:
    """
    ...

def CallFunc16Callback(func: Callable[[list[bool], int, list[int], Vector4, Matrix4x4, Vector2, list[int], list[str], str, int, list[int], Vector3, float, float, int, int], int]) -> int:
    """

    Args:
        func (function):

    Returns:
        ptr64:

    Callback Prototype (Func16):
        Args:
            vecB (bool[]):
            i16 (int16):
            vecI8 (int8[]):
            vec4 (vec4):
            mat (mat4x4):
            vec2 (vec2):
            vecU64 (uint64[]):
            vecC (char8[]):
            str (string):
            i64 (int64):
            vecU32 (uint32[]):
            vec3 (vec3):
            f (float):
            d (double):
            i8 (int8):
            u16 (uint16):

        Returns:
            ptr64:
    """
    ...

def CallFunc17Callback(func: Callable[[int], None]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func17):
        Args:
            i32 (int32):
    """
    ...

def CallFunc18Callback(func: Callable[[int, int], Vector2]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func18):
        Args:
            i8 (int8):
            i16 (int16):

        Returns:
            vec2:
    """
    ...

def CallFunc19Callback(func: Callable[[int, Vector3, list[int]], None]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func19):
        Args:
            u32 (uint32):
            vec3 (vec3):
            vecU32 (uint32[]):
    """
    ...

def CallFunc20Callback(func: Callable[[str, Vector4, list[int], str], int]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func20):
        Args:
            ch16 (char16):
            vec4 (vec4):
            vecU64 (uint64[]):
            ch (char8):

        Returns:
            int32:
    """
    ...

def CallFunc21Callback(func: Callable[[Matrix4x4, list[int], Vector2, bool, float], float]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func21):
        Args:
            mat (mat4x4):
            vecI32 (int32[]):
            vec2 (vec2):
            b (bool):
            extraParam (double):

        Returns:
            float:
    """
    ...

def CallFunc22Callback(func: Callable[[int, int, list[float], int, str, Vector4], int]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func22):
        Args:
            ptr64Ref (ptr64):
            uint32Ref (uint32):
            vectorDoubleRef (double[]):
            int16Ref (int16):
            plgStringRef (string):
            plgVector4Ref (vec4):

        Returns:
            uint64:
    """
    ...

def CallFunc23Callback(func: Callable[[int, Vector2, list[int], str, float, int, list[int]], None]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func23):
        Args:
            uint64Ref (uint64):
            plgVector2Ref (vec2):
            vectorInt16Ref (int16[]):
            char16Ref (char16):
            floatRef (float):
            int8Ref (int8):
            vectorUInt8Ref (uint8[]):
    """
    ...

def CallFunc24Callback(func: Callable[[list[str], int, list[int], Vector4, int, list[int], float, list[int]], Matrix4x4]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func24):
        Args:
            vectorCharRef (char8[]):
            int64Ref (int64):
            vectorUInt8Ref (uint8[]):
            plgVector4Ref (vec4):
            uint64Ref (uint64):
            vectorptr64Ref (ptr64[]):
            doubleRef (double):
            vectorptr64Ref2 (ptr64[]):

        Returns:
            mat4x4:
    """
    ...

def CallFunc25Callback(func: Callable[[int, list[int], bool, int, str, Vector3, int, Vector4, int], float]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func25):
        Args:
            int32Ref (int32):
            vectorptr64Ref (ptr64[]):
            boolRef (bool):
            uint8Ref (uint8):
            plgStringRef (string):
            plgVector3Ref (vec3):
            int64Ref (int64):
            plgVector4Ref (vec4):
            uint16Ref (uint16):

        Returns:
            double:
    """
    ...

def CallFunc26Callback(func: Callable[[str, Vector2, Matrix4x4, list[float], int, int, int, list[int], int, bool], str]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func26):
        Args:
            char16Ref (char16):
            plgVector2Ref (vec2):
            plgMatrix4x4Ref (mat4x4):
            vectorFloatRef (float[]):
            int16Ref (int16):
            uint64Ref (uint64):
            uint32Ref (uint32):
            vectorUInt16Ref (uint16[]):
            ptr64Ref (ptr64):
            boolRef (bool):

        Returns:
            char8:
    """
    ...

def CallFunc27Callback(func: Callable[[float, Vector3, int, Vector2, list[int], Matrix4x4, bool, Vector4, int, int, list[int]], int]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func27):
        Args:
            floatRef (float):
            plgVector3Ref (vec3):
            ptr64Ref (ptr64):
            plgVector2Ref (vec2):
            vectorInt16Ref (int16[]):
            plgMatrix4x4Ref (mat4x4):
            boolRef (bool):
            plgVector4Ref (vec4):
            int8Ref (int8):
            int32Ref (int32):
            vectorUInt8Ref (uint8[]):

        Returns:
            uint8:
    """
    ...

def CallFunc28Callback(func: Callable[[int, int, list[int], Matrix4x4, float, Vector4, str, list[int], int, bool, Vector3, list[float]], str]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func28):
        Args:
            ptr64Ref (ptr64):
            uint16Ref (uint16):
            vectorUInt32Ref (uint32[]):
            plgMatrix4x4Ref (mat4x4):
            floatRef (float):
            plgVector4Ref (vec4):
            plgStringRef (string):
            vectorUInt64Ref (uint64[]):
            int64Ref (int64):
            boolRef (bool):
            plgVector3Ref (vec3):
            vectorFloatRef (float[]):

        Returns:
            string:
    """
    ...

def CallFunc29Callback(func: Callable[[Vector4, int, list[int], float, bool, int, list[int], float, str, Matrix4x4, int, Vector3, list[int]], list[str]]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func29):
        Args:
            plgVector4Ref (vec4):
            int32Ref (int32):
            vectorInt8Ref (int8[]):
            doubleRef (double):
            boolRef (bool):
            int8Ref (int8):
            vectorUInt16Ref (uint16[]):
            floatRef (float):
            plgStringRef (string):
            plgMatrix4x4Ref (mat4x4):
            uint64Ref (uint64):
            plgVector3Ref (vec3):
            vectorInt64Ref (int64[]):

        Returns:
            string[]:
    """
    ...

def CallFunc30Callback(func: Callable[[int, Vector4, int, list[int], bool, str, Vector3, list[int], float, Vector2, Matrix4x4, int, list[float], float], int]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func30):
        Args:
            ptr64Ref (ptr64):
            plgVector4Ref (vec4):
            int64Ref (int64):
            vectorUInt32Ref (uint32[]):
            boolRef (bool):
            plgStringRef (string):
            plgVector3Ref (vec3):
            vectorUInt8Ref (uint8[]):
            floatRef (float):
            plgVector2Ref (vec2):
            plgMatrix4x4Ref (mat4x4):
            int8Ref (int8):
            vectorFloatRef (float[]):
            doubleRef (double):

        Returns:
            int32:
    """
    ...

def CallFunc31Callback(func: Callable[[str, int, list[int], Vector4, str, bool, int, Vector2, int, int, list[int], Matrix4x4, Vector3, float, list[float]], Vector3]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func31):
        Args:
            charRef (char8):
            uint32Ref (uint32):
            vectorUInt64Ref (uint64[]):
            plgVector4Ref (vec4):
            plgStringRef (string):
            boolRef (bool):
            int64Ref (int64):
            vec2Ref (vec2):
            int8Ref (int8):
            uint16Ref (uint16):
            vectorInt16Ref (int16[]):
            mat4x4Ref (mat4x4):
            vec3Ref (vec3):
            floatRef (float):
            vectorDoubleRef (double[]):

        Returns:
            vec3:
    """
    ...

def CallFunc32Callback(func: Callable[[int, int, list[int], Vector4, int, list[int], Matrix4x4, int, str, int, Vector2, list[int], bool, Vector3, int, list[str]], float]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func32):
        Args:
            p1 (int32):
            p2 (uint16):
            p3 (int8[]):
            p4 (vec4):
            p5 (ptr64):
            p6 (uint32[]):
            p7 (mat4x4):
            p8 (uint64):
            p9 (string):
            p10 (int64):
            p11 (vec2):
            p12 (int8[]):
            p13 (bool):
            p14 (vec3):
            p15 (uint8):
            p16 (char16[]):

        Returns:
            double:
    """
    ...

def CallFunc33Callback(func: Callable[[object], None]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (Func33):
        Args:
            variant (any):
    """
    ...

def CallFuncEnumCallback(func: Callable[[Example, list[Example]], list[Example]]) -> str:
    """

    Args:
        func (function):

    Returns:
        string:

    Callback Prototype (FuncEnum):
        Args:
            p1 (int32):
            p2 (int32[]):

        Returns:
            int32[]:
    """
    ...

def ResourceHandleCreate(id: int, name: str) -> int:
    """

    Args:
        id (int32):
        name (string):

    Returns:
        ptr64:
    """
    ...

def ResourceHandleCreateDefault() -> int:
    """

    Returns:
        ptr64:
    """
    ...

def ResourceHandleDestroy(handle: int) -> None:
    """

    Args:
        handle (ptr64):
    """
    ...

def ResourceHandleGetId(handle: int) -> int:
    """

    Args:
        handle (ptr64):

    Returns:
        int32:
    """
    ...

def ResourceHandleGetName(handle: int) -> str:
    """

    Args:
        handle (ptr64):

    Returns:
        string:
    """
    ...

def ResourceHandleSetName(handle: int, name: str) -> None:
    """

    Args:
        handle (ptr64):
        name (string):
    """
    ...

def ResourceHandleIncrementCounter(handle: int) -> None:
    """

    Args:
        handle (ptr64):
    """
    ...

def ResourceHandleGetCounter(handle: int) -> int:
    """

    Args:
        handle (ptr64):

    Returns:
        int32:
    """
    ...

def ResourceHandleAddData(handle: int, value: float) -> None:
    """

    Args:
        handle (ptr64):
        value (float):
    """
    ...

def ResourceHandleGetData(handle: int) -> list[float]:
    """

    Args:
        handle (ptr64):

    Returns:
        float[]:
    """
    ...

def ResourceHandleGetAliveCount() -> int:
    """

    Returns:
        int32:
    """
    ...

def ResourceHandleGetTotalCreated() -> int:
    """

    Returns:
        int32:
    """
    ...

def ResourceHandleGetTotalDestroyed() -> int:
    """

    Returns:
        int32:
    """
    ...

def CounterCreate(initialValue: int) -> int:
    """

    Args:
        initialValue (int64):

    Returns:
        ptr64:
    """
    ...

def CounterCreateZero() -> int:
    """

    Returns:
        ptr64:
    """
    ...

def CounterGetValue(counter: int) -> int:
    """

    Args:
        counter (ptr64):

    Returns:
        int64:
    """
    ...

def CounterSetValue(counter: int, value: int) -> None:
    """

    Args:
        counter (ptr64):
        value (int64):
    """
    ...

def CounterIncrement(counter: int) -> None:
    """

    Args:
        counter (ptr64):
    """
    ...

def CounterDecrement(counter: int) -> None:
    """

    Args:
        counter (ptr64):
    """
    ...

def CounterAdd(counter: int, amount: int) -> None:
    """

    Args:
        counter (ptr64):
        amount (int64):
    """
    ...

def CounterReset(counter: int) -> None:
    """

    Args:
        counter (ptr64):
    """
    ...

def CounterIsPositive(counter: int) -> bool:
    """

    Args:
        counter (ptr64):

    Returns:
        bool:
    """
    ...

def CounterCompare(value1: int, value2: int) -> int:
    """

    Args:
        value1 (int64):
        value2 (int64):

    Returns:
        int32:
    """
    ...

def CounterSum(values: list[int]) -> int:
    """

    Args:
        values (int64[]):

    Returns:
        int64:
    """
    ...

class ResourceHandle:
    """
    RAII wrapper for ResourceHandle pointer
    """
    def __init__(self, id: int, name: str) -> None:
        ...

    def __init__(self) -> None:
        ...

    def __del__(self) -> None:
        """
        Destructor. Releases the underlying handle if owned.
        """
        ...

    def close(self) -> None:
        """
        Close/destroy the handle if owned.
        """
        ...

    def __enter__(self) -> "ResourceHandle":
        """
        Enter the runtime context for this object.

        Returns:
            ResourceHandle: self
        """
        ...

    def __exit__(self, exc_type: type[BaseException] | None, exc_val: BaseException | None, exc_tb: object) -> None:
        """
        Exit the runtime context and release resources.

        Args:
            exc_type (type[BaseException] | None): Exception type if an exception was raised
            exc_val (BaseException | None): Exception value if an exception was raised
            exc_tb (object): Traceback if an exception was raised
        """
        ...

    def get(self) -> int:
        """
        Get the raw handle value without transferring ownership.

        Returns:
            int: The underlying handle value
        """
        ...

    def release(self) -> int:
        """
        Release ownership of the handle and return it.

        Returns:
            int: The released handle value
        """
        ...

    def reset(self) -> None:
        """
        Reset the handle by closing it.
        """
        ...

    def valid(self) -> bool:
        """
        Check if the handle is valid.

        Returns:
            bool: True if the handle is valid, False otherwise
        """
        ...

    def GetId(self) -> int:
        ...

    def GetName(self) -> str:
        ...

    def SetName(self, name: str) -> None:
        ...

    def IncrementCounter(self) -> None:
        ...

    def GetCounter(self) -> int:
        ...

    def AddData(self, value: float) -> None:
        ...

    def GetData(self) -> list[float]:
        ...

    @staticmethod
    def GetAliveCount() -> int:
        ...

    @staticmethod
    def GetTotalCreated() -> int:
        ...

    @staticmethod
    def GetTotalDestroyed() -> int:
        ...


class Counter:
    def __init__(self, initialValue: int) -> None:
        ...

    def __init__(self) -> None:
        ...

    def get(self) -> int:
        """
        Get the raw handle value without transferring ownership.

        Returns:
            int: The underlying handle value
        """
        ...

    def release(self) -> int:
        """
        Release ownership of the handle and return it.

        Returns:
            int: The released handle value
        """
        ...

    def reset(self) -> None:
        """
        Reset the handle by closing it.
        """
        ...

    def valid(self) -> bool:
        """
        Check if the handle is valid.

        Returns:
            bool: True if the handle is valid, False otherwise
        """
        ...

    def GetValue(self) -> int:
        ...

    def SetValue(self, value: int) -> None:
        ...

    def Increment(self) -> None:
        ...

    def Decrement(self) -> None:
        ...

    def Add(self, amount: int) -> None:
        ...

    def Reset(self) -> None:
        ...

    def IsPositive(self) -> bool:
        ...

    @staticmethod
    def Compare(value1: int, value2: int) -> int:
        ...

    @staticmethod
    def Sum(values: list[int]) -> int:
        ...


