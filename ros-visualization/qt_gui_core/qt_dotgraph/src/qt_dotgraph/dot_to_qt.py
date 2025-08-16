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

import codecs

import pydot

from python_qt_binding.QtCore import QPointF, QRectF
from python_qt_binding.QtGui import QColor

from .edge_item import EdgeItem
from .node_item import NodeItem

POINTS_PER_INCH = 72


# hack required by pydot
def get_unquoted(item, name):
    value = item.get(name)
    if value is None:
        return None
    try:
        return value.strip('"\n"')
    except AttributeError:
        # not part of the string family
        return value


# approximately, for workarounds (TODO: get this from dotfile somehow)
LABEL_HEIGHT = 30


# Class generating Qt Elements from doctcode
class DotToQtGenerator():

    def __init__(self):
        pass

    def getNodeItemForSubgraph(self, subgraph, highlight_level, scene=None):
        # let pydot imitate pygraphviz api
        attr = {}
        for name in subgraph.get_attributes().keys():
            value = get_unquoted(subgraph, name)
            attr[name] = value
        obj_dic = subgraph.__getattribute__('obj_dict')
        for name in obj_dic:
            if name not in ['nodes', 'attributes', 'parent_graph'] and obj_dic[name] is not None:
                attr[name] = get_unquoted(obj_dic, name)
            elif name == 'nodes':
                for key in obj_dic['nodes']['graph'][0]['attributes']:
                    attr[key] = get_unquoted(obj_dic['nodes']['graph'][0]['attributes'], key)
        subgraph.attr = attr

        bb = subgraph.attr.get('bb', None)
        if bb is None:
            # no bounding box
            return None
        bb = bb.strip('"').split(',')
        if len(bb) < 4:
            # bounding box is empty
            return None
        bounding_box = QRectF(0, 0, float(bb[2]) - float(bb[0]), float(bb[3]) - float(bb[1]))
        if 'lp' in subgraph.attr:
            label_pos = subgraph.attr['lp'].strip('"').split(',')
        else:
            label_pos = (float(bb[0]) + (float(bb[2]) - float(bb[0])) / 2,
                         float(bb[1]) + (float(bb[3]) - float(bb[1])) - LABEL_HEIGHT / 2)
        bounding_box.moveCenter(QPointF(float(bb[0]) + (float(bb[2]) - float(bb[0])) / 2,
                                        -float(bb[1]) - (float(bb[3]) - float(bb[1])) / 2))
        name = subgraph.attr.get('label', '')
        color = QColor(subgraph.attr['color']) if 'color' in subgraph.attr else None
        subgraph_nodeitem = NodeItem(highlight_level,
                                     bounding_box,
                                     label=name,
                                     shape='box',
                                     color=color,
                                     parent=scene.activePanel() if scene is not None else None,
                                     label_pos=QPointF(float(label_pos[0]), -float(label_pos[1])))
        bounding_box = QRectF(bounding_box)
        # With clusters we have the problem that mouse hovers cannot
        # decide whether to be over the cluster or a subnode. Using
        # just the "title area" solves this. TODO: Maybe using a
        # border region would be even better (multiple RectF)
        bounding_box.setHeight(LABEL_HEIGHT)
        subgraph_nodeitem.set_hovershape(bounding_box)

        if scene is not None:
            scene.addItem(subgraph_nodeitem)
        return subgraph_nodeitem

    def getNodeItemForNode(self, node, highlight_level, scene=None):
        """Return a pyqt NodeItem object, or None in case of error or invisible style."""
        # let pydot imitate pygraphviz api
        attr = {}
        for name in node.get_attributes().keys():
            value = get_unquoted(node, name)
            attr[name] = value
        obj_dic = node.__getattribute__('obj_dict')
        for name in obj_dic:
            if name not in ['attributes', 'parent_graph'] and obj_dic[name] is not None:
                attr[name] = get_unquoted(obj_dic, name)
        node.attr = attr

        if node.attr.get('style') == 'invis':
            return None

        color = QColor(node.attr['color']) if 'color' in node.attr else None

        name = node.attr.get('label', node.attr.get('name'))
        if name is None:
            print('Error, no label defined for node with attr: %s' % node.attr)
            return None

        name = codecs.escape_decode(name)[0].decode('utf-8')

        # decrease rect by one so that edges do not reach inside
        bb_width = node.attr.get('width', len(name) / 5)
        bb_height = node.attr.get('height', 1.0)
        bounding_box = QRectF(0, 0, POINTS_PER_INCH * float(
            bb_width) - 1.0, POINTS_PER_INCH * float(bb_height) - 1.0)
        pos = node.attr.get('pos', '0,0').split(',')
        bounding_box.moveCenter(QPointF(float(pos[0]), -float(pos[1])))

        node_item = NodeItem(highlight_level=highlight_level,
                             bounding_box=bounding_box,
                             label=name,
                             shape=node.attr.get('shape', 'ellipse'),
                             color=color,
                             tooltip=node.attr.get('tooltip'),
                             parent=scene.activePanel() if scene is not None else None
                             # label_pos=None
                             )
        if scene is not None:
            scene.addItem(node_item)
        return node_item

    def addEdgeItem(
            self, edge, nodes, edges, highlight_level, same_label_siblings=False, scene=None):
        """
        Add EdgeItem by data in edge to edges.

        :param same_label_siblings:
            if true, edges with same label will be considered siblings (collective highlighting)
        """
        # let pydot imitate pygraphviz api
        attr = {}
        for name in edge.get_attributes().keys():
            value = get_unquoted(edge, name)
            attr[name] = value
        edge.attr = attr

        style = edge.attr.get('style')
        if style == 'invis':
            return

        label = edge.attr.get('label', None)
        label_pos = edge.attr.get('lp', None)
        label_center = None
        if label_pos is not None:
            label_pos = label_pos.split(',')
            label_center = QPointF(float(label_pos[0]), -float(label_pos[1]))

        # try pydot, fallback for pygraphviz
        source_node = edge.get_source() if hasattr(edge, 'get_source') else edge[0]
        destination_node = edge.get_destination() if hasattr(edge, 'get_destination') else edge[1]

        # create edge with from-node and to-node
        edge_pos = edge.attr.get('pos')
        if edge_pos is None:
            return
        if label is not None:
            label = codecs.escape_decode(label)[0].decode('utf-8')

        penwidth = int(edge.attr.get('penwidth', 1))

        color = None
        if 'colorR' in edge.attr and 'colorG' in edge.attr and 'colorB' in edge.attr:
            r = edge.attr['colorR']
            g = edge.attr['colorG']
            b = edge.attr['colorB']
            color = QColor(float(r), float(g), float(b))

        edge_item = EdgeItem(highlight_level=highlight_level,
                             spline=edge_pos,
                             label_center=label_center,
                             label=label,
                             from_node=nodes[source_node],
                             to_node=nodes[destination_node],
                             penwidth=penwidth,
                             parent=scene.activePanel() if scene is not None else None,
                             edge_color=color,
                             style=style,
                             edgetooltip=edge.attr.get('edgetooltip'))

        if same_label_siblings:
            if label is None:
                # for sibling detection
                label = '%s_%s' % (source_node, destination_node)
            # symmetrically add all sibling edges with same label
            if label in edges:
                for sibling in edges[label]:
                    edge_item.add_sibling_edge(sibling)
                    sibling.add_sibling_edge(edge_item)

        edge_name = source_node.strip('"\n"') + '_TO_' + destination_node.strip('"\n"')
        if label is not None:
            edge_name = edge_name + '_' + label

        if edge_name not in edges:
            edges[edge_name] = []
        edges[edge_name].append(edge_item)
        if scene is not None:
            edge_item.add_to_scene(scene)

    def dotcode_to_qt_items(self, dotcode, highlight_level, same_label_siblings=False, scene=None):
        """
        Take dotcode, run layout, and creates qt items based on the dot layout.

        Returns two dicts, one mapping node names to Node_Item, one mapping edge names to lists of
            Edge_Item.
        :param same_label_siblings:
            if true, edges with same label will be considered siblings (collective highlighting)
        """
        # layout graph
        if dotcode is None:
            return {}, {}
        graph = pydot.graph_from_dot_data(dotcode)
        if isinstance(graph, list):
            graph = graph[0]

        # graph = pygraphviz.AGraph(string=self._current_dotcode, strict=False, directed=True)
        # graph.layout(prog='dot')

        nodes = self.parse_nodes(graph, highlight_level, scene=scene)
        edges = self.parse_edges(graph, nodes, highlight_level, same_label_siblings, scene=scene)
        return nodes, edges

    def parse_nodes(self, graph, highlight_level, scene=None):
        """Recursively search all nodes inside the graph and all subgraphs."""
        # let pydot imitate pygraphviz api
        graph.nodes_iter = graph.get_node_list
        graph.subgraphs_iter = graph.get_subgraph_list

        nodes = {}
        for subgraph in graph.subgraphs_iter():
            subgraph_nodeitem = self.getNodeItemForSubgraph(subgraph, highlight_level, scene=scene)
            nodes.update(self.parse_nodes(subgraph, highlight_level, scene=scene))
            # skip subgraphs with empty bounding boxes
            if subgraph_nodeitem is None:
                continue

            subgraph.nodes_iter = subgraph.get_node_list
            nodes[subgraph.get_name()] = subgraph_nodeitem
            for node in subgraph.nodes_iter():
                # hack required by pydot
                if node.get_name() in ('graph', 'node', 'empty'):
                    continue
                nodes[node.get_name()] = \
                    self.getNodeItemForNode(node, highlight_level, scene=scene)
        for node in graph.nodes_iter():
            # hack required by pydot
            if node.get_name() in ('graph', 'node', 'empty'):
                continue
            nodes[node.get_name()] = self.getNodeItemForNode(node, highlight_level, scene=scene)
        return nodes

    def parse_edges(self, graph, nodes, highlight_level, same_label_siblings, scene=None):
        """Recursively search all edges inside the graph and all subgraphs."""
        # let pydot imitate pygraphviz api
        graph.subgraphs_iter = graph.get_subgraph_list
        graph.edges_iter = graph.get_edge_list

        edges = {}
        for subgraph in graph.subgraphs_iter():
            subgraph.edges_iter = subgraph.get_edge_list
            edges.update(
                self.parse_edges(
                    subgraph, nodes, highlight_level, same_label_siblings, scene=scene))
            for edge in subgraph.edges_iter():
                self.addEdgeItem(edge, nodes, edges,
                                 highlight_level=highlight_level,
                                 same_label_siblings=same_label_siblings,
                                 scene=scene)

        for edge in graph.edges_iter():
            self.addEdgeItem(edge, nodes, edges,
                             highlight_level=highlight_level,
                             same_label_siblings=same_label_siblings,
                             scene=scene)

        return edges
