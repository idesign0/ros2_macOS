# Copyright (c) 2011, Dirk Thomas, Dorian Scholz, TU Darmstadt
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

from python_qt_binding.QtCore import QObject, QSignalMapper, Signal, Slot
from python_qt_binding.QtWidgets import QAction, QMenu

from qt_gui.icon_loader import get_icon
from qt_gui.menu_manager import MenuManager
from qt_gui.plugin_instance_id import PluginInstanceId


class PluginMenu(QObject):
    """Menu of available plugins to load and running plugin instances to unload."""

    load_plugin_signal = Signal(str)
    unload_plugin_signal = Signal(str)

    def __init__(self, menu_bar, plugin_manager):
        super(PluginMenu, self).__init__()
        self.setObjectName('PluginMenu')

        plugin_menu = menu_bar.addMenu(menu_bar.tr('&Plugins'))
        running_menu = menu_bar.addMenu(menu_bar.tr('&Running'))
        self._plugin_menu_manager = MenuManager(plugin_menu)
        self._plugin_mapper = QSignalMapper(plugin_menu)
        self._plugin_mapper.mapped[str].connect(self.load_plugin_signal)
        self._running_menu_manager = MenuManager(running_menu)
        action = QAction(
            ' Hidden action to work around QTBUG-52582', self._running_menu_manager.menu)
        action.setVisible(False)
        self._running_menu_manager.add_item(action)
        self._running_mapper = QSignalMapper(running_menu)
        self._running_mapper.mapped[str].connect(self.unload_plugin_signal)

        self._instances = {}

    def add_plugin(self, plugin_descriptor):
        base_path = plugin_descriptor.attributes().get('plugin_path')

        menu_manager = self._plugin_menu_manager
        # create submenus
        for group in plugin_descriptor.groups():
            label = group['label']
            if menu_manager.contains_menu(label):
                submenu = menu_manager.get_menu(label)
            else:
                submenu = QMenu(label, menu_manager.menu)
                menu_action = submenu.menuAction()
                self._enrich_action(menu_action, group, base_path)
                menu_manager.add_item(submenu)
            menu_manager = MenuManager(submenu)
        # create action
        action_attributes = plugin_descriptor.action_attributes()
        action = QAction(action_attributes['label'], menu_manager.menu)
        self._enrich_action(action, action_attributes, base_path)

        self._plugin_mapper.setMapping(action, plugin_descriptor.plugin_id())
        action.triggered.connect(self._plugin_mapper.map)

        not_available = plugin_descriptor.attributes().get('not_available')
        if not_available:
            action.setEnabled(False)
            action.setStatusTip(self.tr('Plugin is not available: %s') % not_available)

        # add action to menu
        menu_manager.add_item(action)

    def add_plugin_prefix(self, plugin_descriptor):
        action_attributes = plugin_descriptor.action_attributes()
        action = QAction(action_attributes['label'], self._plugin_menu_manager.menu)
        self._enrich_action(action, action_attributes)
        self._plugin_mapper.setMapping(action, plugin_descriptor.plugin_id())
        action.triggered.connect(self._plugin_mapper.map)
        self._plugin_menu_manager.add_prefix(action)

    def add_instance(self, plugin_descriptor, instance_id):
        action_attributes = plugin_descriptor.action_attributes()
        action = QAction(self._get_instance_label(
            str(instance_id)), self._running_menu_manager.menu)
        base_path = plugin_descriptor.attributes().get('plugin_path')
        self._enrich_action(action, action_attributes, base_path)

        self._running_mapper.setMapping(action, str(instance_id))
        action.triggered.connect(self._running_mapper.map)

        self._running_menu_manager.add_item(action)
        self._instances[instance_id] = action

    def remove_instance(self, instance_id):
        action = self._instances[instance_id]
        self._running_mapper.removeMappings(action)
        self._running_menu_manager.remove_item(action)

    @Slot(str, str)
    def update_plugin_instance_label(self, instance_id_str, label):
        instance_id = PluginInstanceId(instance_id=instance_id_str)
        action = self._instances[instance_id]
        action.setText(self._get_instance_label(label))

    def _get_instance_label(self, label):
        return self.tr('Close:') + ' ' + label

    def _enrich_action(self, action, action_attributes, base_path=None):
        if 'icon' in action_attributes and action_attributes['icon'] is not None:
            icon = get_icon(
                action_attributes['icon'], action_attributes.get('icontype', None), base_path)
            action.setIcon(icon)

        if 'statustip' in action_attributes:
            action.setStatusTip(action_attributes['statustip'])
