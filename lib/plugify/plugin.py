import ast
import os
import importlib.util
from copy import deepcopy
from enum import Enum
from typing import Any, Callable, List, Optional, Union, Tuple


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
        rows = ", ".join(f"[{', '.join(map(str, r))}]" for r in self.m)
        return f"Matrix4x4({rows})"

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
        constructors: Union[List[Callable], Tuple[Callable, ...]],
        destructor: Optional[Callable],
        methods: Union[List[Tuple[str, Callable, bool, Optional[List[Tuple[str, bool]]]]], Tuple[Tuple[str, Callable, bool, Optional[List[Tuple[str, bool]]], ...]]],
        invalid_value: Any = 0
):
    """
    Dynamically bind methods to a class for RAII handle management.

    Args:
        cls: The class to extend with methods
        constructors: Tuple/List of constructor functions
        destructor: Destructor function (can be None)
        methods: Tuple/List of (name, func, bindSelf, paramAliases, retAlias)
                 - name (str): Method name
                 - func (callable): Underlying C function
                 - bindSelf (bool): If True, pass self._handle; if False, make static method
                 - paramAliases: tuple of Alias named tuples or tuples (name, owner)
                 - retAlias: Alias named tuple or tuple (name, owner) or empty tuple
        invalid_value: Value representing an invalid/closed handle (default: 0)
    """

    class_name = cls.__name__

    def __init__(self, *args, **kwargs):
        # Initialize to invalid state first (in case of exceptions)
        self._handle = invalid_value
        self._owned = Ownership.BORROWED

        # Check if this is handle + ownership construction
        # Pattern: ClassName(handle_value, Ownership.OWNED/BORROWED)
        if len(args) == 2 and isinstance(args[1], Ownership):
            self._handle = args[0]
            self._owned = args[1]
            return

        # Check if this is handle-only construction (assume OWNED by default)
        # Pattern: ClassName(handle_value)
        if len(args) == 1 and len(constructors) == 0:
            self._handle = args[0]
            self._owned = Ownership.OWNED
            return

        # Constructor call mode
        if len(constructors) == 0:
            raise ValueError(
                f"{className} has no constructors. "
                f"Use: {className}(handle, Ownership.OWNED/BORROWED) or "
                f"{className}(handle) to wrap an existing handle."
            )

        # Try constructors
        errors = []
        for i, constructor in enumerate(constructors):
            try:
                self._handle = constructor(*args, **kwargs)
                self._owned = Ownership.OWNED
                return  # Success!
            except Exception as e:
                errors.append(f"Constructor {i}: {e}")

        # If we get here, all constructors failed
        raise ValueError(
            f"No constructor matched the arguments for {className}.\n"
            f"Tried {len(constructors)} constructor(s):\n" + "\n".join(errors)
        )

    cls.__init__ = __init__

    def close(self):
        """Close/destroy the handle if owned."""
        # Protect against being called before __init__ completes or on already-closed handles
        if not hasattr(self, '_handle'):
            return

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

    def release(self) -> Any:
        """Release ownership of the handle and return it."""
        if not hasattr(self, '_handle'):
            return invalid_value
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
        if not hasattr(self, '_handle'):
            return invalid_value
        return self._handle

    cls.get = get

    def valid(self) -> bool:
        """Check if the handle is valid."""
        if not hasattr(self, '_handle'):
            return False
        return self._handle != invalid_value

    cls.valid = valid

    _class_registry[class_name] = cls

    # Process methods
    for method_info in methods:
        method_name = method_info[0]
        func = method_info[1]
        bind_self = method_info[2]
        param_aliases = method_info[3]
        ret_alias = method_info[4]

        # Helper to handle return value wrapping
        def wrap_return(result, ret_alias, invalid_value):
            if ret_alias and len(ret_alias) >= 2:
                ret_class_name = ret_alias[0]
                owner = ret_alias[1]
                ownership = Ownership.OWNED if owner else Ownership.BORROWED

                ret_class = _class_registry.get(ret_class_name)
                if not ret_class:
                    import sys
                    frame = sys._getframe(2)  # Go up 2 frames
                    while frame:
                        if ret_class_name in frame.f_globals:
                            ret_class = frame.f_globals[ret_class_name]
                            _class_registry[ret_class_name] = ret_class
                            break
                        frame = frame.f_back

                if ret_class and result != invalid_value:
                    return ret_class(result, ownership)
                elif result == invalid_value:
                    return None

            return result

        # Helper to process parameter aliases
        def process_param_aliases(args, param_aliases):
            args_list = list(args)
            for i, alias_info in enumerate(param_aliases):
                if alias_info and len(alias_info) >= 2 and i < len(args_list):
                    alias_name = alias_info[0]
                    owner = alias_info[1]

                    if alias_name and args_list[i] is not None:
                        arg = args_list[i]
                        if hasattr(arg, 'release') and hasattr(arg, 'get'):
                            if owner:
                                args_list[i] = arg.release()
                            else:
                                args_list[i] = arg.get()
            return args_list

        if not bind_self:
            # Static method - doesn't need self or handle
            def create_static_method(func, param_aliases, ret_alias, method_name, invalid_value):
                def static_method(*args, **kwargs):
                    args_list = process_param_aliases(args, param_aliases)
                    result = func(*args_list, **kwargs)
                    return wrap_return(result, ret_alias, invalid_value)

                static_method.__name__ = method_name
                return staticmethod(static_method)

            setattr(cls, method_name, create_static_method(func, param_aliases, ret_alias, method_name, invalid_value))

        else:
            # Instance method - binds self._handle
            def create_method(func, param_aliases, ret_alias, method_name, invalid_value, class_name):
                def method(self, *args, **kwargs):
                    if self._handle == invalid_value:
                        raise RuntimeError(f"{class_name} handle is closed")

                    args_list = process_param_aliases(args, param_aliases)
                    result = func(self._handle, *args_list, **kwargs)
                    return wrap_return(result, ret_alias, invalid_value)

                method.__name__ = method_name
                return method

            setattr(cls, method_name, create_method(func, param_aliases, ret_alias, method_name, invalid_value, class_name))

    return cls
