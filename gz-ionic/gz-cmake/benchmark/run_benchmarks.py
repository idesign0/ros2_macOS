#!/usr/bin/env python3
# Copyright 2019 Open Source Robotics Foundation, Inc.
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
import datetime
import logging
import os
import shutil
import subprocess
import sys
import tarfile

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger('run_benchmarks')


def run_benchmark(target, output_dir):
    """Run a specified benchmark target."""
    basename = os.path.basename(target)
    name = basename.split('BENCHMARK_')[1]
    filename = os.path.join(output_dir, name + '.json')

    logger.info('Running benchmark: {name}'.format_map(locals()))

    cmd = [
        target,
        '--benchmark_out={filename}'.format_map(locals()),
        '--benchmark_out_format=json'
    ]

    logger.debug(' '.join(cmd))
    try:
        benchmark = subprocess.call(cmd,
                                    stdout=sys.stdout,
                                    stderr=sys.stderr)
    except (OSError, subprocess.CalledProcessError) as exception:
        logger.error('Exception occurred: ' + str(exception))
        logger.error('Benchmark failed')
    else:
        logger.info('Finished benchmark: {name}'.format_map(locals()))


def run_benchmarks(targets, output_dir):
    """Run all benchmark targets in a list."""
    for target in targets:
        run_benchmark(target, output_dir)


def create_results_dir(project_name, time, root, version_file):
    """Create a directory to store benchmark results in."""
    time_str = time.strftime('%Y-%m-%d_%H-%M-%S')

    folder_name = '{project_name}_{time_str}'.format_map(locals())
    results_dir = os.path.join(root, folder_name)
    logger.info('Creating results dir: {results_dir}'.format_map(locals()))

    # Create output paths as needed.
    if not os.path.exists(root):
        os.mkdir(root)
    if not os.path.exists(results_dir):
        os.mkdir(results_dir)

    # Copy version information over
    shutil.copy(version_file, os.path.join(results_dir, 'version_info.json'))
    return results_dir


def collect_results(project_name, results_dir):
    """Collect results from a given execution into a tar file."""
    path, basename = os.path.split(results_dir)
    tarname = basename + '.tar.gz'

    with tarfile.open(os.path.join(path, tarname), 'w:gz') as tar:
        tar.add(results_dir, arcname=basename)
    logger.info('Benchmark results collected in: ' +
                os.path.join(path, tarname))


if __name__ == '__main__':
    parser = argparse.ArgumentParser('Run and aggregate available benchmarks')
    parser.add_argument('--project-name', help='Name of the Gazebo project')
    parser.add_argument('--version-file',
                        help='Generated file containing version information')
    parser.add_argument('--benchmark-targets', help='Targets to be executed')
    parser.add_argument('--results-root',
                        help='Root directory to store results')

    args = parser.parse_args()

    # Targets will be semicolon delimited from CMake.

    if not os.path.exists(args.benchmark_targets):
        print('Invalid Targets File: {f}'.format(f=args.benchmark_targets))
        parser.exit()

    targets = []
    with open(args.benchmark_targets, 'r') as f:
        for line in f.readlines():
            targets.extend(line.split(';'))

    results_dir = create_results_dir(args.project_name,
                                     datetime.datetime.utcnow(),
                                     args.results_root,
                                     args.version_file)

    run_benchmarks(targets, results_dir)
    collect_results(args.project_name, results_dir)
