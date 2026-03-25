#!/usr/bin/env python3
# Copyright (c) 2024-2026 Carologistics
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# Read the full text in the LICENSE.GPL file in the main directory.

import argparse
from datetime import datetime
import re

from ament_index_python.packages import get_package_share_directory
from jinja2 import Environment, FileSystemLoader


def to_camel_case(s):
    words = s.split('_')
    camel_case_words = [word.capitalize() for word in words]
    camel_case_string = ''.join(camel_case_words)
    return camel_case_string


def to_upper_case(s):
    words = s.split('_')
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


def main():
    # Set up argument parsing
    parser = argparse.ArgumentParser(
        description='Generate CLIPS bindings for ROS 2 messages, services, \
        or actions for usage with the CLIPS Executive.'
    )
    parser.add_argument('plugin_name', type=str, help='Name of the plugin in camel case')

    # Parse the arguments
    args = parser.parse_args()

    camel_case_plugin_name = args.plugin_name

    print(f'Generating bindings for plugin "{args.plugin_name}"')
    print(f'Generating bindings for plugin "{camel_case_plugin_name}"')
    package_dir = get_package_share_directory('cx_protobuf_plugin')
    time_string = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    file_name = to_snake_case(camel_case_plugin_name)

    # Create environment for Jinja templates
    env = Environment(loader=FileSystemLoader(package_dir + '/templates'))

    # Generate source files for the plugin
    try:
        with open(file_name + '.cpp', 'w') as source_file:
            tmpl = env.get_template('plugin.jinja.cpp')
            out = tmpl.render(
                gen_date=time_string,
                name_camel=camel_case_plugin_name,
                name_snake=to_snake_case(camel_case_plugin_name),
                name_kebab=to_kebab_case(camel_case_plugin_name),
            )
            source_file.write(out)
    except Exception as e:
        print(f'Failed to load plugin.jinja.cpp: {e}')
        exit(1)

    try:
        with open(file_name + '.hpp', 'w') as header_file:
            tmpl = env.get_template('plugin.jinja.hpp')
            out = tmpl.render(
                gen_date=time_string,
                name_camel=camel_case_plugin_name,
                name_snake=to_snake_case(camel_case_plugin_name),
                name_kebab=to_kebab_case(camel_case_plugin_name),
            )
            header_file.write(out)
    except Exception as e:
        print(f'Failed to load plugin.jinja.hpp: {e}')
        exit(1)

    # Generate plugin XML file
    try:
        with open(file_name + '.xml', 'w') as plugin_file:
            tmpl = env.get_template('plugin.jinja.xml')
            out = tmpl.render(
                gen_date=time_string,
                name_camel=camel_case_plugin_name,
                name_snake=to_snake_case(camel_case_plugin_name),
                name_kebab=to_kebab_case(camel_case_plugin_name),
            )
            plugin_file.write(out)
    except Exception as e:
        print(f'Failed to load plugin.jinja.xml: {e}')
        exit(1)


if __name__ == '__main__':
    main()
