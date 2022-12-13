#!/usr/bin/python3

import os
import json
import sys

if __name__ == '__main__':

    # Check arguments
    if len(sys.argv) <= 2:
        print("Usage: gen.py <output_fn> <static_lib_fn>")
        sys.exit(1)

    # Unpack arguments
    output_fn = sys.argv[1]
    static_lib_fn = sys.argv[2]

    dumped_strings_fn = f'{output_fn}.txt'

    # Dump unresolved symbols from static library
    os.system(
        f'nm -u {static_lib_fn} | c++filt | grep eprofiler | grep ::to_id > {dumped_strings_fn}')

    strings = {}

    # Parse the dumped symbols
    with open(dumped_strings_fn, 'r') as f:
        for i, line_raw in enumerate(f.readlines()):
            line = line_raw.strip().replace('(char)', '')

            if not line.startswith('eprofiler::internal::EProfiler<eprofiler::internal::EProfilerName<'):
                continue

            if not line.endswith('::to_id() const'):
                continue

            def list_ordstr_to_strin(x): return ''.join(
                [chr(int(a.strip())) for a in x if a.strip() != '' and a != 'char'])

            profiler_name = list_ordstr_to_strin(
                line.split('{')[2].split('}')[0].split(','))

            tag_type_template_params = line.split('StringConstant_WithID')[
                1].split('>')[0] + '>'
            tag_name = list_ordstr_to_strin(
                tag_type_template_params[:-1].split(',')[1:])
            tag_type = 'StringConstant_WithID' + tag_type_template_params

            profiler_type_base = '<'.join(line.split('<')[0:2])
            profiler_type = profiler_type_base + '{"' + profiler_name + '"}>'

            if profiler_type not in strings.keys():
                strings[profiler_type] = {}

            strings[profiler_type][tag_type] = {
                'profiler_type': profiler_type,
                'profiler_name': profiler_name,
                'tag_type': tag_type,
                'tag_name': tag_name,
            }

    # Generate the output C++ file
    with open(output_fn, 'w') as f:

        # Include cstint and eprofiler.hpp
        f.write('#include <cstdint>\n')
        f.write('#include <eprofiler/eprofiler.hpp>\n\n')

        for profiler_idx, profiler in enumerate(strings.keys()):
            tags = strings[profiler]

            # Explicitly instansiate the profiler class template
            f.write(f'template class {profiler};\n')

            for tag_idx, tag in enumerate(tags.keys()):

                # Get the tag info
                tag_info = tags[tag]

                # Assign tag id
                tag_info['id'] = tag_idx

                # Explicitly instansiate the tag class template
                f.write(f'template class {profiler}::{tag};\n')

                # Generate the to_id() function
                f.write(
                    f'\ntemplate<>\ntemplate<> std::size_t {profiler}::{tag}::to_id() const noexcept \n{{\n return {tag_idx}; \n}}\n\n')
            
            # Create string literal array of tags
            f.write(f'static constexpr std::string_view profiler_{profiler_idx}_tags[] = {{')
            for tag_idx, tag in enumerate(tags.keys()):
                f.write(f'"{tags[tag]["tag_name"]}"')
                if tag_idx != len(tags.keys()) - 1:
                    f.write(', ')
            f.write('};\n\n')

            f.write(f'template<>\nconst std::span<const std::string_view> {profiler}::tags(profiler_{profiler_idx}_tags,{len(tags.keys())});\n\n')


    # Generate JSON file containing the mapped strings
    with open(dumped_strings_fn.replace('.txt', '.json'), 'w') as f:
        f.write(json.dumps(strings))

    sys.exit(0)
