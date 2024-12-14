#!/usr/bin/python3
import sys
import argparse
import os
import json
from enum import Enum


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
    'vec2': 'list[Vector2]',
    'vec3': 'list[Vector3]',
    'vec4': 'list[Vector4]',
    'mat4x4': 'list[Matrix4x4]'
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
    if not result:
        raise ValueError(f"Unsupported type: {type_name}")
    if result == 'delegate':
        return gen_delegate(param.get('prototype')) if 'prototype' in param else 'Callable[..., Any]'
    return result
 

def generate_name(name: str) -> str:
    """Generate a valid Python variable name."""
    return f'{name}_' if name in INVALID_NAMES else name


class ParamGen(Enum):
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

    
def generate_stub(plugin_name: str, pplugin: dict) -> str:
    """Generate Python stub content."""
    link = 'https://github.com/untrustedmodders/plugify-module-python3.12/blob/main/generator/generator.py'
    body = [f'from typing import Callable\nfrom plugify.plugin import Vector2, Vector3, Vector4, Matrix4x4\n\n'
            f'# Generated from {plugin_name}.pplugin by {link}\n\n']
    for method in pplugin.get('exportedMethods', []):
        method_name = method.get('name', None)
        param_types = method.get('paramTypes', [])
        ret_type = method.get('retType', {})
        
        if not method_name:
            continue
        
        signature = f'def {method_name}({gen_params(param_types, ParamGen.TypesNames)}) -> {convert_type(ret_type)}:'
        documentation = gen_documentation(method)
        body.append(f'{signature}\n{documentation}\n    ...\n\n\n')
    return ''.join(body)


def main(manifest_path: str, output_dir: str, override: bool):
    """Main entry point for the script."""
    if not os.path.isfile(manifest_path):
        print(f'Manifest file does not exist: {manifest_path}')
        return 1
    if not os.path.isdir(output_dir):
        print(f'Output directory does not exist: {output_dir}')
        return 1

    plugin_name = os.path.splitext(os.path.basename(manifest_path))[0]
    output_path = os.path.join(output_dir, 'pps', f'{plugin_name}.pyi')
    os.makedirs(os.path.dirname(output_path), exist_ok=True)

    if os.path.isfile(output_path) and not override:
        print(f'Output file already exists: {output_path}. Use --override to overwrite existing file.')
        return 1

    try:
        with open(manifest_path, 'r', encoding='utf-8') as file:
            pplugin = json.load(file)

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
