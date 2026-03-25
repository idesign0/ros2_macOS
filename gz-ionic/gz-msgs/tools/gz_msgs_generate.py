#!/usr/bin/env python3
#
# Copyright (C) 2022 Open Source Robotics Foundation
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
import subprocess
import sys

def main(argv=sys.argv[1:]):
    parser = argparse.ArgumentParser(
        description='Generate protobuf support files',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument(
        '--protoc-exec',
        required=True,
        help='The path to the protoc executable')
    parser.add_argument(
        '--gz-generator-bin',
        help='The path to the gz specific protobuf generator')
    parser.add_argument(
        '--generate-cpp',
        help='Flag to indicate if C++ bindings should be generated',
        action='store_true')
    parser.add_argument(
        '--generate-python',
        help='Flag to indicate if Python bindings should be generated',
        action='store_true')
    parser.add_argument(
        '--output-cpp-path',
        help='The basepath of the generated C++ files')
    parser.add_argument(
        '--output-python-path',
        help='The basepath of the generated C++ files')
    parser.add_argument(
        '--proto-path',
        required=True,
        help='The location of the protos',
        action='append')
    parser.add_argument(
        '--input-path',
        required=True,
        help='The location of the template files')
    parser.add_argument(
        '--dependency-proto-descs',
        nargs='*',
        help='The location of proto descriptor files these messages depend on')
    parser.add_argument(
        '--dllexport-decl',
        help='The DLL visibility macro to use, if not set, no macro will be used')
    args = parser.parse_args(argv)

    # First generate the base cpp files
    cmd = [args.protoc_exec]

    for path in args.proto_path:
        cmd += [f'--proto_path={path}']

    if args.dependency_proto_descs:
        for path in args.dependency_proto_descs:
            cmd += [f'--descriptor_set_in={path}']

    if args.generate_cpp:
        cmd += [f'--plugin=protoc-gen-gzmsgs={args.gz_generator_bin}']
        if args.dllexport_decl:
            cmd += [f'--cpp_out=dllexport_decl={args.dllexport_decl}:{args.output_cpp_path}']
        else:
            cmd += [f'--cpp_out={args.output_cpp_path}']

        cmd += [f'--gzmsgs_out={args.output_cpp_path}']
    if args.generate_python:
        cmd += [f'--python_out={args.output_python_path}']

    cmd += [args.input_path]

    os.makedirs(args.output_cpp_path, exist_ok=True)

    try:
        subprocess.check_call(cmd)
    except subprocess.CalledProcessError as e:
        print(f'Failed to execute protoc compiler: {e}')
        sys.exit(-1)

    # Move original generated cpp to details/
    proto_file = os.path.splitext(os.path.relpath(args.input_path, args.proto_path[0]))[0]
    detail_proto_file = proto_file.split(os.sep)

    detail_proto_dir = detail_proto_file[:-1]
    detail_proto_dir.append('details')
    detail_proto_dir = os.path.join(*detail_proto_dir)
    detail_proto_file.insert(-1, 'details')
    detail_proto_file = os.path.join(*detail_proto_file)

    header = os.path.join(args.output_cpp_path, proto_file + ".pb.h")
    gz_header = os.path.join(args.output_cpp_path, proto_file + ".gz.h")
    detail_header = os.path.join(args.output_cpp_path, detail_proto_file + ".pb.h")

    try:
        os.makedirs(os.path.join(args.output_cpp_path, detail_proto_dir),
                exist_ok=True)
        # Windows cannot rename a file to an existing file
        if os.path.exists(detail_header):
            os.remove(detail_header)

        os.rename(header, detail_header)
        os.rename(gz_header, header)
    except Exception as e:
        print(f'Failed to manipulate gz-msgs headers: {e}')
        sys.exit(-1)

if __name__ == '__main__':
    sys.exit(main())
