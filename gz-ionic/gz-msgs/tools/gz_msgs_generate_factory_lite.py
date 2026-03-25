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
import re
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
#include "{include_path}/MessageTypes.hh"

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
        '--cc-output',
        required=True,
        help='The path to the generated cpp file')
    parser.add_argument(
        '--hh-output',
        required=True,
        help='The path to the generated hh file')
    parser.add_argument(
        '--proto-path',
        required=True,
        help='The location of the protos')
    parser.add_argument(
        '--proto-include-path',
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

    package_re = re.compile('^package (.*);$')
    message_re = re.compile(r'message (\w*)\s?{?$')

    registrations = dict()
    gz_msgs_headers = []
    package = []
    messages = []

    for proto in args.protos:
        try:
            with open(proto, 'r') as f:
                content = f.readlines()
                for line in content:
                    package_found = package_re.match(line)
                    if package_found:
                        package = package_found.group(1).split('.')

                    message_found = message_re.match(line)
                    if message_found:
                        messages.append(message_found.group(1))
        except:
            pass

        if package and messages:
            for message in messages:
                registrations['_'.join([*package, message])] = register_fn.format(
                    package_str='.'.join(package),
                    message_str=message,
                    message_cpp_type='::'.join([*package, message])
                )

            split = proto.replace(args.proto_include_path, '')
            split = [s for s in split.split("/") if s]
            split[-1] = split[-1].replace(".proto", ".pb.h")
            gz_msgs_headers.append("#include <" + "/".join(split) + ">")

    namespace = '::'.join(package)
    include_path = '/'.join(package)

    with open(os.path.join(args.cc_output), 'w') as f:
        f.write((cc_source.format(registrations='\n'.join(registrations.values()),
                                  nRegistrations=len(registrations.values()),
                                  namespace=namespace,
                                  include_path=include_path) +
                 cc_factory.format(namespace=namespace)))

    with open(os.path.join(args.hh_output), 'w') as f:
        f.write(cc_header.format(namespace=namespace,
                                 gz_msgs_headers='\n'.join(gz_msgs_headers)))

if __name__ == '__main__':
    sys.exit(main())
