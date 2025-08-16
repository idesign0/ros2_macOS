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

from qt_gui.dock_widget_container import DockWidgetContainer
from qt_gui.plugin_handler import PluginHandler
from qt_gui.window_changed_signaler import WindowChangedSignaler


class PluginHandlerContainer(PluginHandler):
    """Handler for creating a container."""

    def __init__(self, parent, main_window, instance_id, application_context, container_manager):
        super(PluginHandlerContainer, self).__init__(
            parent, main_window, instance_id, application_context, container_manager)
        self.setObjectName('PluginHandlerContainer')
        self._container = None

    def _load(self):
        self._container = DockWidgetContainer(
            self._container_manager, self._instance_id.serial_number)
        self._container.setObjectName(self._instance_id.tidy_str())
        title = self.tr('Container')
        if self._instance_id.serial_number > 1:
            title += ' (%d)' % self._instance_id.serial_number
        self._container.setWindowTitle(title)
        self._update_dock_widget_features(self._container)
        self._add_dock_widget_to_main_window(self._container)
        self._update_title_bar(self._container, True, True)
        signaler2 = WindowChangedSignaler(self._container, self._container)
        signaler2.hide_signal.connect(self._on_dock_widget_hide)
        signaler2.show_signal.connect(self._on_dock_widget_show)
        signaler2.window_title_changed_signal.connect(self._on_dock_widget_title_changed)
        # trigger to update initial window state
        signaler2.emit_all()
        self._widgets[self._container.main_window] = [self._container, None, signaler2]
        self._container_manager.add_container(self._container)
        self._emit_load_completed()

    def remove_widget(self, widget):
        self._container_manager.move_container_children_to_parent(self._container)
        self._container_manager.remove_container(self._container)
        super(PluginHandlerContainer, self).remove_widget(widget)
        self._container = None

    def _shutdown_plugin(self):
        self.emit_shutdown_plugin_completed()

    def _unload(self):
        self._emit_unload_completed()

    def _save_settings(self, plugin_settings, instance_settings):
        self.emit_save_settings_completed()

    def _restore_settings(self, plugin_settings, instance_settings):
        self.emit_restore_settings_completed()

    def _close_dock_widget(self, dock_widget):
        self._emit_close_plugin()
