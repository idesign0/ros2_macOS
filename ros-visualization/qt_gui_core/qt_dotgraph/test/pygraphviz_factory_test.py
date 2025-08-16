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

try:
    from qt_dotgraph.pygraphvizfactory import PygraphvizFactory
except ImportError:
    PygraphvizFactory = None


@unittest.skipIf(
    PygraphvizFactory is None, 'skipping test since pygraphviz is unavailable')
class PygraphvizFactoryTest(unittest.TestCase):

    def test_get_graph(self):
        fac = PygraphvizFactory()
        g = fac.get_graph()
        self.assertEqual('same', g.graph_attr['rank'])
        self.assertTrue(g.is_directed())

    def test_add_node(self):
        fac = PygraphvizFactory()
        g = fac.get_graph()
        fac.add_node_to_graph(g, 'foo')
        self.assertEqual(1, len(g.nodes()))
        self.assertEqual('foo', g.nodes()[0].get_name())
        self.assertEqual('foo', g.nodes()[0].attr['label'])

    def test_add_node_escape_name(self):
        fac = PygraphvizFactory()
        g = fac.get_graph()
        fac.add_node_to_graph(g, 'graph')
        self.assertEqual(1, len(g.nodes()))
        self.assertEqual('graph', g.nodes()[0].get_name())
        self.assertEqual('graph', g.nodes()[0].attr['label'])

    def test_add_edge(self):
        fac = PygraphvizFactory()
        g = fac.get_graph()
        fac.add_node_to_graph(g, 'foo')
        fac.add_node_to_graph(g, 'bar')
        fac.add_edge_to_graph(g, 'foo', 'bar')
        self.assertEqual(2, len(g.nodes()))
        self.assertEqual(1, len(g.edges()))
        self.assertEqual('foo', g.edges()[0][0])
        self.assertEqual('bar', g.edges()[0][1])

    def test_add_subgraph(self):
        fac = PygraphvizFactory()
        g = fac.get_graph()
        fac.add_subgraph_to_graph(g, 'foo')
        self.assertEqual(1, len(g.subgraphs()))
        self.assertEqual('cluster_foo', g.subgraphs()[0].get_name())
        self.assertEqual('foo', g.subgraphs()[0].graph_attr['label'])

    def test_add_subgraph_escape_name(self):
        fac = PygraphvizFactory()
        g = fac.get_graph()
        fac.add_subgraph_to_graph(g, 'graph')
        self.assertEqual(1, len(g.subgraphs()))
        self.assertEqual('cluster_graph', g.subgraphs()[0].get_name())
        self.assertEqual('graph', g.subgraphs()[0].graph_attr['label'])

    def test_create_dot(self):
        fac = PygraphvizFactory()
        g = fac.get_graph()
        fac.add_node_to_graph(g, 'foo')
        fac.add_node_to_graph(g, 'edge')
        fac.add_edge_to_graph(g, 'foo', 'edge')
        fac.add_subgraph_to_graph(g, 'graph')
        snippets = ['strict digraph { graph',
                    'foo',
                    'label=foo',
                    '"edge"',
                    'label="edge"',
                    'foo -> "edge"']
        result = fac.create_dot(g)
        # get rid of version specific quotes / whitespaces
        result = re.sub('""', ' ', result)
        result = re.sub('[\n\t ]+', ' ', result)
        for sn in snippets:
            self.assertTrue(sn in result, '%s \nmissing in\n %s' % (sn, result))
