#!/usr/bin/env python3
# Copyright (c) 2024-2026 Carologistics
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
from datetime import datetime
import re

from ament_index_python.packages import get_package_share_directory
from jinja2 import Environment, FileSystemLoader, Template
import rosidl_parser.definition
from rosidl_runtime_py.utilities import get_action, get_message, get_service


def to_camel_case(s):
    words = s.split('_')  # Split the string into words based on underscores
    camel_case_words = [word.capitalize() for word in words[0:]]
    camel_case_string = ''.join(camel_case_words)
    return camel_case_string


def to_upper_case(s):
    words = s.split('_')  # Split the string into words based on underscores
    upper_case_words = [word.upper() for word in words]
    upper_case_words = ''.join(upper_case_words)
    return upper_case_words


def to_snake_case(name):
    name_snake = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1_\2', name)

    name_snake = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', name_snake).lower()
    return name_snake


def to_kebab_case(name):
    name_kebab = re.sub(r'([A-Z]+)([A-Z][a-z])', r'\1-\2', name)
    name_kebab = re.sub(r'([a-z0-9])([A-Z])', r'\1-\2', name_kebab).lower()
    return name_kebab


# Dictionary for mapping ROS2 types to CLIPS types
clips_types = {
    'bool': 'BOOLEAN',
    'boolean': 'BOOLEAN',
    'byte': 'INTEGER',
    'char': 'STRING',
    'float32': 'FLOAT',
    'float64': 'FLOAT',
    'double': 'FLOAT',
    'int8': 'INTEGER',
    'uint8': 'INTEGER',
    'int16': 'INTEGER',
    'uint16': 'INTEGER',
    'int32': 'INTEGER',
    'uint32': 'INTEGER',
    'int64': 'INTEGER',
    'uint64': 'INTEGER',
    'string': 'STRING',
    'wstring': 'STRING',
}

# Dictionary for mapping ROS2 types to C++ types
cpp_types = {
    'bool': 'bool',
    'boolean': 'bool',
    'byte': 'uint8_t',
    'char': 'char',
    'float32': 'float',
    'float64': 'double',
    'int8': 'int8_t',
    'uint8': 'uint8_t',
    'int16': 'int16_t',
    'uint16': 'uint16_t',
    'int32': 'int32_t',
    'uint32': 'uint32_t',
    'int64': 'int64_t',
    'uint64': 'uint64_t',
    'string': 'std::string',
    'wstring': 'std::u16string',
}


# Helper function to convert ROS 2 types into readable strings
def get_field_type_str(field_type):
    if isinstance(field_type, rosidl_parser.definition.AbstractString):
        return 'string'
    elif isinstance(field_type, rosidl_parser.definition.BasicType):
        return field_type.typename
    elif isinstance(field_type, rosidl_parser.definition.AbstractSequence):
        return f'{get_field_type_str(field_type.value_type)}'
    # Check if the field is a namespaced type
    # (i.e., a custom message type like geometry_msgs/Twist)
    elif isinstance(field_type, rosidl_parser.definition.NamespacedType):
        # Join the namespace and the type name (e.g., geometry_msgs/Twist)
        full_name = '::'.join(field_type.namespaces + [field_type.name])
        return full_name

    # Fallback for any other unexpected type
    else:
        print('Unexpected field type: ', field_type)
        return str(field_type)


# Function to extract fields from a message class and convert to readable format
def extract_fields(message_type):
    extracted_fields = {}
    for field_name, field_type in zip(message_type.__slots__, message_type.SLOT_TYPES):
        # Remove leading underscore from field_name
        clean_field_name = field_name[1:] if field_name.startswith('_') else field_name
        str_type = get_field_type_str(field_type)
        if str_type in clips_types:
            clips_type = clips_types[str_type]
        else:
            clips_type = 'EXTERNAL-ADDRESS'
        if str_type in cpp_types:
            cpp_type = cpp_types[str_type]
        else:
            cpp_type = 'void *'

        extracted_fields[clean_field_name] = {
            'name': clean_field_name,
            'type': str_type,
            'clips_type': clips_type,
            'cpp_type': cpp_type,
            'array': isinstance(
                field_type, rosidl_parser.definition.AbstractSequence
            ),  # Detect if it's an array
            'restrictions': None,  # Customize for specific restrictions if necessary
            'lengths': None,  # Customize for specific length if necessary
        }
    return extracted_fields


def main():
    # Set up argument parsing
    parser = argparse.ArgumentParser(
        description='Generate CLIPS bindings for ROS 2 messages, services, \
        or actions for usage with the CLIPS Executive.'
    )

    # Define arguments
    parser.add_argument(
        'type',
        choices=['msg', 'srv', 'action'],
        help='Type of ROS 2 entity to generate (message, service, action).',
    )
    parser.add_argument(
        'package',
        type=str,
        help='Name of the package containing the message/service/action.',
    )
    parser.add_argument(
        'name',
        type=str,
        help='Name of the message/service/action to generate bindings for.',
    )

    # Parse the arguments
    args = parser.parse_args()

    # Simulate generating the bindings
    print(f'Generating {args.type} bindings for {args.package}/{args.type}/{args.name}')
    fields = {}
    response_fields = {}

    package_dir = get_package_share_directory('cx_ros_comm_gen')
    time_string = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    short_class_name = to_camel_case(args.package) + args.name
    class_name = 'CX' + short_class_name + 'Plugin'
    file_name = 'cx_' + to_snake_case(args.package) + '_' + to_snake_case(args.name) + '_plugin'
    include_header = (
        '<' + args.package + '/' + args.type + '/' + to_snake_case(args.name) + '.hpp>'
    )
    cpp_msg_type = args.package + '::' + args.type + '::' + args.name
    msg_type_str = args.package + '/' + args.type + '/' + args.name
    env = Environment(loader=FileSystemLoader(package_dir + '/templates'))
    env.filters['camel_case'] = to_camel_case
    env.filters['kebab_case'] = to_kebab_case
    env.filters['snake_case'] = to_snake_case

    if args.type == 'srv':
        try:
            srv_type = get_service(msg_type_str)

            msg_type_request = srv_type.Request
            msg_type_response = srv_type.Response
            fields = extract_fields(msg_type_request)
            response_fields = extract_fields(msg_type_response)
            with open(file_name + '.cpp', 'w') as source_file:  # noqa: E501
                tmpl = env.get_template(args.type + '.jinja.cpp')
                out = tmpl.render(
                    name_camel=class_name,
                    message_type=cpp_msg_type,
                    request_slots=fields.values(),
                    response_slots=response_fields.values(),
                    gen_date=time_string,
                    name_snake=to_snake_case(class_name),
                    name_kebab=to_kebab_case(short_class_name),
                )
                source_file.write(out)
        except Exception as e:
            print(f'Failed to load {args.type} {msg_type_str}: {e}')
            exit(1)

    if args.type == 'msg':
        try:
            msg_type = get_message(msg_type_str)
            fields = extract_fields(msg_type)
            with open(file_name + '.cpp', 'w') as source_file:  # noqa: E501
                tmpl = env.get_template(args.type + '.jinja.cpp')
                out = tmpl.render(
                    name_camel=class_name,
                    message_type=cpp_msg_type,
                    slots=fields.values(),
                    gen_date=time_string,
                    name_snake=to_snake_case(class_name),
                    name_kebab=to_kebab_case(short_class_name),
                )
                source_file.write(out)
        except Exception as e:
            print(f'Failed to load {args.type} {msg_type_str}: {e}')
            exit(1)
    if args.type == 'action':
        try:
            action_type = get_action(msg_type_str)

            msg_type_goal = action_type.Goal
            msg_type_result = action_type.Result
            msg_type_feedback = action_type.Feedback
            fields = extract_fields(msg_type_goal)
            result_fields = extract_fields(msg_type_result)
            feedback_fields = extract_fields(msg_type_feedback)
            with open(file_name + '.cpp', 'w') as source_file:  # noqa: E501
                tmpl = env.get_template('action.jinja.cpp')
                out = tmpl.render(
                    name_camel=class_name,
                    message_type=cpp_msg_type,
                    goal_slots=fields.values(),
                    result_slots=result_fields.values(),
                    feedback_slots=feedback_fields.values(),
                    gen_date=time_string,
                    name_snake=to_snake_case(class_name),
                    name_kebab=to_kebab_case(short_class_name),
                )
                source_file.write(out)
        except Exception as e:
            print(f'Failed to load {args.type} {msg_type_str}: {e}')
            exit(1)

    with open(file_name + '.hpp', 'w') as header_file:  # noqa: E501
        tmpl = env.get_template(args.type + '.jinja.hpp')
        out = tmpl.render(
            name_upper=to_upper_case(args.name),
            name_camel=class_name,
            message_type=cpp_msg_type,
            gen_date=time_string,
            message_include_path=include_header,
            name_snake=to_snake_case(class_name),
        )
        header_file.write(out)
    with open(file_name + '.xml', 'w') as plugin_file, open(
        package_dir + '/templates/' + 'plugin.jinja.xml'
    ) as pl:
        tmpl = Template(pl.read())
        out = tmpl.render(
            message_type=cpp_msg_type,
            gen_date=time_string,
            message_include_path=include_header,
            name_camel=class_name,
            name_snake=to_snake_case(class_name),
        )
        plugin_file.write(out)


if __name__ == '__main__':
    main()
