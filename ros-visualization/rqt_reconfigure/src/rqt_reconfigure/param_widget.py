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
# Author: Isaac Saito

from __future__ import division

import sys

from python_qt_binding.QtCore import QMargins, Signal
from python_qt_binding.QtWidgets import (
    QHBoxLayout, QLabel, QSplitter, QVBoxLayout, QWidget
)

from rqt_reconfigure import logging
from rqt_reconfigure.node_selector_widget import NodeSelectorWidget
from rqt_reconfigure.paramedit_widget import ParameditWidget
from rqt_reconfigure.text_filter import TextFilter
from rqt_reconfigure.text_filter_widget import TextFilterWidget


class ParamWidget(QWidget):
    _TITLE_PLUGIN = 'Parameter Reconfigure'

    # To be connected to PluginContainerWidget
    sig_sysmsg = Signal(str)
    sig_sysprogress = Signal(str)

    # To make selections from CLA
    sig_selected = Signal(str, bool)

    def __init__(self, context, node=None):
        """
        Init param widget.

        This class is intended to be called by rqt plugin framework class.
        Currently (12/12/2012) the whole widget is splitted into 2 panes:
        one on left allows you to choose the node(s) you work on. Right side
        pane lets you work with the parameters associated with the node(s) you
        select on the left.

        (12/27/2012) Despite the pkg name is changed to rqt_reconfigure to
        reflect the available functionality, file & class names remain
        'param', expecting all the parameters will become handle-able.
        """
        super(ParamWidget, self).__init__()
        self.setObjectName(self._TITLE_PLUGIN)
        self.setWindowTitle(self._TITLE_PLUGIN)

        # TODO: .ui file needs to replace the GUI components declaration
        #       below. For unknown reason, referring to another .ui files
        #       from a .ui that is used in this class failed. So for now,
        #       I decided not use .ui in this class.
        #       If someone can tackle this I'd appreciate.
        _hlayout_top = QHBoxLayout(self)
        _hlayout_top.setContentsMargins(QMargins(0, 0, 0, 0))
        self._splitter = QSplitter(self)
        _hlayout_top.addWidget(self._splitter)

        _vlayout_nodesel_widget = QWidget()
        _vlayout_nodesel_side = QVBoxLayout()
        _hlayout_filter_widget = QWidget(self)
        _hlayout_filter = QHBoxLayout()
        self._text_filter = TextFilter()
        self.filter_lineedit = TextFilterWidget(self._text_filter)
        self.filterkey_label = QLabel('&Filter key:')
        self.filterkey_label.setBuddy(self.filter_lineedit)
        _hlayout_filter.addWidget(self.filterkey_label)
        _hlayout_filter.addWidget(self.filter_lineedit)
        _hlayout_filter_widget.setLayout(_hlayout_filter)
        self._nodesel_widget = NodeSelectorWidget(
            self, context, self.sig_sysmsg
        )
        _vlayout_nodesel_side.addWidget(_hlayout_filter_widget)
        _vlayout_nodesel_side.addWidget(self._nodesel_widget)
        _vlayout_nodesel_side.setSpacing(1)
        _vlayout_nodesel_widget.setLayout(_vlayout_nodesel_side)

        self._param_edit_widget = ParameditWidget()

        self._splitter.insertWidget(0, _vlayout_nodesel_widget)
        self._splitter.insertWidget(1, self._param_edit_widget)
        # 1st column, _vlayout_nodesel_widget, to minimize width.
        # 2nd col to keep the possible max width.
        self._splitter.setStretchFactor(0, 0)
        self._splitter.setStretchFactor(1, 1)

        # Signal from paramedit widget to node selector widget.
        self._param_edit_widget.sig_node_disabled_selected.connect(
            self._nodesel_widget.node_deselected
        )
        # Pass name of node to editor widget
        self._nodesel_widget.sig_node_selected.connect(
            self._param_edit_widget.show
        )

        if not node:
            title = self._TITLE_PLUGIN
        else:
            title = self._TITLE_PLUGIN + ' %s' % node
        self.setObjectName(title)

        # Connect filter signal-slots.
        self._text_filter.filter_changed_signal.connect(
            self._filter_key_changed
        )

        # Signal from widget to open a new editor widget
        self.sig_selected.connect(self._nodesel_widget.node_selected)

        self._explicit_nodes_to_select = list(context.argv())

    def shutdown(self):
        # TODO: Needs implemented. Trigger dynamic_reconfigure to unlatch
        #       subscriber.
        pass

    def save_settings(self, plugin_settings, instance_settings):
        instance_settings.set_value('splitter', self._splitter.saveState())
        self.filter_lineedit.save_settings(instance_settings)
        self._nodesel_widget.save_settings(instance_settings)
        instance_settings.set_value(
            'selected_nodes', list(self._param_edit_widget.get_active_grns()))

    def restore_settings(self, plugin_settings, instance_settings):
        if instance_settings.contains('splitter'):
            self._splitter.restoreState(instance_settings.value('splitter'))
        else:
            self._splitter.setSizes([100, 100, 200])
        self.filter_lineedit.restore_settings(instance_settings)
        self._nodesel_widget.restore_settings(instance_settings)

        # Ignore previously open nodes if we were given an explicit list
        if self._explicit_nodes_to_select:
            nodes_to_select = self._explicit_nodes_to_select
            explicit = True
        else:
            nodes_to_select = instance_settings.value('selected_nodes') or []
            explicit = False

        for rn in nodes_to_select:
            if rn in self._nodesel_widget.get_nodeitems():
                self.sig_selected.emit(rn, explicit)
            elif explicit:
                logging.warn(
                    'Could not find a dynamic reconfigure client'
                    " named '{}'".format(str(rn))
                )

    def get_filter_text(self):
        return self.filter_lineedit.text()

    def _filter_key_changed(self):
        self._nodesel_widget.set_filter(self._text_filter)

    # TODO: This method should be integrated into common architecture. I just
    #       can't think of how to do so within current design.
    def emit_sysmsg(self, msg_str):
        self.sig_sysmsg.emit(msg_str)


if __name__ == '__main__':
    # main should be used only for debug purpose.
    # This launches this QWidget as a standalone rqt gui.
    from rqt_gui.main import Main

    main = Main()
    sys.exit(main.main(sys.argv, standalone='rqt_reconfigure'))
