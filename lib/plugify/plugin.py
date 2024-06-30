class Plugin:
    pass


class PluginInfo:
    def __init__(self, class_name):
        self.class_name = class_name
        self.instance = None


class Vector2:
    def __init__(self, x=0.0, y=0.0):
        self.x = x
        self.y = y

    def __add__(self, other):
        if isinstance(other, Vector2):
            return Vector2(self.x + other.x, self.y + other.y)
        raise ValueError("Can only add another Vector2")

    def __sub__(self, other):
        if isinstance(other, Vector2):
            return Vector2(self.x - other.x, self.y - other.y)
        raise ValueError("Can only subtract another Vector2")

    def __mul__(self, scalar):
        if isinstance(scalar, (int, float)):
            return Vector2(self.x * scalar, self.y * scalar)
        raise ValueError("Can only multiply by a scalar")

    def __truediv__(self, scalar):
        if isinstance(scalar, (int, float)):
            return Vector2(self.x / scalar, self.y / scalar)
        raise ValueError("Can only divide by a scalar")

    def __repr__(self):
        return f"Vector2({self.x}, {self.y})"


class Vector3:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x = x
        self.y = y
        self.z = z

    def __add__(self, other):
        if isinstance(other, Vector3):
            return Vector3(self.x + other.x, self.y + other.y, self.z + other.z)
        raise ValueError("Can only add another Vector3")

    def __sub__(self, other):
        if isinstance(other, Vector3):
            return Vector3(self.x - other.x, self.y - other.y, self.z - other.z)
        raise ValueError("Can only subtract another Vector3")

    def __mul__(self, scalar):
        if isinstance(scalar, (int, float)):
            return Vector3(self.x * scalar, self.y * scalar, self.z * scalar)
        raise ValueError("Can only multiply by a scalar")

    def __truediv__(self, scalar):
        if isinstance(scalar, (int, float)):
            return Vector3(self.x / scalar, self.y / scalar, self.z / scalar)
        raise ValueError("Can only divide by a scalar")

    def __repr__(self):
        return f"Vector3({self.x}, {self.y}, {self.z})"


class Vector4:
    def __init__(self, x=0.0, y=0.0, z=0.0, w=0.0):
        self.x = x
        self.y = y
        self.z = z
        self.w = w

    def __add__(self, other):
        if isinstance(other, Vector4):
            return Vector4(self.x + other.x, self.y + other.y, self.z + other.z, self.w + other.w)
        raise ValueError("Can only add another Vector4")

    def __sub__(self, other):
        if isinstance(other, Vector4):
            return Vector4(self.x - other.x, self.y - other.y, self.z - other.z, self.w - other.w)
        raise ValueError("Can only subtract another Vector4")

    def __mul__(self, scalar):
        if isinstance(scalar, (int, float)):
            return Vector4(self.x * scalar, self.y * scalar, self.z * scalar, self.w * scalar)
        raise ValueError("Can only multiply by a scalar")

    def __truediv__(self, scalar):
        if isinstance(scalar, (int, float)):
            return Vector4(self.x / scalar, self.y / scalar, self.z / scalar, self.w / scalar)
        raise ValueError("Can only divide by a scalar")

    def __repr__(self):
        return f"Vector4({self.x}, {self.y}, {self.z}, {self.w})"


class Matrix4x4:
    def __init__(self, elements=None):
        if elements is None:
            # Initialize to an identity matrix
            self.elements = [
                [1.0, 0.0, 0.0, 0.0],
                [0.0, 1.0, 0.0, 0.0],
                [0.0, 0.0, 1.0, 0.0],
                [0.0, 0.0, 0.0, 1.0]
            ]
        else:
            if isinstance(elements, list) and len(elements) == 16:
                self.elements = [elements[0:4], elements[4:8], elements[8:12], elements[12:16]]
            elif (isinstance(elements, list) and len(elements) == 4
                  and all(isinstance(row, list) and len(row) == 4 for row in elements)):
                self.elements = elements
            else:
                raise ValueError("Elements must be a 4x4 or 1x16 list")

    def __add__(self, other):
        if isinstance(other, Matrix4x4):
            return Matrix4x4([[self.elements[i][j] + other.elements[i][j] for j in range(4)] for i in range(4)])
        raise ValueError("Can only add another Matrix4x4")

    def __sub__(self, other):
        if isinstance(other, Matrix4x4):
            return Matrix4x4([[self.elements[i][j] - other.elements[i][j] for j in range(4)] for i in range(4)])
        raise ValueError("Can only subtract another Matrix4x4")

    def __mul__(self, other):
        if isinstance(other, Matrix4x4):
            result = [[0] * 4 for _ in range(4)]
            for i in range(4):
                for j in range(4):
                    result[i][j] = sum(self.elements[i][k] * other.elements[k][j] for k in range(4))
            return Matrix4x4(result)
        elif isinstance(other, (int, float)):
            return Matrix4x4([[self.elements[i][j] * other for j in range(4)] for i in range(4)])
        raise ValueError("Can only multiply by another Matrix4x4 or a scalar")

    def __truediv__(self, scalar):
        if isinstance(scalar, (int, float)):
            return Matrix4x4([[self.elements[i][j] / scalar for j in range(4)] for i in range(4)])
        raise ValueError("Can only divide by a scalar")

    def __repr__(self):
        return "\n".join([f"Row {i}: {self.elements[i]}" for i in range(4)])

    def transpose(self):
        return Matrix4x4([[self.elements[j][i] for j in range(4)] for i in range(4)])

    @staticmethod
    def identity():
        return Matrix4x4()

    @staticmethod
    def zero():
        return Matrix4x4([[0] * 4 for _ in range(4)])

    @staticmethod
    def from_list(elements):
        return Matrix4x4(elements)

    def to_list(self):
        return self.elements
