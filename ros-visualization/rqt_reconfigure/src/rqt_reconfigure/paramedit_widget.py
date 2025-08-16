# Copyright (c) 2012, Willow Garage, Inc.
# All rights reserved.
#
# Software License Agreement (BSD License 2.0)
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
#
# Author: Isaac Saito, Ze'ev Klapow

from collections import OrderedDict
import os

from ament_index_python import get_resource
from python_qt_binding import loadUi
from python_qt_binding.QtCore import Signal
from python_qt_binding.QtWidgets import QVBoxLayout, QWidget, QWidgetItem

from rqt_py_common.layout_util import LayoutUtil

from rqt_reconfigure import logging


class ParameditWidget(QWidget):
    """
    This class represents a pane.

    Parameter editor widgets of multiple nodes are shown.
    In rqt_reconfigure, this pane occupies right half of the
    entire visible area.
    """

    # public signal
    sig_node_disabled_selected = Signal(str)

    def __init__(self):
        super(ParameditWidget, self).__init__()

        _, package_path = get_resource('packages', 'rqt_reconfigure')
        ui_file = os.path.join(package_path, 'share', 'rqt_reconfigure',
                               'resource', 'paramedit_pane.ui')
        loadUi(ui_file, self, {'ParameditWidget': ParameditWidget})

        self._param_client_widgets = OrderedDict()

        # Adding the list of Items
        self._vlayout = QVBoxLayout(self.scrollarea_holder_widget)

        # causes error
        # self._set_index_widgets(self.listview, paramitems_dict)

        self.destroyed.connect(self.close)

    def _set_index_widgets(self, view, paramitems_dict):
        i = 0
        for p in paramitems_dict:
            view.setIndexWidget(i, p)
            i += 1

    def show(self, param_client_widget):
        # Callback when user chooses a node.
        node_grn = param_client_widget.get_node_grn()
        logging.debug('ParameditWidget.show'
                      ' str(node_grn)={}'.format(str(node_grn)))

        if node_grn not in self._param_client_widgets:
            self._param_client_widgets[node_grn] = param_client_widget
            self._vlayout.addWidget(param_client_widget)
            param_client_widget.sig_node_disabled_selected.connect(
                self._node_disabled)
        else:  # If there has one already existed, remove it.
            self._remove_node(node_grn)
            # LayoutUtil.clear_layout(self.vlayout)

            """"
             Re-add the rest of existing items to layout.
             for k, v in self._param_client_widgets.items():
                 logging.info('added to layout k={} v={}'.format(k, v))
                 self.vlayout.addWidget(v)
            """

        # Add color to alternate the rim of the widget.
        LayoutUtil.alternate_color(
            self._param_client_widgets.values(),
            [self.palette().window().color().lighter(125),
             self.palette().window().color().darker(125)])

    def close(self):
        for w in self._param_client_widgets:
            w.close()
        self._param_client_widgets.clear()
        self._paramedit_scrollarea.deleteLater()

    def get_active_grns(self):
        return self._param_client_widgets.keys()

    def filter_param(self, filter_key):
        # TODO Pick nodes that match filter_key.
        #  TODO For the nodes that are kept in previous step, call ParamClientWidget.filter_param
        pass

    def _remove_node(self, node_grn):
        try:
            i = list(self._param_client_widgets.keys()).index(node_grn)
        except ValueError:
            """
            ValueError occurring here means that the specified key is not
             found, most likely already removed, which is possible in the
             following situation/sequence:

             Node widget on ParameditWidget removed by clicking disable button
             --> Node deselected on tree widget gets updated
             --> Tree widget detects deselection
             --> Tree widget emits deselection signal, which is captured by
                 ParameditWidget's slot. Thus reaches this method again.
            """
            return

        item = self._vlayout.itemAt(i)
        if isinstance(item, QWidgetItem):
            item.widget().close()
        w = self._param_client_widgets.pop(node_grn)

        logging.debug('popped={} Len of left clients={}'.format(
            w, len(self._param_client_widgets)
        ))

    def _node_disabled(self, node_grn):
        logging.debug('paramedit_w _node_disabled grn={}'.format(node_grn))

        """
         Signal to notify other GUI components (eg. nodes tree pane) that
         a node widget is disabled.
        """
        self.sig_node_disabled_selected.emit(node_grn)

        # Remove the selected node widget from the internal list of nodes.
        self._remove_node(node_grn)
