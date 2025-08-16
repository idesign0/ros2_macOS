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

import pygraphviz


# Reference implementation for a dotcode factory
class PygraphvizFactory():

    def __init__(self):
        pass

    def get_graph(
            self, graph_type='digraph', rank='same',
            simplify=True, rankdir='TB', ranksep=0.2, compound=True):
        graph = pygraphviz.AGraph(
            directed=(graph_type == 'digraph'), ranksep=ranksep, rankdir=rankdir,
            rank=rank, compound=True, simplify=simplify)
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

        kwargs = {}
        if tooltip is not None:
            kwargs['tooltip'] = tooltip
        if color is not None:
            kwargs['color'] = color

        graph.add_node(nodename, label=str(nodelabel), shape=shape, url=url, **kwargs)

    def add_subgraph_to_graph(self,
                              graph,
                              subgraphlabel,
                              rank='same',
                              simplify=True,
                              rankdir='TB',
                              ranksep=0.2,
                              compound=True,
                              color=None,
                              shape='box',
                              style='bold'):
        """
        Create a cluster subgraph  item for this factory, adds it to the graph.

        Cluster name can vary from label but must always be same for the same node label.
        Most layouters require cluster names to start with cluster.
        """
        if subgraphlabel is None or subgraphlabel == '':
            raise ValueError('Empty subgraph label')

        sg = graph.add_subgraph(
            name='cluster_%s' % subgraphlabel, ranksep=ranksep, rankdir=rankdir,
            rank=rank, compound=compound, label=str(subgraphlabel), style=style, color=color)

        return sg

    def add_edge_to_graph(
            self, graph, nodename1, nodename2, label=None, url=None,
            simplify=True, style=None, edgetooltip=None):
        kwargs = {'url': url}
        if label is not None:
            kwargs['label'] = label
        if style is not None:
            kwargs['style'] = style
        if edgetooltip is not None:
            kwargs['edgetooltip'] = edgetooltip
        graph.add_edge(nodename1, nodename2, **kwargs)

    def create_dot(self, graph):
        graph.layout('dot')
        # sadly pygraphviz generates line wraps cutting between numbers
        return graph.string().replace('\\\n', '')
