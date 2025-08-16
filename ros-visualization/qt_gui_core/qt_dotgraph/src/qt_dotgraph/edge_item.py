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

from python_qt_binding.QtCore import QPointF, Qt
from python_qt_binding.QtGui import QBrush, QPainterPath, QPen, QPolygonF
from python_qt_binding.QtWidgets import (QGraphicsPathItem,
                                         QGraphicsPolygonItem,
                                         QGraphicsSimpleTextItem)


from qt_dotgraph.graph_item import GraphItem


class EdgeItem(GraphItem):

    _qt_pen_styles = {
        'dashed': Qt.DashLine,
        'dotted': Qt.DotLine,
        'solid': Qt.SolidLine,
    }

    def __init__(
            self, highlight_level, spline, label_center, label, from_node, to_node,
            parent=None, penwidth=1, edge_color=None, style='solid',
            edgetooltip=None):
        super(EdgeItem, self).__init__(highlight_level, parent)

        self.from_node = from_node
        self.from_node.add_outgoing_edge(self)
        self.to_node = to_node
        self.to_node.add_incoming_edge(self)

        self._default_edge_color = self._COLOR_BLACK
        if edge_color is not None:
            self._default_edge_color = edge_color

        self._default_text_color = self._COLOR_BLACK
        self._default_color = self._COLOR_BLACK
        self._text_brush = QBrush(self._default_color)
        self._shape_brush = QBrush(self._default_color)
        if style in ['dashed', 'dotted']:
            self._shape_brush = QBrush(Qt.transparent)
        self._label_pen = QPen()
        self._label_pen.setColor(self._default_text_color)
        self._label_pen.setJoinStyle(Qt.RoundJoin)
        self._edge_pen = QPen(self._label_pen)
        self._edge_pen.setWidth(penwidth)
        self._edge_pen.setColor(self._default_edge_color)
        self._edge_pen.setStyle(self._qt_pen_styles.get(style, Qt.SolidLine))

        self._sibling_edges = set()

        self._label = None
        if label is not None:
            self._label = QGraphicsSimpleTextItem(label)
            self._label.setFont(GraphItem._LABEL_FONT)
            label_rect = self._label.boundingRect()
            label_rect.moveCenter(label_center)
            self._label.setPos(label_rect.x(), label_rect.y())
            self._label.hoverEnterEvent = self._handle_hoverEnterEvent
            self._label.hoverLeaveEvent = self._handle_hoverLeaveEvent
            self._label.setAcceptHoverEvents(True)

        # spline specification according to
        # http://www.graphviz.org/doc/info/attrs.html#k:splineType
        coordinates = spline.split(' ')
        # extract optional end_point
        end_point = None
        if (coordinates[0].startswith('e,')):
            parts = coordinates.pop(0)[2:].split(',')
            end_point = QPointF(float(parts[0]), -float(parts[1]))
        # extract optional start_point
        if (coordinates[0].startswith('s,')):
            parts = coordinates.pop(0).split(',')

        # first point
        parts = coordinates.pop(0).split(',')
        point = QPointF(float(parts[0]), -float(parts[1]))
        path = QPainterPath(point)

        while len(coordinates) > 2:
            # extract triple of points for a cubic spline
            parts = coordinates.pop(0).split(',')
            point1 = QPointF(float(parts[0]), -float(parts[1]))
            parts = coordinates.pop(0).split(',')
            point2 = QPointF(float(parts[0]), -float(parts[1]))
            parts = coordinates.pop(0).split(',')
            point3 = QPointF(float(parts[0]), -float(parts[1]))
            path.cubicTo(point1, point2, point3)

        self._arrow = None
        if end_point is not None:
            # draw arrow
            self._arrow = QGraphicsPolygonItem()
            polygon = QPolygonF()
            polygon.append(point3)
            offset = QPointF(end_point - point3)
            corner1 = QPointF(-offset.y(), offset.x()) * 0.35
            corner2 = QPointF(offset.y(), -offset.x()) * 0.35
            polygon.append(point3 + corner1)
            polygon.append(end_point)
            polygon.append(point3 + corner2)
            self._arrow.setPolygon(polygon)
            self._arrow.hoverEnterEvent = self._handle_hoverEnterEvent
            self._arrow.hoverLeaveEvent = self._handle_hoverLeaveEvent
            self._arrow.setAcceptHoverEvents(True)

        self._edgetooltip = None
        if edgetooltip is not None:
            edgetooltip = edgetooltip.replace('\\n', '<br/>')
            self.setToolTip(edgetooltip)
            self._edgetooltip = edgetooltip

        self._path = QGraphicsPathItem(parent)
        self._path.setPath(path)
        self.addToGroup(self._path)

        self.set_node_color()
        self.set_label_color()

    def add_to_scene(self, scene):
        scene.addItem(self)
        if self._label is not None:
            scene.addItem(self._label)
        if self._arrow is not None:
            scene.addItem(self._arrow)

    def setToolTip(self, tool_tip):
        super(EdgeItem, self).setToolTip(tool_tip)
        if self._label is not None:
            self._label.setToolTip(tool_tip)
        if self._arrow is not None and self._edgetooltip is None:
            self._arrow.setToolTip(tool_tip)

    def add_sibling_edge(self, edge):
        self._sibling_edges.add(edge)

    def set_node_color(self, color=None):
        if color is None:
            self._label_pen.setColor(self._default_text_color)
            self._text_brush.setColor(self._default_color)
            if self._shape_brush.isOpaque():
                self._shape_brush.setColor(self._default_edge_color)
            self._edge_pen.setColor(self._default_edge_color)
        else:
            self._label_pen.setColor(color)
            self._text_brush.setColor(color)
            if self._shape_brush.isOpaque():
                self._shape_brush.setColor(color)
            self._edge_pen.setColor(color)

        self._path.setPen(self._edge_pen)
        if self._arrow is not None:
            self._arrow.setBrush(self._shape_brush)
            self._arrow.setPen(self._edge_pen)

    def set_label_color(self, color=None):
        if color is None:
            self._label_pen.setColor(self._default_text_color)
        else:
            self._label_pen.setColor(color)

        if self._label is not None:
            self._label.setBrush(self._text_brush)
            self._label.setPen(self._label_pen)

    def _handle_hoverEnterEvent(self, event):
        # hovered edge item in red
        self.set_node_color(self._COLOR_RED)
        self.set_label_color(self._COLOR_RED)

        if self._highlight_level > 1:
            if self.from_node != self.to_node:
                # from-node in blue
                self.from_node.set_node_color(self._COLOR_BLUE)
                # to-node in green
                self.to_node.set_node_color(self._COLOR_GREEN)
            else:
                # from-node/in-node in teal
                self.from_node.set_node_color(self._COLOR_TEAL)
                self.to_node.set_node_color(self._COLOR_TEAL)
        if self._highlight_level > 2:
            # sibling edges in orange
            for sibling_edge in self._sibling_edges:
                sibling_edge.set_node_color(self._COLOR_ORANGE)

    def _handle_hoverLeaveEvent(self, event):
        self.set_node_color()
        self.set_label_color()
        if self._highlight_level > 1:
            self.from_node.set_node_color()
            self.to_node.set_node_color()
        if self._highlight_level > 2:
            for sibling_edge in self._sibling_edges:
                sibling_edge.set_node_color()
