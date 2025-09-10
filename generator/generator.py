#!/usr/bin/python3
import sys
import argparse
import os
import json
from enum import IntEnum


TYPES_MAP = {
    'void': 'None',
    'bool': 'bool',
    'char8': 'str',
    'char16': 'str',
    'int8': 'int',
    'int16': 'int',
    'int32': 'int',
    'int64': 'int',
    'uint8': 'int',
    'uint16': 'int',
    'uint32': 'int',
    'uint64': 'int',
    'ptr64': 'int',
    'float': 'float',
    'double': 'float',
    'function': 'delegate',
    'string': 'str',
    'any': 'object',
    'bool[]': 'list[bool]',
    'char8[]': 'list[str]',
    'char16[]': 'list[str]',
    'int8[]': 'list[int]',
    'int16[]': 'list[int]',
    'int32[]': 'list[int]',
    'int64[]': 'list[int]',
    'uint8[]': 'list[int]',
    'uint16[]': 'list[int]',
    'uint32[]': 'list[int]',
    'uint64[]': 'list[int]',
    'ptr64[]': 'list[int]',
    'float[]': 'list[float]',
    'double[]': 'list[float]',
    'string[]': 'list[str]',
    'any[]': 'list[object]',
    'vec2[]': 'list[Vector2]',
    'vec3[]': 'list[Vector3]',
    'vec4[]': 'list[Vector4]',
    'mat4x4[]': 'list[Matrix4x4]',
    'vec2': 'Vector2',
    'vec3': 'Vector3',
    'vec4': 'Vector4',
    'mat4x4': 'Matrix4x4'
}


INVALID_NAMES = {
    'False',   
    'await',       
    'else',        
    'import',     
    'pass', 
    'None',       
    'break',       
    'except',      
    'in',         
    'raise', 
    'True',        
    'class',       
    'finally',     
    'is',          
    'return', 
    'and',         
    'continue',    
    'for',        
    'lambda',      
    'try', 
    'as',         
    'def',        
    'from',        
    'nonlocal',    
    'while', 
    'assert',     
    'del',         
    'global',     
    'not',        
    'with', 
    'async',       
    'elif',       
    'if',         
    'or',          
    'yield'
}


def gen_delegate(prototype: dict) -> str:
    """Generate a delegate type definition."""
    param_types = prototype.get('paramTypes', [])
    ret_type = prototype.get('retType', {})
    return f'Callable[[{gen_params(param_types, ParamGen.Types)}], {convert_type(ret_type)}]'


def convert_type(param: dict) -> str:
    """Convert a JSON-defined type to Python typing."""
    type_name = param.get('type', '')
    result = TYPES_MAP.get(type_name)
    #if not result:
    #    raise ValueError(f'Unsupported type: {type_name}')
    if result == 'delegate':
        return gen_delegate(param.get('prototype')) if 'prototype' in param else 'Callable[..., Any]'
    elif 'enum' in param:
        if '[]' in type_name:
            return f'list[{generate_name(param["enum"].get("name", "UnnamedEnum"))}]'
        else:
            return generate_name(param['enum'].get('name', 'UnnamedEnum'))
    return result
 

def generate_name(name: str) -> str:
    """Generate a valid Python variable name."""
    return f'{name}_' if name in INVALID_NAMES else name


class ParamGen(IntEnum):
    """Enumeration for parameter generation modes."""
    Types = 1
    Names = 2
    TypesNames = 3


def gen_params(params: list[dict], param_gen: ParamGen) -> str:
    """Generate function parameters as strings."""
    def gen_param(index: int, param: dict) -> str:
        return convert_type(param) if param_gen == ParamGen.Types else f'{generate_name(param.get("name", f"p{index}"))}: {convert_type(param)}'

    result = []
    if params:
        for i, p in enumerate(params):
            result.append(gen_param(i, p))
    return ', '.join(result)


def gen_return(ret_type: dict, param_types: list[dict]) -> str:
    if not any('ref' in p and p['ref'] is True for p in param_types):
        return convert_type(ret_type)
    result = [convert_type(ret_type)]
    for p in param_types:
        if 'ref' in p and p['ref'] is True:
            result.append(convert_type(p))
    return f'tuple[{",".join(result)}]'


def gen_documentation(method: dict) -> str:
    """
    Generate a Python function documentation string from a JSON block.

    Args:
        method (Dict[str, Any]): The input JSON data describing the function.

    Returns:
        str: The generated documentation string.
    """
    # Extract general details
    name = method.get('name', 'UnnamedFunction')
    description = method.get('description', 'No description provided.')
    param_types = method.get('paramTypes', [])
    ret_type = method.get('retType', {}).get('type', 'void')

    # Start building the docstring
    docstring = [f'    """\n    {description}\n    Args:\n']

    # Add parameters
    for param in param_types:
        param_name = param.get('name')
        param_type = param.get('type', 'Any')
        param_desc = param.get('description', 'No description available.')
        docstring.append(f'        {param_name} ({param_type}): {param_desc}\n')

    # Add return type
    if ret_type.lower() != 'void':
        ret_desc = method.get('retType', {}).get('description', 'No description available.')
        docstring.append(f'\n    Returns:\n        {ret_type}: {ret_desc}\n')

    # Add callback prototype if present
    for param in param_types:
        if param.get('type') == 'function' and 'prototype' in param:
            prototype = param['prototype']
            proto_name = prototype.get('name', 'UnnamedCallback')
            proto_desc = prototype.get('description', 'No description provided.')
            proto_params = prototype.get('paramTypes', [])
            proto_ret = prototype.get('retType', {})

            docstring.append(f'\n    Callback Prototype ({proto_name}):\n        {proto_desc}\n\n')
            docstring.append('        Args:\n')
            for proto_param in proto_params:
                p_name = proto_param.get('name')
                p_type = proto_param.get('type', 'Any')
                p_desc = proto_param.get('description', 'No description available.')
                docstring.append(f'            {p_name} ({p_type}): {p_desc}\n')

            if proto_ret:
                proto_ret_type = proto_ret.get('type', 'void')
                proto_ret_desc = proto_ret.get('description', 'No description available.')
                docstring.append(f'\n        Returns:\n            {proto_ret_type}: {proto_ret_desc}\n')

    # Close docstring
    docstring.append('    """')

    return ''.join(docstring)


def gen_enum_body(enum: dict, enum_type: str, enums: set[str]) -> str:
    """
    Generates a Python enum definition from the provided enum metadata.

    Args:
        enum (dict): The JSON dictionary describing the enum.
        enum_type (str): The underlying type of the enum (not directly used in Python enums).
        enums (set): A set to track already defined enums to prevent duplicates.

    Returns:
        str: The generated Python enum code or an empty string if the enum already exists.
    """
    # Extract enum name and values
    enum_name = enum.get('name', 'InvalidEnum')
    enum_description = enum.get('description', '')
    enum_values = enum.get('values', [])

    # Check for duplicate enums
    if enum_name in enums:
        return ''  # Skip if already generated

    # Add the enum name to the set
    enums.add(enum_name)

    # Start building the enum definition
    enum_code = [f'class {enum_name}(IntEnum):']
    if enum_description:
        enum_code.append(f"    \"\"\"\n    {enum_description}\n    \"\"\"")

    # Iterate over the enum values and generate corresponding Python entries
    for i, value in enumerate(enum_values):
        name = value.get('name', f'InvalidName_{i}')
        enum_value = value.get('value', str(i))
        description = value.get('description', '')

        # Add comment for each value
        if description:
            enum_code.append(f'    # {description}')
        enum_code.append(f'    {name} = {enum_value}')

    # Join the list into a single formatted string
    return '\n'.join(enum_code)


def generate_enum_code(pplugin: dict, enums: set[str]) -> str:
    """
    Generate Python enum code from a plugin definition.
    """
    # Container for all generated enum code
    content = []

    def process_enum(enum_data: dict, enum_type: str):
        """
        Generate enum code from the given enum data if it hasn't been processed.
        """
        enum_code = gen_enum_body(enum_data, enum_type, enums)
        if enum_code:
            content.append(enum_code)
            content.append('\n')

    def process_prototype(prototype: dict):
        """
        Recursively process a function prototype for enums.
        """
        if 'enum' in prototype.get('retType', {}):
            process_enum(prototype['retType']['enum'], prototype['retType'].get('type', ''))

        for param in prototype.get('paramTypes', []):
            if 'enum' in param:
                process_enum(param['enum'], param.get('type', ''))
            if 'prototype' in param:  # Process nested prototypes
                process_prototype(param['prototype'])

    # Main loop: Process all exported methods in the plugin
    for method in pplugin.get('methods', []):
        if 'retType' in method and 'enum' in method['retType']:
            process_enum(method['retType']['enum'], method['retType'].get('type', ''))

        for param in method.get('paramTypes', []):
            if 'enum' in param:
                process_enum(param['enum'], param.get('type', ''))
            if 'prototype' in param:  # Handle nested function prototypes
                process_prototype(param['prototype'])

    # Join all generated enums into a single string
    return '\n'.join(content)

    
def generate_stub(plugin_name: str, pplugin: dict) -> str:
    """Generate Python stub content."""
    link = 'https://github.com/untrustedmodders/plugify-module-python3.12/blob/main/generator/generator.py'
    content = [
        'from collections.abc import Callable',
        'from enum import IntEnum',
        'from plugify.plugin import Vector2, Vector3, Vector4, Matrix4x4\n\n'
        f'# Generated from {plugin_name}.pplugin by {link}\n\n']

    # Append enum definitions
    enums = set()
    content.append(generate_enum_code(pplugin, enums))

    if len(enums) == 0:
        content.pop(1)

    for method in pplugin.get('methods', []):
        method_name = method.get('name', 'UnnamedMethod')
        param_types = method.get('paramTypes', [])
        ret_type = method.get('retType', {})

        content.append(f'def {method_name}({gen_params(param_types, ParamGen.TypesNames)}) ->'
                       f' {gen_return(ret_type, param_types)}:')
        content.append(gen_documentation(method))
        content.append(f'    ...\n')

    if not any('Callable' in s for s in content[1:]):
        content.pop(0)

    return '\n'.join(content)


def main(manifest_path: str, output_dir: str, override: bool):
    """Main entry point for the script."""
    if not os.path.isfile(manifest_path):
        print(f'Manifest file does not exist: {manifest_path}')
        return 1
    if not os.path.isdir(output_dir):
        print(f'Output directory does not exist: {output_dir}')
        return 1
        
    try:
        with open(manifest_path, 'r', encoding='utf-8') as file:
            pplugin = json.load(file)

    except Exception as e:
        print(f'An error occurred: {e}')
        return 1

    plugin_name = pplugin.get('name', os.path.basename(manifest_path).rsplit('.', 3)[0])
    output_path = os.path.join(output_dir, 'pps', f'{plugin_name}.pyi')
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    if os.path.isfile(output_path) and not override:
        print(f'Output file already exists: {output_path}. Use --override to overwrite existing file.')
        return 1

    try:
        content = generate_stub(plugin_name, pplugin)
        
        with open(output_path, 'w', encoding='utf-8') as file:
            file.write(content)

    except Exception as e:
        print(f'An error occurred: {e}')
        return 1
    
    print(f'Stub generated at: {output_path}')
    return 0


def get_args():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(description='Generate Python .pyi stub files for plugin manifests.')
    parser.add_argument('manifest', help='Path to the plugin manifest file')
    parser.add_argument('output', help='Output directory for the generated stub')
    parser.add_argument('--override', action='store_true', help='Override existing files')
    return parser.parse_args()


if __name__ == '__main__':
    args = get_args()
    sys.exit(main(args.manifest, args.output, args.override))
