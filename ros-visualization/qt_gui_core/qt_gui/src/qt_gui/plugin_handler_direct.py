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

import traceback

from python_qt_binding.QtCore import qCritical, QEvent, QObject, qWarning, Slot

from qt_gui.plugin_context import PluginContext
from qt_gui.plugin_handler import PluginHandler


class PluginHandlerDirect(PluginHandler):
    """Handler for directly passing invocations between the framework and one `Plugin` instance."""

    def __init__(self, parent, main_window, instance_id,
                 application_context, container_manager, argv):
        super(PluginHandlerDirect, self).__init__(
            parent, main_window, instance_id, application_context, container_manager, argv)
        self.setObjectName('PluginHandlerDirect')
        self._context = None
        self._plugin = None

    def load(self, plugin_provider, callback=None):
        self._context = PluginContext(self)
        super(PluginHandlerDirect, self).load(plugin_provider, callback)

    def _load(self):
        self._plugin = self._plugin_provider.load(self._instance_id.plugin_id, self._context)
        if hasattr(self._plugin, 'has_configuration'):
            self._plugin_has_configuration = self._plugin.has_configuration()
        else:
            self._plugin_has_configuration = hasattr(self._plugin, 'trigger_configuration')
        self._update_title_bars()
        self._emit_load_completed()

    def _emit_load_completed(self, exception=None):
        if exception is None and hasattr(self._plugin, 'installEventFilter'):
            # emit close_signal when deferred delete event for plugin is received
            self._plugin.installEventFilter(self)
        super(PluginHandlerDirect, self)._emit_load_completed(exception)

    def eventFilter(self, watched, event):
        if event.type() == QEvent.DeferredDelete:
            # TOOD: check if ignore() is necessary
            event.ignore()
            self.close_signal.emit(str(self._instance_id))
            return True
        return QObject.eventFilter(self, watched, event)

    def shutdown_plugin(self, callback):
        if hasattr(self._plugin, 'removeEventFilter'):
            self._plugin.removeEventFilter(self)
        super(PluginHandlerDirect, self).shutdown_plugin(callback)
        if hasattr(self._plugin, 'deleteLater'):
            self._plugin.deleteLater()

    def _shutdown_plugin(self):
        if hasattr(self._plugin, 'shutdown_plugin'):
            try:
                self._plugin.shutdown_plugin()
            except Exception:
                qCritical(
                    'PluginHandlerDirect._shutdown_plugin() plugin "%s" raised an '
                    'exception:\n%s' % (str(self._instance_id), traceback.format_exc()))
        self.emit_shutdown_plugin_completed()

    def _delete_widget(self, widget):
        # only delete widgets which are not at the same time the plugin
        if widget != self._plugin:
            super(PluginHandlerDirect, self)._delete_widget(widget)

    def _unload(self):
        self._plugin_provider.unload(self._plugin)
        self._plugin = None
        self._emit_unload_completed()

    def _save_settings(self, plugin_settings, instance_settings):
        if hasattr(self._plugin, 'save_settings'):
            plugin_settings_plugin = plugin_settings.get_settings('plugin')
            instance_settings_plugin = instance_settings.get_settings('plugin')
            try:
                self._plugin.save_settings(plugin_settings_plugin, instance_settings_plugin)
            except Exception:
                qCritical(
                    'PluginHandlerDirect._save_settings() plugin "%s" raised an exception:\n%s' %
                    (str(self._instance_id), traceback.format_exc()))
        self.emit_save_settings_completed()

    def _restore_settings(self, plugin_settings, instance_settings):
        if hasattr(self._plugin, 'restore_settings'):
            plugin_settings_plugin = plugin_settings.get_settings('plugin')
            instance_settings_plugin = instance_settings.get_settings('plugin')
            try:
                self._plugin.restore_settings(plugin_settings_plugin, instance_settings_plugin)
            except Exception:
                qCritical(
                    'PluginHandlerDirect._restore_settings() plugin "%s" raised an '
                    'exception:\n%s' % (str(self._instance_id), traceback.format_exc()))
        self.emit_restore_settings_completed()

    # pointer to QWidget must be used for PySide to work (at least with 1.0.1)
    @Slot('QWidget*')
    def add_widget(self, widget):
        if widget in self._widgets:
            qWarning('PluginHandlerDirect.add_widget() widget "%s" already added' %
                     widget.objectName())
            return
        dock_widget = self._create_dock_widget()
        self._add_dock_widget(dock_widget, widget)

    # pointer to QToolBar must be used for PySide to work (at least with 1.0.1)
    @Slot('QToolBar*')
    def add_toolbar(self, toolbar):
        if toolbar in self._toolbars:
            qWarning('PluginHandlerDirect.add_toolbar() toolbar "%s" already added' %
                     toolbar.objectName())
            return
        self._add_toolbar(toolbar)

    @Slot()
    def close_plugin(self):
        # only non-standalone plugins are closable
        if self._application_context.options.standalone_plugin is None:
            self._emit_close_plugin()

    @Slot()
    def reload_plugin(self):
        self._emit_reload_signal()
