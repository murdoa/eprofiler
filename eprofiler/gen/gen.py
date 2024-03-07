import argparse
import copy
import hashlib
import json
import os
import sys
from itertools import chain
from typing import Union

from lark import Lark, Transformer, Tree

# Hacked together parser for C++ symbols 
# This is not a complete parser only written to parse the symbols currently produced by eprofiler

symbol_parser = Lark(r"""
    // Valid Type/Variable Names
    // begins with a letter, and can contain letters, digits, and underscores
    // Exclamation mark is used to prevent filtering of underscore terminal
    !valid_name: (LETTER | "_") (LETTER | DIGIT | "_")*
                     
    // Terminal type for fundamental integers 
    FUNDAMENTAL_INT: [ ("unsigned" | "signed") WS] ( "char" | "short" | "int" | "long" | "long" WS "int" | "long" WS "long" WS "int" )
    OTHER_FUNDAMENTAL_WITH_WS: "long double"

    // Core Types
    array_type: (FUNDAMENTAL_INT | type) WS "[" integer_literal "]"
    type: valid_name [template_argument_pack]
    // All other types that aren't simple names or scoped
    complex_type: array_type | FUNDAMENTAL_INT
    scoped_type: ((type "::")*  type)

    // Template Arguments
    template_argument: literal_value |  scoped_type | complex_type
    template_argument_pack: "<" [WS] (template_argument  [WS] "," [WS])* template_argument [WS] ">"

    // Literals 
    integer_literal: SIGNED_NUMBER /(?:ull|ll|ul|l|u)/i?
    string_literal_suffix: [ "_" valid_name ]
    string_literal: ESCAPED_STRING string_literal_suffix
    initializer_list: "{" (literal_value "," WS)* [literal_value] "}"

    cast_cstyle: "(" (complex_type | scoped_type ) ")"
    literal_value: [cast_cstyle] (integer_literal | string_literal | (scoped_type initializer_list) | (array_type initializer_list)) [WS]

    // CV Qualifiers
    cv_qualifier: /const/ | /volatile/
    cv_qualifiers: (cv_qualifier WS?)* cv_qualifier 

    // Function Signatures
    function_sig: /\(\)/

    // Static Members
    static_member: (type "::")+ valid_name [function_sig] [WS] [cv_qualifiers]

    %import common.LETTER
    %import common.DIGIT
    %import common.WORD
    %import common.ESCAPED_STRING
    %import common.SIGNED_NUMBER
    %import common.WS       
    """, start='static_member', parser='lalr', maybe_placeholders=True)


class CXXMember:
    """
    Represents C++ member functions and variables.
    """

    TYPE_FUNC = 0
    TYPE_VAR = 1

    def __init__(self, name : str, mem_type : int, cv_qualifiers : list):
        """
        Parameters
            name : str -> Name of the member
            mem_type : int -> Type of the member (CXXMember.TYPE_FUNC, CXXMember.TYPE_VAR)
            cv_qualifiers : list -> CV qualifiers as strings
        """
        self.name = name
        self.type = mem_type
        self.cv_qualifiers = cv_qualifiers

    def to_cpp_string(self) -> str:
        """
        Returns
            str -> C++ string representation of the member
        """
        function_sig = ''
        if self.type == CXXMember.TYPE_FUNC:
            function_sig = '()'
        return f'{self.name}{function_sig} {" ".join(self.cv_qualifiers)}'

    def __repr__(self) -> str:
        return f'CXXMember: ({self.to_cpp_string()})'

class CXXType:
    """
    Represents C++ types and namespaces.
    """

    def __init__(self, name : str, template_args : list =[]):
        """
        Parameters
            name : str -> Name of the type
            template_args : list -> List of template arguments
        """
        self.name = name
        self.template_args = template_args
        self.parsed_child = None
        self.parsed_member = None

        if self.template_args is None:
            self.template_args = []

    def is_template(self) -> bool:
        """
        Returns
            bool -> True if the type is a template
        """
        return len(self.template_args) != 0

    def to_cpp_string_template(self) -> str:
        """
        Returns
            str -> C++ string representation of the template arguments
        """
        template_args_str = ''
        if len(self.template_args) != 0:
            template_args_str = '<' + ', '.join([ x.to_cpp_string() for x in self.template_args]) + '>'
        return template_args_str

    def to_cpp_string(self) -> str:
        """
        Returns
            str -> C++ string representation of the type
        """
        template_args_str = self.to_cpp_string_template()

        child_str = ''
        if self.parsed_child is not None:
            # If the child is a template type, we need to add the template keyword
            if type(self.parsed_child) == CXXType and self.parsed_child.is_template():
                child_str = f'::template {self.parsed_child.to_cpp_string()}'
            else:
                child_str = f'::{self.parsed_child.to_cpp_string()}'

        member_str = ''
        if self.parsed_member is not None:
            member_str = f'::{self.parsed_member.to_cpp_string()}'

        return f'{self.name}{template_args_str}{child_str}{member_str}'

    def __repr__(self):
        return f'CXXType: ({self.to_cpp_string()})'

class CXXArrType(CXXType):
    """
    Represents C++ array types.
    """

    def __init__(self, name, template_args, size):
        """
        Parameters
            name : str -> Name of the type
            template_args : list -> List of template arguments
            size : int -> Size of the array
        """
        super().__init__(name, template_args)
        self.size = size

    def __repr__(self) -> str:
        return f'CXXArrType: ({self.to_cpp_string()})'

class CXXInitializerList:
    """
    Represents C++ initializer lists.
    """

    def __init__(self, values):
        """
        Parameters
            values : list -> List of values in the initializer list
        """
        self.values = list(values)

    def to_cpp_string(self) -> str:
        """
        Returns
            str -> C++ string representation of the initializer list
        """
        return '{' + ', '.join([x.to_cpp_string() for x in self.values]) + '}'

    def __repr__(self) -> str:
        return f'CXXInitializerList: ({self.to_cpp_string()})'

class CXXLiteral:
    """
    Represents C++ literals, describes the type and value of the literal.
    """

    def __init__(self, cast : CXXType, literal_type : CXXType, literal_value, suffix=None):
        """
        Parameters
            cast : CXXType -> Cast type
            literal_type : CXXType -> Type of the literal
            literal_value -> Value of the literal
            suffix : str -> Suffix of the literal
        """

        self.casts = []
        self.literal_type = literal_type
        self.literal_value = literal_value
        self.suffix = suffix

        if cast:
            self.casts.append(cast)

    def add_cast(self, cast : CXXType):
        """
        Add a cast to the literal.
        Parameters
            cast : CXXType -> Cast type
        """
        self.casts.append(cast)

    def to_cpp_string(self) -> str:
        """
        Returns
            str -> C++ string representation of the literal
        """
        suffix = self.suffix if self.suffix else ''

        casts = ''
        if len(self.casts) != 0:
            casts = '(' + ')('.join([ cast.to_cpp_string() for cast in self.casts ]) + ')'

        literal_type = ''

        literal_value = self.literal_value.to_cpp_string() if hasattr(self.literal_value, 'to_cpp_string') else self.literal_value

        if isinstance(self.literal_value,CXXInitializerList):
            if not isinstance(self.literal_type, CXXArrType):
                literal_type = self.literal_type.to_cpp_string()

        return f'{casts}{literal_type}{literal_value}{suffix}'

    def __repr__(self) -> str:
        return f'CXXLiteral: ({self.to_cpp_string()})'



class SymbolsTransformer(Transformer):
    """
    Lark transformer for parsing C++ symbols.
    """

    # Helper non parsing functions

    def remove_nones(self, items) -> list:
        """
        Removes None values from a list.

        Parameters
            items : list -> List of items
        Returns
            list -> List of items with None values removed
        """
        return list(filter(lambda x: x, items))

    def chain_types(self, type_chain) -> CXXType:
        """
        Chains a list of types together.

        Parameters
            type_chain : list -> List of types
        Returns
            CXXType -> The main type of the chain
        """
        if len(type_chain) == 0:
            return None

        if len(type_chain) == 1:
            return type_chain[0]

        main_type = type_chain[-1]
        for i in range(len(type_chain)-2, -1, -1):
            type_chain[i].parsed_child = main_type
            main_type = type_chain[i]
        return main_type

    def LETTER(self, items : list) -> str:
        """
        Transforms LETTER terminal to a string.

        Parameters
            items : list -> Parsed terminal list containing LETTER 
        Returns
            str -> The parsed letter
        """
        return str(items[0])

    def valid_name(self, items : list) -> str:
        """
        Transforms of valid_name symbol to string.

        Parameters
            items : list -> Parsed symbol list containing LETTER (LETTER | DIGIT | "_")* 
        Returns
            str -> The parsed name
        """
        name =  ''.join(self.remove_nones(items))
        return name

    def WORD(self, items : list) -> str:
        """
        Transforms WORD terminal to a string.

        Parameters
            items : list -> Parsed terminal list containing WORD
        Returns
            str -> The parsed word
        """
        return str(items)

    def WS(self, items : list) -> None:
        """
        Transforms whitespace (maps to None).

        Parameters
            items : list -> Parsed whitespace
        Returns
            None
        """
        return None

    def namespace(self, items : list) -> str:
        """
        Transforms namespace symbol to string.

        Parameters
            items : list -> Parsed symbol list containing valid_name
        Returns
            str -> The parsed namespace
        """
        return items[0]

    def namespaces(self, items: list) -> list:
        """
        Transforms namespaces symbol to a list of namespaces.

        Parameters
            items : list -> Parsed list of namespaces
        Returns
            list -> List of namespaces
        """
        return items

    def cast_cstyle(self, items: list) -> CXXType:
        """
        Transforms C-style casts to CXXType.

        Parameters
            items : list -> Parsed list of types
        Returns
            CXXType -> The parsed type
        """
        return items[0]

    # Literals
    def integer_literal(self, items : list) -> CXXLiteral:
        """
        Transforms integer literals to CXXLiteral.

        Parameters
            items : list -> Parsed list of integer literals
        Returns
            CXXLiteral -> The parsed literal
        """

        suffix = None
        if len(items) > 1:
            suffix = items[1].value
        return CXXLiteral(None, 'integer', int(items[0].value), suffix)

    def string_literal(self, items : list) -> CXXLiteral:
        """
        Transforms string literals to CXXLiteral.

        Parameters
            items : list -> Parsed list of string literals
        Returns
            CXXLiteral -> The parsed literal
        """
        string, suffix = items
        return CXXLiteral(None, 'string', string, suffix)

    def literal_value(self, items: list) -> CXXLiteral:
        """
        Transforms literal values to CXXLiteral.

        Parameters
            items : list -> Parsed list of literal values
        Returns
            CXXLiteral -> The parsed literal
        """

        if isinstance(items[1], CXXLiteral):
            if items[0]:
                items[1].add_cast(items[0])
            return items[1]

        if isinstance(items[-2], CXXInitializerList):
            return CXXLiteral(*items)

        raise Exception('Unhandled literal value type')

    def template_argument(self, items : list) -> Union[CXXType,  CXXLiteral]:
        """
        Transforms template arguments to CXXType.

        Parameters
            items : list -> Parsed list of template arguments
        Returns
            CXXType or CXXLiteral -> The parsed type or literal
        """
        return self.remove_nones(items)[0]

    def template_argument_pack(self, items : list) -> list:
        """
        Transforms template argument packs to a list of template arguments.

        Parameters
            items : list -> List of tokens with following grammar "<" [WS] (template_argument  [WS] "," [WS])* template_argument [WS] ">"
        Returns
            list -> List of template arguments
        """
        return self.remove_nones(items)

    def type(self, items : list) -> CXXType: 
        """
        Transforms type to CXXType.

        Parameters
            items : list -> [ name, template_args ]
        Returns
            CXXType -> The parsed type
        """
        return CXXType(*items)

    def array_type(self, items : list) -> CXXArrType: 
        """
        Transforms array type to CXXArrType.

        Parameters
            items : list -> [ type, None, size, None ]
        Returns
            CXXArrType -> The parsed array type
        """
        return CXXArrType(items[0].name, [], items[2])

    def FUNDAMENTAL_INT(self, int_type : str) -> CXXType:
        """
        Transforms fundamental integers to CXXType.

        Parameters
            int_type : str -> Parsed fundamental integer type
        Returns
            CXXType -> CXXType representing the fundamental integer type 
        """
        return CXXType(int_type)
    
    def OTHER_FUNDAMENTAL_WITH_WS(self, fundamental_type : str) -> CXXType:
        """
        Transforms other fundamental types to CXXType.

        Parameters
            fundamental_type : str -> Parsed fundamental type
        Returns
            CXXType -> CXXType representing the fundamental type
        """

        return CXXType(fundamental_type)

    def complex_type(self, items: list) -> CXXType: 
        """
        Transforms any_type to CXXType.

        Parameters
            items : list -> [ type | array_type ]
        Returns
            CXXType -> The parsed type
        """
        return items[0]

    # Initializer list
    def initializer_list(self, items : list) -> CXXInitializerList: 
        """
        Transforms initializer list to CXXInitializerList.

        Parameters
            items : list -> List of literal values including None terminals
        Returns
            CXXInitializerList -> The parsed initializer list
        """
        return CXXInitializerList(filter(lambda x: x, items))

    # Function Signatures
    def function_sig(self, items : list) -> list:
        """
        Transforms function signatures to a list of function parameters.
        Only handles no parameter functions for now.

        Parameters
            items : list -> Parsed list of function parameters including None terminals 
        Returns
            list -> List of function signatures
        """
        return self.remove_nones(items)

    def scoped_type(self, items: list) -> CXXType:
        """
        Transforms scoped types to CXXType.

        Parameters
            items : list -> Parsed list of namespaces with a type at the end

        Returns
            CXXType -> The parsed type
        """
        return self.chain_types(items)

    def static_member(self, items : list) -> CXXType:
        """
        Transforms static members to CXXType.

        Parameters
            items : list -> Parsed list containing [type, name, is_function, WS, cv_qualifiers]
        Returns
            CXXType -> The parsed type
        """

        # (any_type "::")+ valid_name [function_sig] [WS] [cv_qualifiers]

        cxx_type = self.chain_types(items[:-4])
        member_name = items[-4]
        function_sig = items[-3]
        cv_qualifiers = items[-1]

        if not cv_qualifiers:
            cv_qualifiers = []
        
        member_type = CXXMember.TYPE_VAR
        if function_sig:
            member_type = CXXMember.TYPE_FUNC

        cxx_type.parsed_member = CXXMember(member_name, member_type, cv_qualifiers)

        return cxx_type

    def cv_qualifier(self, items : list) -> str: 
        """
        Transforms cv_qualifier token to a string.

        Parameters
            items : list -> Parsed list containing cv_qualifier
        Returns
            str -> The parsed cv_qualifier
        """
        return items[0].value

    def cv_qualifiers(self, items : list) -> list:
        """
        Transforms cv_qualifiers token to a list of cv_qualifiers.

        Parameters
            items : list -> Parsed list of cv_qualifiers including None terminals
        Returns
            list -> List of cv_qualifiers
        """
        return self.remove_nones(items)

if __name__ == "__main__":
    # Setup argument parser
    parser = argparse.ArgumentParser(
                    prog='gen.py',
                    description='Generates a C++ file with the unresolved symbols from a static library mapping to unique ids.'
    )

    parser.add_argument('output_fn', type=str, help='Output file name')
    parser.add_argument('static_lib_fn', type=str, help='Static library file name')

    # Parse and unpack arguments
    args = parser.parse_args()
    output_fn = args.output_fn
    static_lib_fn = args.static_lib_fn

    print(f'Generating file: {output_fn} from static library: {static_lib_fn}')

    # Validate file path
    if not os.path.exists(static_lib_fn):
        print(f'Error: {static_lib_fn} does not exist')
        sys.exit(1)


    # Dump unresolved symbols from static library
    # Hacky way to get the unresolved symbols from the static library better methods exist
    # nm -u <static_lib_fn> dumps the unresolved symbols from the static library
    # c++filt demangles the symbols providing consistent names across platforms/abis
    # grep eprofiler to filter to only eprofiler symbols
    # TODO: move to use subprocess with piped stdout and stderr and detect failure
    dumped_strings_fn = output_fn.replace(".cpp", ".unresolved.txt")
    os.system(f'nm -u {static_lib_fn} | c++filt | grep eprofiler > {dumped_strings_fn}')

    # Dictionary to store the registered profilers and their tags
    registered_profilers = {}


    # Parse the dumped symbols
    with open(dumped_strings_fn, 'r') as f:
        for line in f.readlines():
            # Remove the 'U ' prefix from the line
            line_start_idx = line.find('U ')
            if line_start_idx == -1 or len(line) - line_start_idx < 3 :
                raise Exception(f'Invalid symbol line: {line}')
            line = line[line_start_idx+2:].strip()

            # Parse and transform line using Lark parser and transformer
            symbol = symbol_parser.parse(line)
            parsed_symbol = SymbolsTransformer().transform(symbol)

            # Extract the profiler name and tag name from the parsed symbol
            profiler_name_literal = parsed_symbol.parsed_child.template_args[0].parsed_child.template_args[0]
            profiler_name_char_init_list = profiler_name_literal.literal_value.values[0].literal_value.values
            profiler_name = ''.join([ chr(x.literal_value) for x in profiler_name_char_init_list]) # 
            # Replace the char array with a string literal
            parsed_symbol.parsed_child.template_args[0].parsed_child.template_args[0] = CXXLiteral(None, 'string', '"' + profiler_name + '"')

            # Extract return and value type from eprofiler template args
            keytype = parsed_symbol.parsed_child.template_args[1].to_cpp_string()
            valuetype = parsed_symbol.parsed_child.template_args[2].to_cpp_string()

            # Register the profiler if first time seen
            if profiler_name not in registered_profilers:
                registered_profilers[profiler_name] = {}

                hashtable_type = copy.deepcopy(parsed_symbol)
                hashtable_type.parsed_member = None
                hashtable_type.parsed_child.parsed_child = None

                registered_profilers[profiler_name]['tags'] = {}
                registered_profilers[profiler_name]['hashtable_type'] = hashtable_type
                registered_profilers[profiler_name]['key_type'] = keytype
                registered_profilers[profiler_name]['value_type'] = valuetype

                # Generate UUID
                sha256 = hashlib.new('sha256')
                sha256.update(profiler_name.encode('utf-8'))
                uuid = sha256.hexdigest()
                registered_profilers[profiler_name]['uuid'] = f'{uuid}'

                # Whether to generate value store or offset
                registered_profilers[profiler_name]['gen_value_store'] = False
            
            
            # check if parsed symbol is value_store
            if parsed_symbol.parsed_member.name == 'value_store':
                registered_profilers[profiler_name]['gen_value_store'] = True
            elif parsed_symbol.parsed_member.name == 'offset':
                pass
            elif parsed_symbol.parsed_member.name == 'to_id':
                tag_name = ''.join([ chr(x.literal_value) for x in parsed_symbol.parsed_child.parsed_child.template_args[1:]]) 
                registered_profilers[profiler_name]['tags'][tag_name] = {
                    'parsed_symbol': parsed_symbol,
                }
            else:
                raise Exception(f'Unhandled symbol: {parsed_symbol.parsed_member.name}')

    # Attach "hashes" to the tags
    count = 1
    for profiler_name, profiler_data in registered_profilers.items():
        profiler_data['offset'] = count
        for tag_name, tag_data in profiler_data['tags'].items():
            tag_data['hash'] = count
            count += 1

    hash_info = { profiler_name: { tag_name: tag_data['hash'] for tag_name, tag_data in profiler_data['tags'].items()} for profiler_name, profiler_data in registered_profilers.items()  }

    json_fn = output_fn.replace('.cpp','.json')
    with open(f'{json_fn}', 'w') as outf:
        outf.write(json.dumps(hash_info, indent=4))

    print(hash_info)

    with open(output_fn, 'w') as outf:
        outf.write('#include <array>\n#include <chrono>\n#include <eprofiler/eprofiler.hpp>\n')

        for profiler_name, profiler_data in registered_profilers.items():

            for tag_name, tag_data in profiler_data['tags'].items():
                func_sig = f"template<>\ntemplate<>\n{profiler_data['key_type']} {tag_data['parsed_symbol'].to_cpp_string()} noexcept"

                outf.write(func_sig)
                outf.write('{\n')
                outf.write(f'    return {tag_data["hash"]};\n')
                outf.write('}\n\n') 

            outf.write(f'template<>\nconst {profiler_data["key_type"]} {profiler_data["hashtable_type"].to_cpp_string()}::offset = {profiler_data["offset"]};\n')

            if profiler_data['gen_value_store']:
                outf.write(f'std::array<{profiler_data["value_type"]}, {len(profiler_data["tags"])}> eprofiler_{profiler_data["uuid"]}_value_store = {{}};')
                outf.write(f'template<>\nconst std::span<{profiler_data["value_type"]}> {profiler_data["hashtable_type"].to_cpp_string()}::value_store = std::span{{ eprofiler_{profiler_data["uuid"]}_value_store }};\n')

    sys.exit(0)
