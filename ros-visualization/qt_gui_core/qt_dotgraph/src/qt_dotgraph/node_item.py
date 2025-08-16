# Copyright (c) 2011, Dirk Thomas, TU Darmstadt
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials provided
#     with the distribution.
#   * Neither the name of the TU Darmstadt nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

from __future__ import print_function

import sys

from python_qt_binding.QtCore import Qt
from python_qt_binding.QtGui import QBrush, QPainterPath, QPen
from python_qt_binding.QtWidgets import (QGraphicsEllipseItem,
                                         QGraphicsRectItem,
                                         QGraphicsSimpleTextItem)

from qt_dotgraph.dot_shapes import QGraphicsBox3dItem
from qt_dotgraph.graph_item import GraphItem


class NodeItem(GraphItem):

    def __init__(
            self, highlight_level, bounding_box, label, shape, color=None,
            parent=None, label_pos=None, tooltip=None):
        super(NodeItem, self).__init__(highlight_level, parent)

        self._default_color = self._COLOR_BLACK if color is None else color
        self._brush = QBrush(self._default_color)
        self._label_pen = QPen()
        self._label_pen.setColor(self._default_color)
        self._label_pen.setJoinStyle(Qt.RoundJoin)
        self._ellipse_pen = QPen(self._label_pen)
        self._ellipse_pen.setWidth(1)

        self._incoming_edges = set()
        self._outgoing_edges = set()

        self.parse_shape(shape, bounding_box)
        self.addToGroup(self._graphics_item)

        self._label = QGraphicsSimpleTextItem(label)
        self._label.setFont(GraphItem._LABEL_FONT)
        label_rect = self._label.boundingRect()
        if label_pos is None:
            label_rect.moveCenter(bounding_box.center())
        else:
            label_rect.moveCenter(label_pos)
        self._label.setPos(label_rect.x(), label_rect.y())
        self.addToGroup(self._label)
        if tooltip is not None:
            self.setToolTip(tooltip)

        self.set_node_color()

        self.setAcceptHoverEvents(True)

        self.hovershape = None

    def parse_shape(self, shape, bounding_box):
        if shape in ('box', 'rect', 'rectangle'):
            self._graphics_item = QGraphicsRectItem(bounding_box)
        elif shape in ('ellipse', 'oval', 'circle'):
            self._graphics_item = QGraphicsEllipseItem(bounding_box)
        elif shape in ('box3d', ):
            self._graphics_item = QGraphicsBox3dItem(bounding_box)
        else:
            print("Invalid shape '%s', defaulting to ellipse" % shape, file=sys.stderr)
            self._graphics_item = QGraphicsEllipseItem(bounding_box)

    def set_hovershape(self, newhovershape):
        self.hovershape = newhovershape

    def shape(self):
        if self.hovershape is not None:
            path = QPainterPath()
            path.addRect(self.hovershape)
            return path
        else:
            return super(self.__class__, self).shape()

    def add_incoming_edge(self, edge):
        self._incoming_edges.add(edge)

    def add_outgoing_edge(self, edge):
        self._outgoing_edges.add(edge)

    def set_node_color(self, color=None):
        if color is None:
            color = self._default_color

        self._brush.setColor(color)
        self._ellipse_pen.setColor(color)
        self._label_pen.setColor(color)

        self._graphics_item.setPen(self._ellipse_pen)
        self._label.setBrush(self._brush)
        self._label.setPen(self._label_pen)

    def hoverEnterEvent(self, event):
        # hovered node item in red
        self.set_node_color(self._COLOR_RED)

        if self._highlight_level > 1:
            cyclic_edges = self._incoming_edges.intersection(self._outgoing_edges)
            # incoming edges in blue
            incoming_nodes = set()
            for incoming_edge in self._incoming_edges.difference(cyclic_edges):
                incoming_edge.set_node_color(self._COLOR_BLUE)
                incoming_edge.set_label_color(self._COLOR_BLUE)
                if incoming_edge.from_node != self:
                    incoming_nodes.add(incoming_edge.from_node)
            # outgoing edges in green
            outgoing_nodes = set()
            for outgoing_edge in self._outgoing_edges.difference(cyclic_edges):
                outgoing_edge.set_node_color(self._COLOR_GREEN)
                outgoing_edge.set_label_color(self._COLOR_GREEN)
                if outgoing_edge.to_node != self:
                    outgoing_nodes.add(outgoing_edge.to_node)
            # incoming/outgoing edges in teal
            for edge in cyclic_edges:
                edge.set_node_color(self._COLOR_TEAL)

            if self._highlight_level > 2:
                cyclic_nodes = incoming_nodes.intersection(outgoing_nodes)
                # incoming nodes in blue
                for incoming_node in incoming_nodes.difference(cyclic_nodes):
                    incoming_node.set_node_color(self._COLOR_BLUE)
                # outgoing nodes in green
                for outgoing_node in outgoing_nodes.difference(cyclic_nodes):
                    outgoing_node.set_node_color(self._COLOR_GREEN)
                # incoming/outgoing nodes in teal
                for node in cyclic_nodes:
                    node.set_node_color(self._COLOR_TEAL)

    def hoverLeaveEvent(self, event):
        self.set_node_color()
        if self._highlight_level > 1:
            for incoming_edge in self._incoming_edges:
                incoming_edge.set_node_color()
                incoming_edge.set_label_color()
                if self._highlight_level > 2 and incoming_edge.from_node != self:
                    incoming_edge.from_node.set_node_color()
            for outgoing_edge in self._outgoing_edges:
                outgoing_edge.set_node_color()
                outgoing_edge.set_label_color()
                if self._highlight_level > 2 and outgoing_edge.to_node != self:
                    outgoing_edge.to_node.set_node_color()
