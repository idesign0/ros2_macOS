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

import re
import unittest

from qt_dotgraph.pydotfactory import PydotFactory


class PyDotFactoryTest(unittest.TestCase):

    def test_get_graph(self):
        fac = PydotFactory()
        g = fac.get_graph()
        self.assertEqual('same', g.get_rank())
        self.assertEqual('digraph', g.get_graph_type())

    def test_add_node(self):
        fac = PydotFactory()
        g = fac.get_graph()
        fac.add_node_to_graph(g, 'foo')
        self.assertEqual(1, len(g.get_nodes()))
        self.assertEqual('foo', g.get_nodes()[0].get_name())
        self.assertEqual('foo', g.get_nodes()[0].get_label())

    def test_add_node_escape_name(self):
        fac = PydotFactory()
        g = fac.get_graph()
        fac.add_node_to_graph(g, 'graph')
        self.assertEqual(1, len(g.get_nodes()))
        self.assertEqual('graph_', g.get_nodes()[0].get_name())
        self.assertEqual('graph_', g.get_nodes()[0].get_label())

    def test_add_edge(self):
        fac = PydotFactory()
        g = fac.get_graph()
        fac.add_node_to_graph(g, 'foo')
        fac.add_node_to_graph(g, 'bar')
        fac.add_edge_to_graph(g, 'foo', 'bar')
        self.assertEqual(2, len(g.get_nodes()))
        self.assertEqual(1, len(g.get_edges()))
        self.assertEqual('foo', g.get_edges()[0].get_source())
        self.assertEqual('bar', g.get_edges()[0].get_destination())

    def test_add_subgraph(self):
        fac = PydotFactory()
        g = fac.get_graph()
        fac.add_subgraph_to_graph(g, 'foo')
        self.assertEqual(1, len(g.get_subgraph_list()))
        self.assertEqual('cluster_foo', g.get_subgraph_list()[0].get_name())
        self.assertEqual('foo', g.get_subgraph_list()[0].get_label())

    def test_add_subgraph_escape_name(self):
        fac = PydotFactory()
        g = fac.get_graph()
        fac.add_subgraph_to_graph(g, 'graph')
        self.assertEqual(1, len(g.get_subgraph_list()))
        self.assertEqual('cluster_graph_', g.get_subgraph_list()[0].get_name())
        self.assertEqual('graph_', g.get_subgraph_list()[0].get_label())

    def test_create_dot(self):
        fac = PydotFactory()
        g = fac.get_graph()
        fac.add_node_to_graph(g, 'foo')
        fac.add_node_to_graph(g, 'edge')
        fac.add_edge_to_graph(g, 'foo', 'edge')
        fac.add_subgraph_to_graph(g, 'foo')
        snippets = ['digraph graphname { graph [',
                    'rankdir=TB',
                    'compound=True',
                    'rank=same',
                    'node [label="\\N"]',
                    'subgraph cluster_foo { graph [',
                    'foo [',
                    'label=foo',
                    'shape=box',
                    'pos="',
                    'edge_ [',
                    'label=edge_',
                    'foo -> edge_ [',
                    '"]; }']
        try:
            result = fac.create_dot(g)
        except FileNotFoundError:
            raise unittest.SkipTest('skipping test since dot is unavailable')
        # get rid of version specific whitespaces
        result = re.sub('[\n\t\r ]+', ' ', result)
        for sn in snippets:
            self.assertTrue(sn in result, '%s \nmissing in\n %s' % (sn, result))
