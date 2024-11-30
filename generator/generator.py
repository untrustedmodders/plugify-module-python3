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

def validate_manifest(pplugin):
    parse_errors = []
    methods = pplugin.get('exportedMethods')
    if type(methods) is list:
        for i, method in enumerate(methods):
            if type(method) is dict:
                if type(method.get('type')) is str:
                    parse_errors += [f'root.exportedMethods[{i}].type not string']
            else:
                parse_errors += [f'root.exportedMethods[{i}] not object']
    else:
        parse_errors += ['root.exportedMethods not array']
    return parse_errors
    
    
def gen_delegate(method):
    return f'Callable[[{gen_params(method["paramTypes"], ParamGen.Types)}], {convert_type(method["retType"])}]'
    
    
def convert_type(param):
    result = TYPES_MAP[param['type']]
    if 'delegate' in result:
        if 'prototype' in param:
            return gen_delegate(param['prototype'])
        else:
            return 'Callable[..., Any]'
    return result
    
 
def generate_name(name):
    if name in INVALID_NAMES:
        return name + '_'
    else:
        return name


class ParamGen(Enum):
    Types = 1
    Names = 2
    TypesNames = 3


def gen_params(params: list[dict], param_gen: ParamGen) -> str:
    def gen_param(param: dict) -> str:
        # Check if the generator is ParamGen.Types and convert accordingly
        return convert_type(param) if param_gen == ParamGen.Types else f'{generate_name(param["name"])}: {convert_type(param)}'

    # Use a list to accumulate the results
    result = []

    if params:
        result.append(gen_param(params[0]))  # Add the first parameter
        for p in params[1:]:  # Loop over the rest of the parameters
            result.append(f', {gen_param(p)}')

    # Join all parts into a single string
    return ''.join(result)
    
    
def gen_documentation(method: list[dict]) -> str:
    """
    Generate a Python function documentation string from a JSON block.

    Args:
        method (Dict[str, Any]): The input JSON data describing the function.

    Returns:
        str: The generated documentation string.
    """
    # Extract general details
    name = method.get("name", "UnnamedFunction")
    description = method.get("description", "No description provided.")
    param_types = method.get("paramTypes", [])
    ret_type = method.get("retType", {}).get("type", "void")

    # Start building the docstring
    docstring = f'\n    """\n    {description}\n    Args:\n'

    # Add parameters
    for param in param_types:
        param_name = param.get("name")
        param_type = param.get("type", "Any")
        param_desc = param.get("description", "No description available.")
        docstring += f"        {param_name} ({param_type}): {param_desc}\n"

    # Add return type
    if ret_type.lower() != "void":
        ret_desc = method.get("retType", {}).get("description", "No description available.")
        docstring += f"\n    Returns:\n        {ret_type}: {ret_desc}\n"

    # Add callback prototype if present
    for param in param_types:
        if param.get("type") == "function" and "prototype" in param:
            prototype = param["prototype"]
            proto_name = prototype.get("name", "UnnamedCallback")
            proto_desc = prototype.get("description", "No description provided.")
            proto_params = prototype.get("paramTypes", [])
            proto_ret = prototype.get("retType", {})

            docstring += f"\n    Callback Prototype ({proto_name}):\n        {proto_desc}\n\n"
            docstring += "        Args:\n"
            for proto_param in proto_params:
                p_name = proto_param.get("name")
                p_type = proto_param.get("type", "Any")
                p_desc = proto_param.get("description", "No description available.")
                docstring += f"            {p_name} ({p_type}): {p_desc}\n"

            if proto_ret:
                proto_ret_type = proto_ret.get("type", "void")
                proto_ret_desc = proto_ret.get("description", "No description available.")
                docstring += f"\n        Returns:\n            {proto_ret_type}: {proto_ret_desc}\n"

    # Close docstring
    docstring += '    """'

    return docstring

    
def main(manifest_path, output_dir, override):
    if not os.path.isfile(manifest_path):
        print(f'Manifest file not exists {manifest_path}')
        return 1
    if not os.path.isdir(output_dir):
        print(f'Output folder not exists {output_dir}')
        return 1

    plugin_name = os.path.splitext(os.path.basename(manifest_path))[0]
    header_dir = os.path.join(output_dir, 'pps')
    if not os.path.exists(header_dir):
        os.makedirs(header_dir, exist_ok=True)
    header_file = os.path.join(header_dir, f'{plugin_name}.pyi')
    if os.path.isfile(header_file) and not override:
        print(f'Already exists {header_file}')
        return 1

    with open(manifest_path, 'r', encoding='utf-8') as fd:
        pplugin = json.load(fd)

    parse_errors = validate_manifest(pplugin)
    if parse_errors:
        print('Parse fail:')
        for error in parse_errors:
            print(f'  {error}')
        return 1

    link = 'https://github.com/untrustedmodders/plugify-module-python3.12/blob/main/generator/generator.py\n'

    content = ('from typing import Callable\n'
               'from plugify.plugin import Vector2, Vector3, Vector4, Matrix4x4\n'
               '\n'
               f'# Generated from {plugin_name}.pplugin by {link} \n\n')
    
    for method in pplugin['exportedMethods']:
        content += (f'def {method["name"]}({gen_params(method["paramTypes"], ParamGen.TypesNames)}) -> {convert_type(method["retType"])}:'
                    f'{gen_documentation(method)}'
                    '\n    ...\n\n\n')

    with open(header_file, 'w', encoding='utf-8') as fd:
        fd.write(content)

    return 0


def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('manifest')
    parser.add_argument('output')
    parser.add_argument('--override', action='store_true')
    return parser.parse_args()


if __name__ == '__main__':
    args = get_args()
    sys.exit(main(args.manifest, args.output, args.override))
