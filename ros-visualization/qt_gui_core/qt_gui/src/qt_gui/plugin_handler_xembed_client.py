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

import os
import traceback

from dbus import Interface
from dbus.connection import Connection
from python_qt_binding.QtCore import (QByteArray, qCritical, QDataStream,
                                      qDebug, QIODevice, Qt, qWarning, Slot)
from python_qt_binding.QtGui import QVBoxLayout, QX11EmbedWidget

from qt_gui.plugin_handler_direct import PluginHandlerDirect
from qt_gui.settings import Settings
from qt_gui.window_changed_signaler import WindowChangedSignaler


class PluginHandlerXEmbedClient(PluginHandlerDirect):
    """
    Client part of the `PluginHandlerXEmbed`.

    Client utilizes the `PluginHandlerDBusService` of the `PluginHandlerXEmbedContainer` through a
    peer-to-peer DBus connection.
    """

    def __init__(self, parent, main_window, instance_id,
                 application_context, container_manager, argv, dbus_object_path):
        super(PluginHandlerXEmbedClient, self).__init__(
            parent, main_window, instance_id, application_context, container_manager, argv)
        self.setObjectName('PluginHandlerXEmbedClient')
        self._dbus_object_path = dbus_object_path
        self._remote_container = None
        self._remote_plugin_settings = None
        self._remote_instance_settings = None
        # mapping of added widgets to their embed widget and WindowChangedSignaler
        self._embed_widgets = {}

    def _load(self):
        conn = Connection(self._application_context.options.embed_plugin_address)
        proxy = conn.get_object(None, self._dbus_object_path)
        self._remote_container = Interface(proxy, 'org.ros.qt_gui.PluginHandlerContainer')
        self._remote_container.connect_to_signal('shutdown_plugin', self._shutdown_plugin)
        self._remote_container.connect_to_signal('save_settings', self._save_settings_from_remote)
        self._remote_container.connect_to_signal(
            'restore_settings', self._restore_settings_from_remote)
        self._remote_container.connect_to_signal(
            'trigger_configuration', self._trigger_configuration)
        self._remote_container.connect_to_signal(
            'toolbar_orientation_changed', self._toolbar_orientation_changed)

        proxy = conn.get_object(None, self._dbus_object_path + '/plugin')
        self._remote_plugin_settings = Interface(proxy, 'org.ros.qt_gui.Settings')
        proxy = conn.get_object(None, self._dbus_object_path + '/instance')
        self._remote_instance_settings = Interface(proxy, 'org.ros.qt_gui.Settings')

        super(PluginHandlerXEmbedClient, self)._load()

    def _emit_load_completed(self, exception=None):
        # signal failed loading before emitting signal, as it might not be possible afterwards
        if exception is not None:
            self._remote_container.load_completed(False, False)
        super(PluginHandlerXEmbedClient, self)._emit_load_completed(exception)
        # signal successful loading after emitting signal, for better message order
        if exception is None:
            self._remote_container.load_completed(True, self._plugin_has_configuration)

    def shutdown_plugin(self, callback):
        # this method should never be called for embedded clients
        assert(False)

    def emit_shutdown_plugin_completed(self):
        self._remote_container.shutdown_plugin_completed()

    def save_settings(self, plugin_settings, instance_settings, callback=None):
        # this method should never be called for embedded clients
        assert(False)

    def _save_settings_from_remote(self):
        qDebug('PluginHandlerXEmbedClient._save_settings_from_remote()')
        try:
            plugin_settings = Settings(self._remote_plugin_settings, '')
            instance_settings = Settings(self._remote_instance_settings, '')
            self._save_settings(plugin_settings, instance_settings)
        except Exception:
            qCritical(
                'PluginHandlerXEmbedClient._save_settings_from_remote() plugin "%s" '
                'raised an exception:\n%s' % (str(self._instance_id), traceback.format_exc()))
            self.emit_save_settings_completed()

    def emit_save_settings_completed(self):
        self._remote_container.save_settings_completed()

    def restore_settings(self, plugin_settings, instance_settings, callback=None):
        # this method should never be called for embedded clients
        assert(False)

    def _restore_settings_from_remote(self):
        qDebug('PluginHandlerXEmbedClient._restore_settings_from_remote()')
        try:
            plugin_settings = Settings(self._remote_plugin_settings, '')
            instance_settings = Settings(self._remote_instance_settings, '')
            self._restore_settings(plugin_settings, instance_settings)
        except Exception:
            qCritical(
                'PluginHandlerXEmbedClient._restore_settings_from_remote() '
                'plugin "%s" raised an exception:\n%s' %
                (str(self._instance_id), traceback.format_exc()))
            self.emit_restore_settings_completed()

    def emit_restore_settings_completed(self):
        self._remote_container.restore_settings_completed()

    # pointer to QWidget must be used for PySide to work (at least with 1.0.1)
    @Slot('QWidget*')
    def add_widget(self, widget):
        if widget in self._embed_widgets:
            qWarning('PluginHandlerXEmbedClient.add_widget() widget "%s" already added' %
                     widget.objectName())
            return
        embed_widget = QX11EmbedWidget()
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(widget)
        embed_widget.setLayout(layout)

        # close embed widget when container is closed
        # TODO necessary?
        # embed_widget.containerClosed.connect(embed_widget.close)

        embed_container_window_id = self._remote_container.embed_widget(
            os.getpid(), widget.objectName())
        embed_widget.embedInto(embed_container_window_id)

        signaler = WindowChangedSignaler(widget, widget)
        signaler.window_icon_changed_signal.connect(self._on_embed_widget_icon_changed)
        signaler.window_title_changed_signal.connect(self._on_embed_widget_title_changed)
        self._embed_widgets[widget] = embed_widget, signaler
        # trigger to update initial window icon and title
        signaler.window_icon_changed_signal.emit(widget)
        signaler.window_title_changed_signal.emit(widget)

        embed_widget.show()

    def _on_embed_widget_icon_changed(self, widget):
        # serialize icon base64-encoded string
        ba = QByteArray()
        s = QDataStream(ba, QIODevice.WriteOnly)
        s << widget.windowIcon()
        icon_str = str(ba.toBase64())
        self._remote_container.update_embedded_widget_icon(widget.objectName(), icon_str)

    def _on_embed_widget_title_changed(self, widget):
        self._remote_container.update_embedded_widget_title(
            widget.objectName(), widget.windowTitle())

    # pointer to QWidget must be used for PySide to work (at least with 1.0.1)
    @Slot('QWidget*')
    def remove_widget(self, widget):
        embed_widget, signaler = self._embed_widgets[widget]
        del self._embed_widgets[widget]
        signaler.window_icon_changed_signal.disconnect(self._on_embed_widget_icon_changed)
        signaler.window_title_changed_signal.disconnect(self._on_embed_widget_title_changed)
        self._remote_container.unembed_widget(widget.objectName())
        # do not delete the widget, only the embed widget
        widget.setParent(None)
        embed_widget.deleteLater()
        # triggering close after last widget and toolbar is closed is handled by the container

    # pointer to QToolBar must be used for PySide to work (at least with 1.0.1)
    @Slot('QToolBar*')
    def add_toolbar(self, toolbar):
        if toolbar in self._embed_widgets:
            qWarning('PluginHandlerXEmbedClient.add_toolbar() toolbar "%s" already added' %
                     toolbar.objectName())
            return
        embed_widget = QX11EmbedWidget()
        layout = QVBoxLayout()
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(toolbar)
        embed_widget.setLayout(layout)

        # close embed widget when container is closed
        # TODO necessary?
        # embed_widget.containerClosed.connect(embed_widget.close)
        def foo():
            print('embed_widget.containerClosed')
        embed_widget.containerClosed.connect(foo)

        embed_container_window_id = self._remote_container.embed_toolbar(
            os.getpid(), toolbar.objectName())
        embed_widget.embedInto(embed_container_window_id)

        self._embed_widgets[toolbar] = embed_widget, None

        embed_widget.show()

    def _toolbar_orientation_changed(self, win_id, is_horizontal):
        for toolbar, (embed_widget, _) in self._embed_widgets.items():
            if embed_widget.containerWinId() == win_id:
                toolbar.setOrientation(Qt.Horizontal if is_horizontal else Qt.Vertical)
                break

    # pointer to QToolBar must be used for PySide to work (at least with 1.0.1)
    @Slot('QToolBar*')
    def remove_toolbar(self, toolbar):
        embed_widget, _ = self._embed_widgets[toolbar]
        del self._embed_widgets[toolbar]
        self._remote_container.unembed_widget(toolbar.objectName())
        # do not delete the toolbar, only the embed widget
        toolbar.setParent(None)
        embed_widget.deleteLater()
        # triggering close after last widget and toolbar is closed is handled by the container

    def _emit_close_plugin(self):
        self._remote_container.close_plugin()
