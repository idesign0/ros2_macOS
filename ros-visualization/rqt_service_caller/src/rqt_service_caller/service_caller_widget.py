#!/usr/bin/env python

# Copyright (c) 2011, Dorian Scholz, TU Darmstadt
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

from __future__ import division

import math
import os
import random
import time

from ament_index_python.resources import get_resource

import importlib

from python_qt_binding import loadUi
from python_qt_binding.QtCore import Qt, Slot, qWarning
from python_qt_binding.QtGui import QIcon
from python_qt_binding.QtWidgets import QMenu, QTreeWidgetItem, QWidget

import rclpy

from rqt_py_common.extended_combo_box import ExtendedComboBox
from rqt_py_common.message_helpers import get_service_class, get_message_class, SRV_MODE
from rqt_py_common.topic_helpers import is_primitive_type, get_type_class

class ServiceCallerWidget(QWidget):
    column_names = ['service', 'type', 'expression']

    def __init__(self, node):
        super(ServiceCallerWidget, self).__init__()
        self.setObjectName('ServiceCallerWidget')
        self._node = node

        # create context for the expression eval statement
        self._eval_locals = {}
        for module in (math, random, time):
            self._eval_locals.update(module.__dict__)
        del self._eval_locals['__name__']
        del self._eval_locals['__doc__']

        pkg_name = 'rqt_service_caller'
        _, package_path = get_resource('packages', pkg_name)
        ui_file = os.path.join(
            package_path, 'share', pkg_name, 'resource', 'ServiceCaller.ui')

        loadUi(ui_file, self, {'ExtendedComboBox': ExtendedComboBox})

        self.refresh_services_button.setIcon(QIcon.fromTheme('view-refresh'))
        self.call_service_button.setIcon(QIcon.fromTheme('call-start'))

        self._column_index = {}
        for column_name in self.column_names:
            self._column_index[column_name] = len(self._column_index)

        self._service_info = None
        self.on_refresh_services_button_clicked()

        self.request_tree_widget.itemChanged.connect(self.request_tree_widget_itemChanged)

    def save_settings(self, plugin_settings, instance_settings):
        instance_settings.set_value('current_service_name', self._service_info['service_name'])
        instance_settings.set_value('splitter_orientation', self.splitter.orientation())

    def restore_settings(self, plugin_settings, instance_settings):
        current_service_name = instance_settings.value('current_service_name', None)
        if current_service_name:
            current_service_index = self.service_combo_box.findData(
                current_service_name, Qt.DisplayRole)
            if current_service_index != -1:
                self.service_combo_box.setCurrentIndex(current_service_index)

        if int(instance_settings.value('splitter_orientation', Qt.Vertical)) == int(Qt.Vertical):
            self.splitter.setOrientation(Qt.Vertical)
        else:
            self.splitter.setOrientation(Qt.Horizontal)

    def trigger_configuration(self):
        new_orientation = \
            Qt.Vertical if self.splitter.orientation() == Qt.Horizontal else Qt.Horizontal
        self.splitter.setOrientation(new_orientation)

    @Slot()
    def on_refresh_services_button_clicked(self):
        service_names_and_types = self._node.get_service_names_and_types()
        self._services = {}
        for service_name, service_types in service_names_and_types:
            if len(service_types) > 1:
                qWarning(
                    'ServiceCallerWidget.on_refresh_services_button_clicked():'
                    'Service {} has multiple service types available: {}.'.format(
                        service_name, service_types))
                qWarning(
                    'ServiceCallerWidget.on_refresh_services_button_clicked(): '
                    'using the first option {}'.format(service_types[0]))

            # Skip service files that are not using the "Service mode",
            # e.g. service types used to implement actions.
            service_name_tokens = service_types[0].split('/')
            if len(service_name_tokens) == 3 and service_name_tokens[1] != SRV_MODE:
                continue
            service_class = get_service_class(service_types[0])
            if service_class is None:
                qWarning(
                    'ServiceCaller.on_refresh_services_button_clicked(): '
                    'could not get class %s of service %s. Skipping it.' % (
                        service_types[0], service_name))
            else:
                self._services[service_name] = service_types[0]

        self.service_combo_box.clear()
        self.service_combo_box.addItems(sorted(self._services.keys()))

    @Slot(str)
    def on_service_combo_box_currentIndexChanged(self, service_name):
        self.request_tree_widget.clear()
        self.response_tree_widget.clear()
        service_name = str(service_name)
        if not service_name:
            return

        self._service_info = {}
        self._service_info['service_name'] = service_name
        self._service_info['service_class_name'] = self._services[service_name]

        service_class = get_service_class(self._service_info['service_class_name'])
        assert service_class, 'Could not find class {} for service: {}'.format(
            self._services[service_name], service_name)

        self._service_info['service_class'] = service_class
        self._service_info['expressions'] = {}
        self._service_info['counter'] = 0

        # recursively create widget items for the service request's slots
        request_class = self._service_info['service_class'].Request
        top_level_item = self._recursive_create_widget_items(
            None, service_name, self._service_info['service_class_name'], request_class())

        # add top level item to tree widget
        self.request_tree_widget.addTopLevelItem(top_level_item)

        # resize columns
        self.request_tree_widget.expandAll()
        for i in range(self.request_tree_widget.columnCount()):
            self.request_tree_widget.resizeColumnToContents(i)

    def _recursive_create_widget_items(self, parent, topic_name, type_name, message, is_editable=True):
        item = QTreeWidgetItem(parent)
        if is_editable:
            item.setFlags(item.flags() | Qt.ItemIsEditable)
        else:
            item.setFlags(item.flags() & (~Qt.ItemIsEditable))

        if parent is None:
            # show full topic name with preceding namespace on toplevel item
            topic_text = topic_name
        else:
            topic_text = topic_name.split('/')[-1]

        item.setText(self._column_index['service'], topic_text)
        item.setText(self._column_index['type'], type_name)

        item.setData(0, Qt.UserRole, topic_name)

        if hasattr(message, 'get_fields_and_field_types'):
            for slot_name, type_name in message.get_fields_and_field_types().items():
                self._recursive_create_widget_items(
                    item, topic_name + '/' + slot_name, type_name,
                    getattr(message, slot_name), is_editable)

        elif type(message) in (list, tuple) and (len(message) > 0) and \
                hasattr(message[0], 'get_fields_and_field_types'):
            type_name = type_name.split('[', 1)[0]
            for index, slot in enumerate(message):
                self._recursive_create_widget_items(
                    item, topic_name + '[%d]' % index, type_name, slot, is_editable)

        else:
            item.setText(self._column_index['expression'], repr(message))

        return item

    @Slot('QTreeWidgetItem*', int)
    def request_tree_widget_itemChanged(self, item, column):
        column_name = self.column_names[column]
        new_value = str(item.text(column))
        # qDebug(
        #   'ServiceCaller.request_tree_widget_itemChanged(): %s : %s' %
        #   (column_name, new_value))

        if column_name == 'expression':
            topic_name = str(item.data(0, Qt.UserRole))
            if len(new_value):
                self._service_info['expressions'][topic_name] = new_value
            elif topic_name in self._service_info['expressions']:
                self._service_info['expressions'].pop(topic_name)

            # qDebug(
            #   'ServiceCaller.request_tree_widget_itemChanged(): %s expression: %s' %
            #   (topic_name, new_value))

    def fill_message_slots(self, message, topic_name, expressions, counter):
        # qDebug(
        #     'fill_message_slots('
        #     '\n\tmessage={}, \n\ttopic_name={}, \n\texpressions={}, \n\tcounter={})'.format(
        #         message, topic_name, expressions, counter))
        if type(message) in (list, set):
            for i, msg in enumerate(message):
                slot_key = topic_name + '[{}]'.format(i)
                if slot_key not in expressions:
                    self.fill_message_slots(msg, slot_key, expressions, counter)
                    continue

                expression = expressions[slot_key]
                if len(expression) == 0:
                    continue

                self._eval_locals['i'] = counter
                slot_type_class = type(msg)
                is_array = False
                if slot_type_class in (list, tuple):
                    is_array = True
                value = self._evaluate_expression(expression, None, is_array)
                if value is not None:
                    message[i] = value

        elif hasattr(message, 'get_fields_and_field_types'):
            for slot_name, slot_type in message.get_fields_and_field_types().items():
                slot_key = topic_name + '/' + slot_name

                # if no expression exists for this slot_key, continue with it's child slots
                if slot_key not in expressions:
                    self.fill_message_slots(
                        getattr(message, slot_name), slot_key, expressions, counter)
                    continue

                expression = expressions[slot_key]
                if len(expression) == 0:
                    continue

                self._eval_locals['i'] = counter
                slot_type_class = None

                slot_type_no_array = slot_type.split('[', 1)[0]
                is_array = slot_type.find('[') >= 0
                if is_primitive_type(slot_type_no_array):
                    slot_type_class = get_type_class(slot_type_no_array)
                else:
                    slot_type_class = get_message_class(slot_type_no_array)
                value = self._evaluate_expression(expression, slot_type_class, is_array)
                if value is not None:
                    try:
                        setattr(message, slot_name, value)
                    except AssertionError as e:
                        qWarning(
                            'Failed to set {} to {}\n\t{}'.format(slot_name, value, e.__str__()))

    def _process_msg_expression(self, expression):
        """
        Checks if expression matches the format <package_name>.msg.<str2>

        If expression matches that format then we attempt to import <package_name>
        and store it in self._eval_locals[<package_name>] for use with eval.
        """
        tokens = expression.lstrip('[').split('.', 2)
        if len(tokens) == 3 and tokens[1] == 'msg':
            try:
                module = importlib.import_module(tokens[0])
                self._eval_locals[tokens[0]] = module
            except ModuleNotFoundError:
                qWarning(
                    'ServiceCallerWidget._process_msg_expression failed to import: {}.'.format(
                        tokens[0] + '.msg'))

    def _evaluate_expression(self, expression, slot_type=None, is_array=False):
        successful_eval = True
        successful_conversion = True
        self._process_msg_expression(expression)

        try:
            # try to evaluate expression
            value = eval(expression, {}, self._eval_locals)
        except Exception:
            # just use expression-string as value
            value = expression
            successful_eval = False

        if is_array:
            if not type(value) is list:
                value = list(value)
            if slot_type and len(value) > 0 and type(value[0]) is not slot_type:
                for i, v in enumerate(value):
                    try:
                        # try to convert value to right type
                        value[i] = slot_type(v)
                    except Exception:
                        successful_conversion = False
        else:
            if slot_type and type(value) != slot_type:
                try:
                    # try to convert value to right type
                    value = slot_type(value)
                except Exception:
                    successful_conversion = False

        if successful_conversion:
            return value

        elif successful_eval:
            qWarning(
                'ServiceCaller._evaluate_expression(): '
                'can not convert expression to slot type: %s -> %s' %
                (type(value), slot_type))
        else:
            qWarning('ServiceCaller._evaluate_expression(): failed to evaluate expression: %s' %
                     (expression))

        return None

    @Slot()
    def on_call_service_button_clicked(self):
        current_services = dict(self._node.get_service_names_and_types())
        if self._service_info['service_name'] not in current_services:
            qWarning('Service "{}" is no longer available. Refresh the list of services'.format(
                     self._service_info['service_name']))
            return

        self.response_tree_widget.clear()

        request = self._service_info['service_class'].Request()
        self.fill_message_slots(
            request, self._service_info['service_name'], self._service_info['expressions'],
            self._service_info['counter'])
        cli = self._node.create_client(
            self._service_info['service_class'],  self._service_info['service_name'])

        future = cli.call_async(request)
        while rclpy.ok() and not future.done():
            pass

        if future.result() is not None:
            response = future.result()
            top_level_item = self._recursive_create_widget_items(
                None, '/', self._service_info['service_class_name'] + '.Response',
                response, is_editable=False)
        else:
            qWarning('ServiceCaller.on_call_service_button_clicked(): request:\n%r' % (request))
            qWarning(
                'ServiceCaller.on_call_service_button_clicked(): error calling service "%s".' %
                (self._service_info['service_name']))
            top_level_item = QTreeWidgetItem()
            top_level_item.setText(self._column_index['service'], 'ERROR')
            top_level_item.setText(self._column_index['type'], 'rospy.ServiceException')
            top_level_item.setText(self._column_index['expression'], '')

        self._node.destroy_client(cli)

        self.response_tree_widget.addTopLevelItem(top_level_item)
        # resize columns
        self.response_tree_widget.expandAll()
        for i in range(self.response_tree_widget.columnCount()):
            self.response_tree_widget.resizeColumnToContents(i)

    @Slot('QPoint')
    def on_request_tree_widget_customContextMenuRequested(self, pos):
        self._show_context_menu(
            self.request_tree_widget.itemAt(pos), self.request_tree_widget.mapToGlobal(pos))

    @Slot('QPoint')
    def on_response_tree_widget_customContextMenuRequested(self, pos):
        self._show_context_menu(
            self.response_tree_widget.itemAt(pos), self.response_tree_widget.mapToGlobal(pos))

    def _show_context_menu(self, item, global_pos):
        if item is None:
            return

        # show context menu
        menu = QMenu(self)
        action_item_expand = menu.addAction(QIcon.fromTheme('zoom-in'), "Expand All Children")
        action_item_collapse = menu.addAction(QIcon.fromTheme('zoom-out'), "Collapse All Children")
        action = menu.exec_(global_pos)

        # evaluate user action
        if action in (action_item_expand, action_item_collapse):
            expanded = (action is action_item_expand)

            def recursive_set_expanded(item):
                item.setExpanded(expanded)
                for index in range(item.childCount()):
                    recursive_set_expanded(item.child(index))
            recursive_set_expanded(item)
