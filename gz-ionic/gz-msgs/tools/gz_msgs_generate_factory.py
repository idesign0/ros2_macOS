#!/usr/bin/env python3
#
# Copyright (C) 2023 Open Source Robotics Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License")
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import os
import pathlib
import sys

# Create <gz/msgs/MessageTypes.hh>
cc_header = """/*
 * Copyright (C) 2023 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

/* This file was automatically generated.
 * Do not edit this directly
 */

#ifndef GZ_MSGS_MESSAGE_TYPES_HH_
#define GZ_MSGS_MESSAGE_TYPES_HH_
{gz_msgs_headers}

namespace {namespace} {{
int RegisterAll();
}}
#endif"""

# Create factory registration bits
cc_source = """/*
 * Copyright (C) 2023 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

/* This file was automatically generated.
 * Do not edit this directly
 */

#include "gz/msgs/Factory.hh"
#include "gz/msgs/MessageFactory.hh"
#include "{package_path}/MessageTypes.hh"

#include <array>

namespace {{
    using NamedFactoryFn = std::pair<std::string, gz::msgs::MessageFactory::FactoryFn>;

    std::array<NamedFactoryFn, {nRegistrations}> kFactoryFunctions = {{{{
{registrations}
}}}};
}}  // namespace
"""

cc_factory = """
namespace {namespace} {{
int RegisterAll() {{
  size_t registered = 0;
  for (const auto &entry: kFactoryFunctions) {{
    gz::msgs::Factory::Register(entry.first, entry.second);
    registered++;
  }}
  return registered;
}}

static int kMessagesRegistered = RegisterAll();
}}  // namespace {namespace}
"""

register_fn = """  {{"{package_str}.{message_str}",
    []()->std::unique_ptr<google::protobuf::Message>{{return std::make_unique<{message_cpp_type}>();}}}},"""

def main(argv=sys.argv[1:]):
    parser = argparse.ArgumentParser(
        description='Generate protobuf factory file',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        '--output-cpp-path',
        required=True,
        help='The basepath of the generated C++ files')
    parser.add_argument(
        '--proto-package',
        required=True,
        help='The basepath of the generated C++ files')
    parser.add_argument(
        '--proto-path',
        required=True,
        help='The location of the protos')
    parser.add_argument(
        '--protos',
        type=str,
        nargs='*',
        required=True,
        help='The list of protos to include'
    )

    args = parser.parse_args(argv)

    headers = []
    registrations = []

    package = [p for p in args.proto_package.split('.') if len(p)]
    namespace = '::'.join(package)
    package_str = '.'.join(package)
    package_path = '/'.join(package)

    for proto in args.protos:
        proto_file = os.path.splitext(os.path.relpath(proto, args.proto_path))[0]
        header = proto_file + ".pb.h"
        headers.append(f"#include <{header}>")

        proto_file = '_'.join(pathlib.Path(proto_file).parts)

        # The gazebo extensions to the gazebo compiler write out a series of index files
        # which capture the message types
        index = os.path.join(args.output_cpp_path, proto_file + ".pb_index")
        with open(index, "r") as index_f:
            for line in index_f.readlines():
                line = line.strip()

                message_str = line
                message_cpp_type = '::'.join(package) + '::' + message_str

                registrations.append(register_fn.format(
                    package_str=package_str,
                    message_str=message_str,
                    message_cpp_type=message_cpp_type))

    with open(os.path.join(args.output_cpp_path, *package, 'MessageTypes.hh'), 'w') as f:
        f.write(cc_header.format(gz_msgs_headers='\n'.join(headers), namespace=namespace))

    with open(os.path.join(args.output_cpp_path, *package, 'register.cc'), 'w') as f:
        f.write((cc_source.format(registrations='\n'.join(registrations),
                                  nRegistrations=len(registrations),
                                  namespace=namespace,
                                  package_path=package_path) +
                 cc_factory.format(namespace=namespace)))

if __name__ == '__main__':
    sys.exit(main())
