#!/usr/bin/env python
# Software License Agreement (BSD License)
#
# Copyright (c) 2009, Willow Garage, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# This file does not pass flake8 due to the raw string literals below.
# flake8: noqa

import subprocess
import sys
import unittest

from python_qt_binding.QtWidgets import QApplication
from qt_dotgraph.dot_to_qt import DotToQtGenerator, get_unquoted


def check_x_server():
    p = subprocess.Popen(sys.executable, stdin=subprocess.PIPE)
    p.stdin.write(b'from python_qt_binding.QtWidgets import QApplication\n')
    p.stdin.write(b'app = QApplication([])\n')
    p.stdin.close()
    p.communicate()

    print(p.returncode)

    return p.returncode == 0


class DotToQtGeneratorTest(unittest.TestCase):

    DOT_CODE = r'''
    digraph graph_name {
        graph [bb="0,0,154,108",
            rank=same
        ];
        node [label="\N"];
        subgraph cluster_foo {
            graph [bb="1,1,100,101",
                label=cluster_foo
            ];
        }
        foo	 [height=0.5,
            label=foo,
            pos="77,90",
            shape=box,
            width=0.75];
        bar	 [height=0.5,
            label=barbarbarbarbarbarbarbar,
            pos="77,18",
            shape=box,
            width=2.25];
        foo -> bar [pos="e,77,36.104 77,71.697 77,63.983 77,54.712 77,46.112"];
    }
    '''

    _Q_APP = None

    def __init__(self, *args):
        super(DotToQtGeneratorTest, self).__init__(*args)

        # needed for creation of QtGraphic items in NodeItem.__init__
        if DotToQtGeneratorTest._Q_APP is None:
            if check_x_server():
                DotToQtGeneratorTest._Q_APP = QApplication([])

    def test_simple_integration(self):
        if DotToQtGeneratorTest._Q_APP is None:
            raise unittest.case.SkipTest

        (nodes, edges) = DotToQtGenerator().dotcode_to_qt_items(
            DotToQtGeneratorTest.DOT_CODE, 1)
        self.assertEqual(3, len(nodes))  # cluster_foo, foo and bar
        self.assertEqual(1, len(edges))  # foo -> bar

    def test_label_sizes(self):
        if DotToQtGeneratorTest._Q_APP is None:
            raise unittest.case.SkipTest

        (nodes, edges) = DotToQtGenerator().dotcode_to_qt_items(DotToQtGeneratorTest.DOT_CODE, 1)

        self.longMessage = True
        for name, node in nodes.items():
            shape_rect = node._graphics_item.sceneBoundingRect()
            label_rect = node._label.sceneBoundingRect()
            self.assertLess(
                label_rect.width(),
                shape_rect.width(),
                "Label text for '%s' is wider than surrounding shape." % name)
            self.assertLess(
                label_rect.height(),
                shape_rect.height(),
                "Label text for '%s' is higher than surrounding shape." % name)

    def test_unquoted(self):
        self.assertEqual('foo', get_unquoted({'bar': 'foo'}, 'bar'))

    def test_recursive(self):
        if DotToQtGeneratorTest._Q_APP is None:
            raise unittest.case.SkipTest

        gen = DotToQtGenerator()
        dotcode = r'''
        strict digraph "" {
            graph [bb="0,0,249,541",
                    compound=True,
                    rank=same,
                    rankdir=TB,
                    ranksep=0.2,
                    simplify=True
            ];
            node [label="\N"];
            subgraph "/Container" {
                    graph [bb="8,67,241,321",
                            color=None,
                            compound=True,
                            label="/Container",
                            lheight=0.21,
                            lp="124.5,309.5",
                            lwidth=0.81,
                            rank=same,
                            rankdir=TB,
                            ranksep=0.2,
                            style=bold
                    ];
                    subgraph "/Container/Subcontainer" {
                            graph [bb="84,142,233,287",
                                    color=None,
                                    compound=True,
                                    label="/Container/Subcontainer",
                                    lheight=0.21,
                                    lp="158.5,275.5",
                                    lwidth=1.85,
                                    rank=same,
                                    rankdir=TB,
                                    ranksep=0.2,
                                    style=bold
                            ];
                            "/Container/Subcontainer/logstate1" [height=0.5,
                                    label=logstate1,
                                    pos="133,235",
                                    shape=box,
                                    url=None,
                                    width=0.90278];
                            "/Container/Subcontainer/finished" [color=blue,
                                    height=0.5,
                                    label=finished,
                                    pos="133,168",
                                    shape=ellipse,
                                    url=None,
                                    width=1.0833];
                            "/Container/Subcontainer/logstate1" -> "/Container/Subcontainer/finished" [label=done,
                                    lp="146.5,201.5",
                                    pos="e,133,186.19 133,216.92 133,210.7 133,203.5 133,196.6",
                                    url=None];
                    }
                    "/Container/finished"            [color=blue,
                            height=0.5,
                            label=finished,
                            pos="86,93",
                            shape=ellipse,
                            url=None,
                            width=1.0833];
                    "/Container/Subcontainer/finished" -> "/Container/finished" [label=finished,
                            lp="132,126.5",
                            pos="e,96.623,110.5 122.33,150.44 116.39,141.19 108.85,129.5 102.19,119.15",
                            url=None];
                    "/Container/logstate"            [height=0.5,
                            label=logstate,
                            pos="46,168",
                            shape=box,
                            url=None,
                            width=0.81944];
                    "/Container/logstate" -> "/Container/finished" [label=done,
                            lp="82.5,126.5",
                            pos="e,74.304,110.45 53.482,149.8 57.712,140.5 63.287,128.93 69,119 69.051,118.91 69.102,118.82 69.153,118.74",
                            url=None];
            }
            "/finished"      [height=0.5,
                    pos="86,18",
                    width=1.1555];
            "/Container/finished" -> "/finished"     [label=finished,
                    lp="108,51.5",
                    pos="e,86,36.176 86,74.7 86,66.245 86,55.869 86,46.373",
                    url=None];
            "/start" -> "/Container/Subcontainer/logstate1"   [
                    lp="146.5,436.5",
                    pos="e,133,250.01 133,355.84 133,337.5 133,316.81 133,260.22",
                    url=None];
            "/start" -> "/Container/logstate"   [
                    lp="146.5,436.5",
                    pos="e,46,185.01 133,355.84 46,337.5 46,316.81 46,192.22",
                    url=None];
            "/start"         [height=0.5,
                    pos="133,373",
                    width=0.79437];
        }
        '''

        (nodes, edges) = gen.dotcode_to_qt_items(dotcode, 1)

        expected_nodes = [
            '"/Container"', '"/Container/Subcontainer"', '"/Container/Subcontainer/finished"',
            '"/Container/Subcontainer/logstate1"', '"/Container/finished"',
            '"/Container/logstate"', '"/finished"', '"/start"'
        ]
        expected_edges = [
            '/Container/Subcontainer/finished_TO_/Container/finished_finished',
            '/Container/Subcontainer/logstate1_TO_/Container/Subcontainer/finished_done',
            '/Container/finished_TO_/finished_finished',
            '/Container/logstate_TO_/Container/finished_done',
            '/start_TO_/Container/Subcontainer/logstate1',
            '/start_TO_/Container/logstate',
        ]

        nodes_sorted = list(sorted(nodes.keys()))
        edges_sorted = list(sorted(edges.keys()))

        self.assertEqual(expected_nodes, nodes_sorted)
        self.assertEqual(expected_edges, edges_sorted)
