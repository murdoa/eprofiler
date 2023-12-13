from lark import Lark
from lark import Transformer
from lark import Tree

import os
import sys
from itertools import chain

# Hacked together parser for C++ symbols 
# This is not a complete parser only written to parse the symbols currently produced by eprofiler


def read_file(filename) -> str:
    with open(filename, 'r') as f:
        return f.read()

symbol_parser = Lark(r"""
    // Valid Type/Variable Names
    // begins with a letter, and can contain letters, digits, and underscores
    valid_name: LETTER (LETTER | DIGIT | "_")*


    // Core Types
    array_type: type WS "[" integer_literal "]"
    type: valid_name [template_argument_pack]
    any_type: (array_type | type) 

    scoped_type: ((any_type "::")*  any_type)

    // Template Arguments
    template_argument: (literal_value |  scoped_type )
    template_argument_pack: "<" (template_argument  "," WS )* template_argument ">"


    // Literals 
    integer_literal: SIGNED_NUMBER /(?:ull|ll|ul|l|u)/i?
    string_literal_suffix: [ "_" valid_name ]
    string_literal: ESCAPED_STRING string_literal_suffix
    initalizer_list: "{" (literal_value "," WS)* [literal_value] "}"

    cast_cstyle: "(" any_type ")"
    literal_value: [cast_cstyle] (integer_literal | string_literal | (scoped_type initalizer_list)) [WS]

    // CV Qualifiers
    cv_qualifier: /const/ | /volatile/
    cv_qualifiers: (cv_qualifier WS?)* cv_qualifier 

    // Function Signatures
    function_sig: /\(\)/

    // Static Members
    static_member: (any_type "::")+ valid_name function_sig? WS? [cv_qualifiers]

    %import common.LETTER
    %import common.DIGIT
    %import common.WORD
    %import common.ESCAPED_STRING
    %import common.SIGNED_NUMBER
    %import common.WS
    
    """, start='static_member', parser='lalr')


class CXXMember:
    TYPE_FUNC = 0
    TYPE_VAR = 1
    def __init__(self, name, type, cv_qualifiers):
        self.name = name
        self.type = type
        self.cv_qualifiers = cv_qualifiers

    def to_cpp_string(self):
        function_sig = ''
        if self.type == CXXMember.TYPE_FUNC:
            function_sig = '()'
        return f'{self.name}{function_sig} {" ".join(self.cv_qualifiers)}'

    def __repr__(self):
        return f'CXXMember: ({self.to_cpp_string()})'

class CXXType:
    def __init__(self, name, template_args=[]):
        self.name = name
        self.template_args = template_args
        self.parsed_child = None
        self.parsed_member = None

        if self.template_args is None:
            self.template_args = []

    def is_template(self):
        return len(self.template_args) != 0

    def to_cpp_string_template(self):
        template_args_str = ''
        if len(self.template_args) != 0:
            template_args_str = '<' + ', '.join([ x.to_cpp_string() for x in self.template_args]) + '>'
        return template_args_str

    def to_cpp_string(self):
        template_args_str = self.to_cpp_string_template()

        child_str = ''
        if self.parsed_child is not None:
            child_str = f'::{self.parsed_child.to_cpp_string()}'

        member_str = ''
        if self.parsed_member is not None:
            member_str = f'::{self.parsed_member.to_cpp_string()}'

        return f'{self.name}{template_args_str}{child_str}{member_str}'

    def __repr__(self):
        return f'CXXType: ({self.to_cpp_string()})'

class CXXArrType(CXXType):
    def __init__(self, name, template_args, size):
        super().__init__(name, template_args)
        self.size = size

    def __repr__(self):
        return f'CXXArrType: ({self.to_cpp_string()})'

class CXXInitalizerList:
    def __init__(self, values):
        self.values = list(values)

    def to_cpp_string(self):
        return '{' + ', '.join([x.to_cpp_string() for x in self.values]) + '}'

    def __repr__(self):
        return f'CXXInitalizerList: ({self.to_cpp_string()})'

class CXXLiteral:
    def __init__(self, cast, literal_type, literal_value, suffix=None):
        self.casts = []
        self.literal_type = literal_type
        self.literal_value = literal_value
        self.suffix = suffix

        if cast:
            self.casts.append(cast)

    def add_cast(self, cast):
        self.casts.append(cast)

    def to_cpp_string(self):
        suffix = self.suffix if self.suffix else ''

        casts = ''
        if len(self.casts) != 0:
            casts = '(' + ')('.join([ cast.to_cpp_string() for cast in self.casts ]) + ')'

        literal_type = ''

        literal_value = self.literal_value.to_cpp_string() if hasattr(self.literal_value, 'to_cpp_string') else self.literal_value

        if isinstance(self.literal_value,CXXInitalizerList):
            if not isinstance(self.literal_type, CXXArrType):
                literal_type = self.literal_type.to_cpp_string()

        return f'{casts}{literal_type}{literal_value}{suffix}'

    def __repr__(self):
        return f'CXXLiteral: ({self.to_cpp_string()})'



class SymbolsTransformer(Transformer):
    # Helper non parsing functions
    def remove_nones(self, items):
        return list(filter(lambda x: x, items))

    def chain_types(self, type_chain):
        main_type = type_chain[-1]
        for i in range(len(type_chain)-2, -1, -1):
            type_chain[i].parsed_child = main_type
            main_type = type_chain[i]
        return main_type

    # Handle basic joining of letters and digits
    def LETTER(self, items):
        return items[0]

    def valid_name_impl(self, items):
        return ''.join(self.remove_nones(items))

    def valid_name(self, items):
        name =  ''.join(self.remove_nones(items))
        return name

    def WORD(self, items):
        return str(items)

    # Map whitespace to None
    def WS(self, items):
        return None

    # Namespaces
    def namespace(self, items):
        return items[0]

    def namespaces(self, items):
        return items

    # C style casts
    def cast_cstyle(self, items):
        return items[0]

    # Literals
    def integer_literal(self, items):
        suffix = None
        if len(items) > 1:
            suffix = items[1].value
        return CXXLiteral(None, 'integer', int(items[0].value), suffix)

    def string_literal(self, items):
        string, suffix = items
        return CXXLiteral(None, 'string', string, suffix)

    def literal_value(self, items):
        if isinstance(items[1], CXXLiteral):
            if items[0]:
                items[1].add_cast(items[0])
            return items[1]

        if isinstance(items[-2], CXXInitalizerList):
            return CXXLiteral(*items)

        return items

    # Template arguments
    def template_argument(self, items):
        return self.remove_nones(items)[0]

    def template_argument_pack(self, items):
        return self.remove_nones(items)

    def type(self, items): # items = [ name, template_args]
        return CXXType(*items)

    def array_type(self,items): 
        return CXXArrType(items[0].name, [], items[2])

    def any_type(self, items): 
        return items[0]

    # Initializer list
    def initalizer_list(self, items): 
        return CXXInitalizerList(filter(lambda x: x, items))

    # Function Signatures
    def function_sig(self, items):
        return self.remove_nones(items)

    def scoped_type(self, items):
        return self.chain_types(items)

    # Static Members
    def static_member(self, items): # items = [type, name, is_function, WS, cv_qualifiers]
        main_type = self.chain_types(items[:len(items)-4])
        member_type = CXXMember.TYPE_VAR
        if items[-3][0].value == '()':
            member_type = CXXMember.TYPE_FUNC
        main_type.parsed_member = CXXMember(items[-4], member_type, items[-1])
        return main_type

    # CV Qualifiers 
    def cv_qualifier(self, items): 
        return items[0].value

    def cv_qualifiers(self, items):
        return self.remove_nones(items)


class TypeTree:
    def __init__(self):
        self.types = {}
        self.template_instances = set()

    def register_type(self, parsed_type, root=None):
        if root is None:
            root = self

        if isinstance(parsed_type, CXXLiteral):
            if isinstance(parsed_type.literal_type, CXXType):
                root.register_type(parsed_type.literal_type, root=root)
            return

        type_entry = (parsed_type.name, None)
        if parsed_type.is_template():
            type_entry = (parsed_type.name, parsed_type.to_cpp_string_template())

        if type_entry not in self.types.keys():
            self.types[type_entry] = TypeTree()

        for type in parsed_type.template_args:
            root.register_type(type, root=root)

        if parsed_type.parsed_child is not None:
            self.types[type_entry].register_type(parsed_type.parsed_child, root=root)


def print_type_tree(type_tree, parent=''):
    if type_tree is None:
        return

    keys_sorted = sorted(type_tree.types.keys(), key=lambda key: len(type_tree.types[key].types))

    non_templates = filter(lambda key: key[1] is None, keys_sorted)
    templates = filter(lambda key: key[1] is not None, keys_sorted)

    keys_sorted = chain(non_templates, templates)

    for type_name in keys_sorted:
        type_data = type_tree.types[type_name]

        name = parent
        if type_name[0] is not None:
            name += type_name[0]
        if type_name[1] is not None:
            name += type_name[1]
        name
        print(name)
        print_type_tree(type_data, parent=name + '::')

if __name__ == "__main__":
    # Check arguments
    if len(sys.argv) <= 2:
        print("Usage: gen.py <output_fn> <static_lib_fn>")
        sys.exit(1)

    # Unpack arguments
    output_fn = sys.argv[1]
    static_lib_fn = sys.argv[2]

    dumped_strings_fn = f'{output_fn}.txt'

    # Dump unresolved symbols from static library
    os.system(f'nm -u {static_lib_fn} | c++filt | grep eprofiler | grep ::to_id > {dumped_strings_fn}')

    hashtables = {}

    root_node = { }


    type_tree = TypeTree()

    # Parse the dumped symbols
    with open(dumped_strings_fn, 'r') as f:
        for line in f.readlines():
            line_start_idx = line.find('U ')
            if line_start_idx == -1 or len(line) - line_start_idx < 3 :
                raise Exception(f'Invalid symbol line: {line}')

            line = line[line_start_idx+2:].strip()

            symbol = symbol_parser.parse(line)

            parsed_symbol = SymbolsTransformer().transform(symbol)

            type_tree.register_type(parsed_symbol)

    print_type_tree(type_tree)

    sys.exit(1)
