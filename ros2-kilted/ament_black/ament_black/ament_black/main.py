#!/usr/bin/env python3

# Copyright 2019 Picknik Robotics.
# Copyright 2023 Dexory
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
import contextlib
from importlib import metadata
import os
import sys
import tempfile
import time
from xml.sax.saxutils import escape
from xml.sax.saxutils import quoteattr

from black import get_sources
from black import main as black
from black import re_compile_maybe_verbose
from black.concurrency import maybe_install_uvloop
from black.const import DEFAULT_EXCLUDES
from black.const import DEFAULT_INCLUDES
from black.report import Report
import click
from packaging.version import Version
from unidiff import PatchSet


def patched_black(*args, **kwargs) -> None:
    from multiprocessing import freeze_support

    maybe_install_uvloop()
    freeze_support()
    black(*args, **kwargs)


def main(argv=sys.argv[1:]):
    parser = argparse.ArgumentParser(
        description='Check code style using black.',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        'paths',
        nargs='*',
        default=[os.curdir],
        help='The files or directories to check. this argument is directly passed to black',
    )
    parser.add_argument(
        '--config',
        metavar='path',
        default=None,
        dest='config_file',
        help='The config file',
    )
    parser.add_argument(
        '--reformat',
        action='store_true',
        help='Reformat the files in place',
    )
    parser.add_argument(
        '--xunit-file',
        help='Generate a xunit compliant XML file',
    )
    args = parser.parse_args(argv)

    # if we have specified a config file, make sure it exists and abort if not
    if args.config_file is not None and not os.path.exists(args.config_file):
        print("Could not find config file '%s'" % args.config_file, file=sys.stderr)
        return 1

    # TODO(Nacho): Inject the config file results into the ctx (use read_pyproject_toml)
    BLACK_VERSION = Version(metadata.version('black'))
    if BLACK_VERSION < Version('23.9.0'):
        sources = get_sources(
            ctx=click.Context(black),
            src=tuple(args.paths),
            quiet=True,
            verbose=False,
            include=re_compile_maybe_verbose(DEFAULT_INCLUDES),
            exclude=re_compile_maybe_verbose(DEFAULT_EXCLUDES),
            extend_exclude=None,
            force_exclude=None,
            report=Report(),
            stdin_filename='',
        )
    else:
        # Hack to support newer versions of black in ROS Jazzy
        # https://github.com/botsandus/ament_black/issues/12
        from black import find_project_root

        sources = get_sources(
            root=find_project_root(tuple(args.paths))[0],
            src=tuple(args.paths),
            quiet=True,
            verbose=False,
            include=re_compile_maybe_verbose(DEFAULT_INCLUDES),
            exclude=re_compile_maybe_verbose(DEFAULT_EXCLUDES),
            extend_exclude=None,
            force_exclude=None,
            report=Report(),
            stdin_filename='',
        )
    checked_files = [str(path) for path in sources]

    if args.xunit_file:
        start_time = time.time()

    # invoke black
    black_args_withouth_path = []
    if args.config_file is not None:
        black_args_withouth_path.extend(['--config', args.config_file])
    black_args = black_args_withouth_path.copy()
    black_args.extend(args.paths)

    with tempfile.NamedTemporaryFile('w') as diff:
        with contextlib.redirect_stdout(diff):
            patched_black([*black_args, '--diff'], standalone_mode=False)
            with open(diff.name, 'r') as file:
                output = file.read()

    # output errors
    patch_set = PatchSet(output)

    changed_files = []
    report = {}
    for patch in patch_set:
        filename = patch.source_file
        changed_files.append(filename)
        report[filename] = patch

    # overwrite original with reformatted files
    if args.reformat and changed_files:
        # pass other arguments, such as the config, but run now only on files to be changed
        reformat_args = black_args_withouth_path.copy()
        reformat_args.extend(changed_files)
        patched_black(reformat_args)

    # output summary
    file_count = sum(1 if report[k] else 0 for k in report.keys())
    replacement_count = sum(len(r) for r in report.values())
    if not file_count:
        print('No problems found')
        rc = 0
    else:
        print(
            '%d files with %d code style divergences' % (file_count, replacement_count),
            file=sys.stderr,
        )
        rc = 1

    # generate xunit file
    if args.xunit_file:
        folder_name = os.path.basename(os.path.dirname(args.xunit_file))
        file_name = os.path.basename(args.xunit_file)
        suffix = '.xml'
        if file_name.endswith(suffix):
            file_name = file_name.split(suffix)[0]
        testname = '%s.%s' % (folder_name, file_name)

        xml = get_xunit_content(report, testname, time.time() - start_time, checked_files)
        path = os.path.dirname(os.path.abspath(args.xunit_file))
        if not os.path.exists(path):
            os.makedirs(path)
        with open(args.xunit_file, 'w') as f:
            f.write(xml)

    return rc


def find_index_of_line_start(data, offset):
    index_1 = data.rfind('\n', 0, offset) + 1
    index_2 = data.rfind('\r', 0, offset) + 1
    return max(index_1, index_2)


def find_index_of_line_end(data, offset):
    index_1 = data.find('\n', offset)
    if index_1 == -1:
        index_1 = len(data)
    index_2 = data.find('\r', offset)
    if index_2 == -1:
        index_2 = len(data)
    return min(index_1, index_2)


def get_line_number(data, offset):
    return data[0:offset].count('\n') + data[0:offset].count('\r') + 1


def get_xunit_content(report, testname, elapsed, checked_files):
    test_count = sum(max(len(r), 1) for r in report.values())
    error_count = sum(len(r) for r in report.values())
    data = {
        'testname': testname,
        'test_count': test_count,
        'error_count': error_count,
        'time': '%.3f' % round(elapsed, 3),
    }
    xml = (
        """<?xml version="1.0" encoding="UTF-8"?>
<testsuite
  name="%(testname)s"
  tests="%(test_count)d"
  errors="0"
  failures="%(error_count)d"
  time="%(time)s"
>
"""
        % data
    )

    for filename in sorted(report.keys()):
        hunks = report[filename]

        if hunks:
            for hunk in hunks:
                data = {
                    'quoted_location': quoteattr('%s:%d' % (filename, hunk.source_start)),
                    'testname': testname,
                    'quoted_message': quoteattr(str(hunk)),
                }
                xml += (
                    """  <testcase
    name=%(quoted_location)s
    classname="%(testname)s"
  >
      <failure message=%(quoted_message)s></failure>
  </testcase>
"""
                    % data
                )

        else:
            # if there are no replacements report a single successful test
            data = {'quoted_location': quoteattr(filename), 'testname': testname}
            xml += (
                """  <testcase
    name=%(quoted_location)s
    classname="%(testname)s"/>
"""
                % data
            )

    # output list of checked files
    data = {'checked_files': escape(''.join(['\n* %s' % r for r in sorted(checked_files)]))}
    xml += (
        """  <system-out>Checked files:%(checked_files)s</system-out>
"""
        % data
    )
    xml += '</testsuite>\n'
    return xml


if __name__ == '__main__':
    sys.exit(main())
