# Software License Agreement (BSD License)
#
# Copyright (c) 2008, Willow Garage, Inc.
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

import os
from urllib.request import quote

import pydot


# Reference implementation for a dotcode factory
class PydotFactory():

    def __init__(self):
        pass

    def escape_label(self, name):
        if name in ['graph', 'subgraph', 'node', 'edge']:
            ret = '%s_' % name
        else:
            ret = name
        return ret

    def escape_name(self, name):
        ret = quote(name.strip())
        ret = ret.replace('/', '__')
        ret = ret.replace('%', '_')
        ret = ret.replace('-', '_')
        return self.escape_label(ret)

    def get_graph(
            self, graph_type='digraph', rank='same', simplify=True,
            rankdir='TB', ranksep=0.2, compound=True):
        graph = pydot.Dot('graphname',
                          graph_type=graph_type,
                          rank=rank,
                          rankdir=rankdir,
                          simplify=simplify
                          )
        graph.set_ranksep(ranksep)
        graph.set_compound(compound)

        return graph

    def add_node_to_graph(self,
                          graph,
                          nodename,
                          nodelabel=None,
                          shape='box',
                          color=None,
                          url=None,
                          tooltip=None):
        """
        Create a node item for this factory, adds it to the graph.

        Node name can vary from label but must always be same for the same node label
        """
        if nodename is None or nodename == '':
            raise ValueError('Empty Node name')
        if nodelabel is None:
            nodelabel = nodename
        node = pydot.Node(self.escape_name(nodename))
        node.set_shape(shape)
        node.set_label(self.escape_label(nodelabel))
        if tooltip is not None:
            node.set_tooltip(tooltip)
        elif url is not None:
            node.set_tooltip(url)
        if url is not None:
            node.set_URL(self.escape_name(url))
        if color is not None:
            node.set_color(color)
        graph.add_node(node)

    def add_subgraph_to_graph(self,
                              graph,
                              subgraphname,
                              rank='same',
                              simplify=True,
                              rankdir='TB',
                              ranksep=0.2,
                              compound=True,
                              color=None,
                              shape='box',
                              style='bold',
                              subgraphlabel=None):
        """
        Create a cluster subgraph item for this factory, adds it to the graph.

        cluster name can vary from label but must always be same for the same node label.
        Most layouters require cluster names to start with cluster.
        """
        if subgraphname is None or subgraphname == '':
            raise ValueError('Empty subgraph name')
        g = pydot.Cluster(self.escape_name(subgraphname),
                          rank=rank, rankdir=rankdir, simplify=simplify)
        if 'set_style' in g.__dict__:
            g.set_style(style)
        if 'set_shape' in g.__dict__:
            g.set_shape(shape)
        g.set_compound(compound)
        g.set_ranksep(ranksep)
        subgraphlabel = subgraphname if subgraphlabel is None else subgraphlabel
        subgraphlabel = self.escape_label(subgraphlabel)
        if subgraphlabel:
            g.set_label(subgraphlabel)
        if 'set_color' in g.__dict__:
            if color is not None:
                g.set_color(color)
        graph.add_subgraph(g)
        return g

    def add_edge_to_graph(
            self, graph, nodename1, nodename2, label=None, url=None,
            style=None, penwidth=1, color=None, edgetooltip=None):
        edge = pydot.Edge(self.escape_name(nodename1), self.escape_name(nodename2))
        if label is not None and label != '':
            edge.set_label(label)
        if url is not None:
            edge.set_URL(self.escape_name(url))
        if style is not None:
            edge.set_style(style)
        edge.obj_dict['attributes']['penwidth'] = str(penwidth)
        if color is not None:
            edge.obj_dict['attributes']['colorR'] = str(color[0])
            edge.obj_dict['attributes']['colorG'] = str(color[1])
            edge.obj_dict['attributes']['colorB'] = str(color[2])
        if edgetooltip is not None:
            edge.obj_dict['attributes']['edgetooltip'] = edgetooltip
        graph.add_edge(edge)

    def create_dot(self, graph):
        dot = graph.create_dot()
        if type(dot) != str:
            dot = dot.decode()
        # sadly pydot generates line wraps cutting between numbers
        return dot.replace('\\%s' % os.linesep, '').replace('\\\n', '')
