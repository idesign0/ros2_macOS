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

from python_qt_binding.QtCore import qDebug, QObject, Qt

from qt_gui.dock_widget import DockWidget
from qt_gui.plugin_descriptor import PluginDescriptor
from qt_gui.reparent_event import ReparentEvent


class ContainerManager(QObject):
    """Manager of `DockWidgetContainer`s enabling reparenting to stored parent."""

    def __init__(self, root_main_window, parent=None):
        super(ContainerManager, self).__init__(parent)
        self._root_main_window = root_main_window
        self._container_descriptor = PluginDescriptor('__DockWidgetContainer')
        self._container_descriptor.set_action_attributes(
            self.tr('Container'), self.tr('Container for other dock widgets'),
            'folder-new', 'theme')

        self._containers = {}

    def get_root_main_window(self):
        return self._root_main_window

    def get_container_descriptor(self):
        return self._container_descriptor

    def add_to_plugin_menu(self, plugin_menu):
        plugin_menu.add_plugin_prefix(self._container_descriptor)

    def add_container(self, container):
        self._containers[container.serial_number()] = container

    def remove_container(self, container):
        del self._containers[container.serial_number()]

    def get_container(self, serial_number):
        if serial_number in self._containers:
            return self._containers[serial_number]
        return None

    def get_containers(self):
        return self._containers.values()

    def move_container_children_to_parent(self, container):
        floating = container.isFloating()
        for child in container.main_window.children():
            if isinstance(child, DockWidget):
                area = container.main_window.dockWidgetArea(child)
                container.parent().addDockWidget(area, child)
                if floating:
                    child.setFloating(floating)

    def get_container_of_dock_widget(self, dock_widget):
        for container in self._containers.values():
            for child in container.main_window.children():
                if child == dock_widget:
                    return container
        return None

    def restore_state_of_containers(self):
        for container in self._containers.values():
            container.restore_state()

    def event(self, e):
        if e.type() == ReparentEvent.reparent_event_type:
            qDebug('ContainerManager.event() reparent event: new parent=%s' %
                   e.new_parent.objectName())
            floating = e.dock_widget.isFloating()
            pos = e.dock_widget.pos()
            e.new_parent.addDockWidget(Qt.BottomDockWidgetArea, e.dock_widget)
            if floating:
                e.dock_widget.setFloating(floating)
                e.dock_widget.move(pos)
            e.accept()
            return True
        return super(ContainerManager, self).event(e)
