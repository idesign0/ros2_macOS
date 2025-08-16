# Copyright (c) 2019 Open Source Robotics Foundation, Inc.
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
# Author: Gonzalo de Pedro

from python_qt_binding.QtCore import QMargins, QSize, Qt, Signal
from python_qt_binding.QtGui import QFont, QIcon
from python_qt_binding.QtWidgets import (QFileDialog, QFormLayout,
                                         QHBoxLayout, QLabel,
                                         QPushButton, QVBoxLayout,
                                         QWidget)

import rclpy

from rqt_reconfigure import logging

from rqt_reconfigure.param_api import create_param_client

"""
 Editor classes that are not explicitly used within this .py file still need
 to be imported. They are invoked implicitly during runtime.
"""

from rqt_reconfigure.param_editors import (BooleanEditor,  # noqa: F401
                                           DoubleEditor, EDITOR_TYPES,
                                           EditorWidget, IntegerEditor,
                                           StringEditor)

import yaml


class ParamClientWidget(QWidget):
    # Represents a widget where users can view and modify ROS params.

    sig_node_disabled_selected = Signal(str)
    sig_node_state_change = Signal(bool)

    def __init__(self, context, node_name):
        """
        Initializaze things.

        :type node_name: str
        """
        super(ParamClientWidget, self).__init__()
        self._node_grn = node_name
        self._toplevel_treenode_name = node_name

        self._editor_widgets = {}

        self._param_client = create_param_client(
            context.node, node_name, self._handle_param_event
        )

        verticalLayout = QVBoxLayout(self)
        verticalLayout.setContentsMargins(QMargins(0, 0, 0, 0))

        widget_nodeheader = QWidget()
        h_layout_nodeheader = QHBoxLayout(widget_nodeheader)
        h_layout_nodeheader.setContentsMargins(QMargins(0, 0, 0, 0))

        nodename_qlabel = QLabel(self)
        font = QFont('Trebuchet MS, Bold')
        font.setUnderline(True)
        font.setBold(True)
        font.setPointSize(10)
        nodename_qlabel.setFont(font)
        nodename_qlabel.setAlignment(Qt.AlignCenter)
        nodename_qlabel.setText(node_name)
        h_layout_nodeheader.addWidget(nodename_qlabel)

        # Button to close a node.
        icon_disable_node = QIcon.fromTheme('window-close')
        bt_disable_node = QPushButton(icon_disable_node, '', self)
        bt_disable_node.setToolTip('Hide this node')
        bt_disable_node_size = QSize(36, 24)
        bt_disable_node.setFixedSize(bt_disable_node_size)
        bt_disable_node.pressed.connect(self._node_disable_bt_clicked)
        h_layout_nodeheader.addWidget(bt_disable_node)

        grid_widget = QWidget(self)
        self.grid = QFormLayout(grid_widget)
        verticalLayout.addWidget(widget_nodeheader)
        verticalLayout.addWidget(grid_widget, 1)
        # Again, these UI operation above needs to happen in .ui file.
        try:
            param_names = self._param_client.list_parameters()
            self.add_editor_widgets(
                self._param_client.get_parameters(param_names),
                self._param_client.describe_parameters(param_names)
            )
        except Exception as e:
            logging.warn('Failed to retrieve parameters from node: ' + str(e))

        # Save and load buttons
        button_widget = QWidget(self)
        button_header = QHBoxLayout(button_widget)
        button_header.setContentsMargins(QMargins(0, 0, 0, 0))

        load_button = QPushButton()
        save_button = QPushButton()

        load_button.setIcon(QIcon.fromTheme('document-open'))
        save_button.setIcon(QIcon.fromTheme('document-save'))

        load_button.clicked[bool].connect(self._handle_load_clicked)
        save_button.clicked[bool].connect(self._handle_save_clicked)

        button_header.addWidget(save_button)
        button_header.addWidget(load_button)

        self.setMinimumWidth(150)

    def get_node_grn(self):
        return self._node_grn

    def _handle_param_event(
        self, new_parameters, changed_parameters, deleted_parameters
    ):
        # TODO: Think about replacing callback architecture with signals.
        if new_parameters:
            try:
                new_descriptors = self._param_client.describe_parameters(
                    names=[p.name for p in new_parameters]
                )
                self.add_editor_widgets(new_parameters, new_descriptors)
            except Exception as e:
                logging.warn('Failed to get information about parameters: ' + str(e))

        if changed_parameters:
            self.update_editor_widgets(changed_parameters)
        if deleted_parameters:
            self.remove_editor_widgets(deleted_parameters)

    def _handle_load_clicked(self):
        filename = QFileDialog.getOpenFileName(
            self, self.tr('Load from File'), '.',
            self.tr('YAML file {.yaml} (*.yaml)'))
        if filename[0] != '':
            self.load_param(filename[0])

    def _handle_save_clicked(self):
        filename = QFileDialog.getSaveFileName(
            self, self.tr('Save parameters to file...'), '.',
            self.tr('YAML files {.yaml} (*.yaml)'))
        if filename[0] != '':
            self.save_param(filename[0])

    def save_param(self, filename):
        with open(filename, 'w') as f:
            try:
                parameters = self._param_client.get_parameters(
                    self._param_client.list_parameters()
                )
                yaml.dump({p.name: p.value for p in parameters}, f)
            except Exception as e:
                logging.warn(
                    "Parameter saving wasn't successful because: " + str(e)
                )

    def load_param(self, filename):
        with open(filename, 'r') as f:
            parameters = [
                rclpy.parameter.Parameter(name=name, value=value)
                for doc in yaml.safe_load_all(f.read())
                for name, value in doc.items()
            ]

        try:
            self._param_client.set_parameters(parameters)
        except Exception as e:
            logging.warn(
                "Parameter loading wasn't successful"
                ' because: {}'.format(e)
            )

    def collect_paramnames(self, config):
        pass

    def remove_editor_widgets(self, parameters):
        for parameter in parameters:
            if parameter.name not in self._editor_widgets:
                continue
            logging.debug('Removing editor widget for {}'.format(
                parameter.name))
            self._editor_widgets[parameter.name].hide(self.grid)
            self._editor_widgets[parameter.name].close()
            del self._editor_widgets[parameter.name]

    def update_editor_widgets(self, parameters):
        for parameter in parameters:
            if parameter.name not in self._editor_widgets:
                continue
            logging.debug('Updating editor widget for {}'.format(
                parameter.name))
            self._editor_widgets[parameter.name].update_local(parameter.value)

    def add_editor_widgets(self, parameters, descriptors):
        for parameter, descriptor in zip(parameters, descriptors):
            if parameter.type_ not in EDITOR_TYPES:
                continue
            logging.debug('Adding editor widget for {}'.format(parameter.name))
            editor_widget = EDITOR_TYPES[parameter.type_](
                self._param_client, parameter, descriptor
            )
            self._editor_widgets[parameter.name] = editor_widget
            editor_widget.display(self.grid)

    def display(self, grid):
        grid.addRow(self)

    def get_treenode_names(self):
        return list(self._editor_widgets.keys())

    def _node_disable_bt_clicked(self):
        logging.debug('param_gs _node_disable_bt_clicked')
        self.sig_node_disabled_selected.emit(self._toplevel_treenode_name)

    def close(self):
        self._param_client.close()

        for w in self._editor_widgets.values():
            w.close()

        self.deleteLater()

    def filter_param(self, filter_key):
        # TODO impl
        pass
