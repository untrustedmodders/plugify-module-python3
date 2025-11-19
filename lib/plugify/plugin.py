import ast
import os
import importlib.util
from copy import deepcopy
from enum import Enum
from typing import Any, Callable, List, Optional, Dict, Tuple


class Plugin:
    def __init__(self, id, name, description, version, author, website, license, location, dependencies, base_dir, extensions_dir, configs_dir, data_dir, logs_dir, cache_dir):
        self.id = id
        self.name = name
        self.description = description
        self.version = version
        self.author = author
        self.website = website
        self.license = license
        self.location = location
        self.dependencies = dependencies

        self.base_dir = base_dir
        self.extensions_dir = extensions_dir
        self.configs_dir = configs_dir
        self.data_dir = data_dir
        self.logs_dir = logs_dir
        self.cache_dir = cache_dir


#base_dir, extensions_dir, configs_dir, data_dir, logs_dir, cache_dir

class PluginInfo:
    def __init__(self, class_name, instance):
        self.class_name = class_name
        self.instance = instance


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
    def __init__(self, m=None):
        if m is None:
            # Initialize to an identity matrix
            self.m = [
                [1.0, 0.0, 0.0, 0.0],
                [0.0, 1.0, 0.0, 0.0],
                [0.0, 0.0, 1.0, 0.0],
                [0.0, 0.0, 0.0, 1.0]
            ]
        else:
            if isinstance(m, list) and len(m) == 16:
                self.m = [m[0:4], m[4:8], m[8:12], m[12:16]]
            elif (isinstance(m, list) and len(m) == 4
                  and all(isinstance(row, list) and len(row) == 4 for row in m)):
                self.m = m
            else:
                raise ValueError("Elements must be a 4x4 or 1x16 list")

    def __add__(self, other):
        if isinstance(other, Matrix4x4):
            return Matrix4x4([[self.m[i][j] + other.m[i][j] for j in range(4)] for i in range(4)])
        raise ValueError("Can only add another Matrix4x4")

    def __sub__(self, other):
        if isinstance(other, Matrix4x4):
            return Matrix4x4([[self.m[i][j] - other.m[i][j] for j in range(4)] for i in range(4)])
        raise ValueError("Can only subtract another Matrix4x4")

    def __mul__(self, other):
        if isinstance(other, Matrix4x4):
            result = [[0] * 4 for _ in range(4)]
            for i in range(4):
                for j in range(4):
                    result[i][j] = sum(self.m[i][k] * other.m[k][j] for k in range(4))
            return Matrix4x4(result)
        elif isinstance(other, (int, float)):
            return Matrix4x4([[self.m[i][j] * other for j in range(4)] for i in range(4)])
        raise ValueError("Can only multiply by another Matrix4x4 or a scalar")

    def __truediv__(self, scalar):
        if isinstance(scalar, (int, float)):
            return Matrix4x4([[self.m[i][j] / scalar for j in range(4)] for i in range(4)])
        raise ValueError("Can only divide by a scalar")

    def __repr__(self):
        return "\n".join([f"Row {i}: {self.m[i]}" for i in range(4)])

    def transpose(self):
        return Matrix4x4([[self.m[j][i] for j in range(4)] for i in range(4)])

    @staticmethod
    def identity():
        return Matrix4x4()

    @staticmethod
    def zero():
        return Matrix4x4([[0.0] * 4 for _ in range(4)])

    @staticmethod
    def from_list(m):
        return Matrix4x4(m)

    def to_list(self):
        return deepcopy(self.m)


def extract_required_modules(module_path: str, visited: Optional[set] = None):
    """
    Recursively extract all imported modules and their fully qualified names.

    Args:
        module_path (str): Path to the Python module file to analyze.
        visited (set): A set of visited modules to prevent circular dependencies.

    Returns:
        set: A set of fully qualified names of all imports.
    """
    if visited is None:
        visited = set()

    # Avoid processing the same module multiple times
    if module_path in visited:
        return set()

    visited.add(module_path)
    required_modules = set()

    try:
        with open(module_path, "r", encoding="utf-8") as file:
            tree = ast.parse(file.read(), filename=module_path)

        for node in ast.walk(tree):
            if isinstance(node, ast.Import):
                for alias in node.names:
                    required_modules.add(alias.name)
            elif isinstance(node, ast.ImportFrom):
                if node.module:
                    for alias in node.names:
                        required_modules.add(f"{node.module}.{alias.name}")
    except Exception as e:
        print(f"Error processing {module_path}: {e}")
        return required_modules

    def find_module_path(module_name: str):
        """
        Locate the file path of a given Python module name, ensuring it's a .py file.
        """
        try:
            spec = importlib.util.find_spec(module_name)
            if spec and spec.origin and spec.origin.endswith(".py"):
                return spec.origin
        except Exception as e:
            #print(f"Error finding module path for {module_name}: {e}")
            pass
        return None

    all_dependencies = set(required_modules)
    try:
        for module_name in required_modules:
            base_module = module_name.split('.')[0]
            module_file = find_module_path(base_module)
            if module_file and os.path.isfile(module_file):
                all_dependencies.update(extract_required_modules(module_file, visited))
    except Exception as e:
        print(f"Error processing dependencies for {module_path}: {e}")

    return all_dependencies


class Ownership(Enum):
    OWNED = True
    BORROWED = False


# Store class registry for retAlias lookups
_class_registry = {}


def bind_class_methods(
        cls: type,
        constructors: List[Callable],
        destructor: Optional[Callable],
        methods: List[Tuple[str, Callable, bool, Optional[List[Tuple[str, bool]]], Optional[Tuple[str, bool]]]],
        invalid_value: Any = 0
):
    """
    Dynamically bind methods to a class for RAII handle management.

    Args:
        cls: The class to extend with methods
        constructors: List of constructor functions (can be empty for handle-only construction)
        destructor: Destructor function (can be None)
        methods: List of [name, func, bindSelf, paramAliases, retAlias]
                 - name (str): Method name
                 - func (callable): Underlying C function
                 - bindSelf (bool): Whether to pass self._handle as first param
                 - paramAliases (list): List of pairs with 'name' and 'owner' values
                 - retAlias (dict): Pair with 'name' and 'owner' values
        invalid_value: Value representing an invalid/closed handle (default: 0)
    """

    class_name = cls.__name__

    # 1. Add __init__ method
    def __init__(self, *args, **kwargs):
        """
        Initialize the wrapper. Supports two modes:
        1. Direct handle construction: ClassName(handle, Ownership.OWNED/BORROWED)
        2. Constructor call: ClassName(*constructor_args)
        """
        # Check if this is handle + ownership construction
        if len(args) >= 2 and isinstance(args[1], Ownership):
            self._handle = args[0]
            self._owned = args[1]
        else:
            # Constructor call
            if len(constructors) == 0:
                raise ValueError(f"{class_name} requires handle and ownership for construction")
            elif len(constructors) == 1:
                # Single constructor - call it directly
                self._handle = constructors[0](*args, **kwargs)
            else:
                # Multiple constructors - first arg should be the constructor function
                if len(args) == 0:
                    raise ValueError(
                        f"{class_name} with multiple constructors requires constructor function as first argument")
                func = args[0]
                if func not in constructors:
                    raise ValueError(f"Invalid constructor function for {class_name}")
                self._handle = func(*args[1:], **kwargs)

            self._owned = Ownership.OWNED

    cls.__init__ = __init__

    # 2. Add lifecycle methods (close, __del__, __enter__, __exit__)
    def close(self):
        """Close/destroy the handle if owned."""
        if self._handle != invalid_value and self._owned == Ownership.OWNED:
            if destructor is not None:
                destructor(self._handle)
        self._handle = invalid_value
        self._owned = Ownership.BORROWED

    cls.close = close

    def __del__(self):
        self.close()

    cls.__del__ = __del__

    def __enter__(self):
        return self

    cls.__enter__ = __enter__

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False

    cls.__exit__ = __exit__

    # 3. Add utility methods (release, reset, get, valid)
    def release(self) -> Any:
        """Release ownership of the handle and return it."""
        tmp = self._handle
        self._handle = invalid_value
        self._owned = Ownership.BORROWED
        return tmp

    cls.release = release

    def reset(self):
        """Reset the handle by closing it."""
        self.close()

    cls.reset = reset

    def get(self) -> Any:
        """Get the raw handle value without transferring ownership."""
        return self._handle

    cls.get = get

    def valid(self) -> bool:
        """Check if the handle is valid."""
        return self._handle != invalid_value

    cls.valid = valid

    # Register this class for retAlias lookups
    _class_registry[class_name] = cls

    # 4. Add bound methods from the methods list
    for method_info in methods:
        method_name = method_info[0]
        func = method_info[1]
        bind_self = method_info[2]
        param_aliases = method_info[3]
        ret_alias = method_info[4]

        # Create the bound method with closure over parameters
        def create_method(
                func: Callable,
                bind_self: bool,
                param_aliases: Optional[List[Tuple[str, bool]]],
                ret_alias: Optional[Tuple[str, bool]],
                method_name: str
        ):
            def method(self, *args, **kwargs):
                # Check if handle is valid
                if self._handle == invalid_value:
                    raise RuntimeError(f"{class_name} handle is closed")

                # Process arguments - convert to list for modification
                args_list = list(args)

                # Handle paramAliases - extract handles from wrapper objects
                if param_aliases:
                    for i, alias_info in enumerate(param_aliases):
                        if alias_info and i < len(args_list):
                            alias_name = alias_info[0]
                            owner = alias_info[1]

                            if alias_name and args_list[i] is not None:
                                arg = args_list[i]
                                # Check if the argument has the expected methods
                                if hasattr(arg, 'release') and hasattr(arg, 'get'):
                                    if owner:
                                        # Transfer ownership - use release()
                                        args_list[i] = arg.release()
                                    else:
                                        # Borrow - use get()
                                        args_list[i] = arg.get()

                # Call the underlying function
                if bind_self:
                    # Pass self._handle as first parameter
                    result = func(self._handle, *args_list, **kwargs)
                else:
                    # Don't pass self._handle
                    result = func(*args_list, **kwargs)

                # Handle retAlias - wrap return value in class
                if ret_alias:
                    ret_name = ret_alias[0]
                    owner = ret_alias[1]

                    # Look up the class
                    ret_class = _class_registry.get(ret_name)
                    if ret_class and result != invalid_value:
                        ownership = Ownership.OWNED if owner else Ownership.BORROWED
                        return ret_class(result, ownership)
                    elif result == invalid_value:
                        return None

                return result

            # Preserve function name for better debugging
            method.__name__ = method_name
            return method

        # Bind the method to the class
        setattr(cls, method_name, create_method(func, bind_self, param_aliases, ret_alias, method_name))

    return cls
