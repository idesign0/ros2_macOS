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

from python_qt_binding.QtCore import qCritical, qDebug, QObject, Qt, qWarning, Signal, Slot
from python_qt_binding.QtWidgets import QDockWidget, QToolBar

from qt_gui.dock_widget import DockWidget
from qt_gui.dock_widget_title_bar import DockWidgetTitleBar
from qt_gui.icon_loader import get_icon
from qt_gui.window_changed_signaler import WindowChangedSignaler


class PluginHandler(QObject):
    """
    Base class for the bidirectional exchange between the framework and one `Plugin` instance.

    It utilizes a `PluginProvider` to load/unload the plugin and provides callbacks for the
    `PluginContext`.
    """

    label_updated = Signal(str, str)
    close_signal = Signal(str)
    reload_signal = Signal(str)
    help_signal = Signal(str)
    _defered_check_close = Signal()

    def __init__(self, parent, main_window, instance_id, application_context, container_manager,
                 argv=None):
        super(PluginHandler, self).__init__(parent)
        self.setObjectName('PluginHandler')

        self._main_window = main_window
        self._instance_id = instance_id
        self._application_context = application_context
        self._container_manager = container_manager
        self._argv = argv if argv else []
        self._minimized_dock_widgets_toolbar = None
        self._plugin_descriptor = None

        self._defered_check_close.connect(self._check_close, Qt.QueuedConnection)
        self._plugin_provider = None
        self.__callback = None
        self.__instance_settings = None

        self._plugin_has_configuration = False

        # mapping of added widgets to their parent dock widget and WindowChangedSignaler
        self._widgets = {}

        self._toolbars = []

    def instance_id(self):
        return self._instance_id

    def argv(self):
        return self._argv

    def set_minimized_dock_widgets_toolbar(self, toolbar):
        self._minimized_dock_widgets_toolbar = toolbar

    def set_plugin_descriptor(self, plugin_descriptor):
        self._plugin_descriptor = plugin_descriptor

    def load(self, plugin_provider, callback=None):
        """
        Load plugin.

        Completion is signaled asynchronously if a callback is passed.
        """
        self._plugin_provider = plugin_provider
        self.__callback = callback
        try:
            self._load()
        except Exception as e:
            self._emit_load_completed(e)

    def _load(self):
        raise NotImplementedError

    def _emit_load_completed(self, exception=None):
        if exception is not None:
            self._garbage_widgets_and_toolbars()
        if self.__callback is not None:
            callback = self.__callback
            self.__callback = None
            callback(self, exception)
        elif exception is not None:
            qCritical('PluginHandler.load() failed%s' %
                      (':\n%s' % str(exception) if not exception else ''))

    def _garbage_widgets_and_toolbars(self):
        for widget in list(self._widgets.keys()):
            self.remove_widget(widget)
            self._delete_widget(widget)
        for toolbar in list(self._toolbars):
            self.remove_toolbar(toolbar)
            self._delete_toolbar(toolbar)

    def shutdown_plugin(self, callback):
        """
        Shut down the plugin and remove all added widgets.

        Completion is signaled asynchronously if a callback is passed.
        """
        self.__callback = callback
        try:
            self._shutdown_plugin()
        except Exception:
            qCritical('PluginHandler.shutdown_plugin() plugin "%s" raised an exception:\n%s' %
                      (str(self._instance_id), traceback.format_exc()))
            self.emit_shutdown_plugin_completed()

    def _shutdown_plugin(self):
        raise NotImplementedError

    def emit_shutdown_plugin_completed(self):
        self._garbage_widgets_and_toolbars()
        if self.__callback is not None:
            callback = self.__callback
            self.__callback = None
            callback(self._instance_id)

    def _delete_widget(self, widget):
        widget.deleteLater()

    def _delete_toolbar(self, toolbar):
        toolbar.deleteLater()

    def unload(self, callback=None):
        """
        Unload plugin.

        Completion is signaled asynchronously if a callback is passed.
        """
        self.__callback = callback
        try:
            self._unload()
        except Exception:
            qCritical('PluginHandler.unload() plugin "%s" raised an exception:\n%s' %
                      (str(self._instance_id), traceback.format_exc()))
            self._emit_unload_completed()

    def _unload(self):
        raise NotImplementedError

    def _emit_unload_completed(self):
        if self.__callback is not None:
            callback = self.__callback
            self.__callback = None
            callback(self._instance_id)

    def save_settings(self, plugin_settings, instance_settings, callback=None):
        """
        Save settings of the plugin and all dock widget title bars.

        Completion is signaled asynchronously if a callback is passed.
        """
        qDebug('PluginHandler.save_settings()')
        self.__instance_settings = instance_settings
        self.__callback = callback
        try:
            self._save_settings(plugin_settings, instance_settings)
        except Exception:
            qCritical('PluginHandler.save_settings() plugin "%s" raised an exception:\n%s' %
                      (str(self._instance_id), traceback.format_exc()))
            self.emit_save_settings_completed()

    def _save_settings(self, plugin_settings, instance_settings):
        raise NotImplementedError

    def emit_save_settings_completed(self):
        qDebug('PluginHandler.emit_save_settings_completed()')
        self._call_method_on_all_dock_widgets('save_settings', self.__instance_settings)
        self.__instance_settings = None
        if self.__callback is not None:
            callback = self.__callback
            self.__callback = None
            callback(self._instance_id)

    def _call_method_on_all_dock_widgets(self, method_name, instance_settings):
        for dock_widget, _, _ in self._widgets.values():
            name = 'dock_widget' + \
                dock_widget.objectName().replace(self._instance_id.tidy_str(), '', 1)
            settings = instance_settings.get_settings(name)
            method = getattr(dock_widget, method_name)
            try:
                method(settings)
            except Exception:
                qCritical('PluginHandler._call_method_on_all_dock_widgets(%s) failed:\n%s' %
                          (method_name, traceback.format_exc()))

    def restore_settings(self, plugin_settings, instance_settings, callback=None):
        """
        Restore settings of the plugin and all dock widget title bars.

        Completion is signaled asynchronously if a callback is passed.
        """
        qDebug('PluginHandler.restore_settings()')
        self.__instance_settings = instance_settings
        self.__callback = callback
        try:
            self._restore_settings(plugin_settings, instance_settings)
        except Exception:
            qCritical('PluginHandler.restore_settings() plugin "%s" raised an exception:\n%s' %
                      (str(self._instance_id), traceback.format_exc()))
            self.emit_restore_settings_completed()

    def _restore_settings(self, plugin_settings, instance_settings):
        raise NotImplementedError

    def emit_restore_settings_completed(self):
        qDebug('PluginHandler.emit_restore_settings_completed()')
        # call after plugin has restored settings as it may spawn additional dock widgets
        self._call_method_on_all_dock_widgets('restore_settings', self.__instance_settings)
        self.__instance_settings = None
        if self.__callback is not None:
            callback = self.__callback
            self.__callback = None
            callback(self._instance_id)

    def _create_dock_widget(self):
        dock_widget = DockWidget(self._container_manager)
        self._update_dock_widget_features(dock_widget)
        self._update_title_bar(dock_widget)
        self._set_window_icon(dock_widget)
        return dock_widget

    def _update_dock_widget_features(self, dock_widget):
        if self._application_context.options.lock_perspective or \
                self._application_context.options.standalone_plugin:
            # dock widgets are not closable when perspective is locked or plugin is
            # running standalone
            features = dock_widget.features()
            dock_widget.setFeatures(features ^ QDockWidget.DockWidgetClosable)
        if self._application_context.options.freeze_layout:
            # dock widgets are not closable when perspective is locked or plugin is
            # running standalone
            features = dock_widget.features()
            dock_widget.setFeatures(
                features ^ (QDockWidget.DockWidgetMovable | QDockWidget.DockWidgetFloatable))

    def _update_title_bar(self, dock_widget, hide_help=False, hide_reload=False):
        title_bar = dock_widget.titleBarWidget()
        if title_bar is None:
            title_bar = DockWidgetTitleBar(
                dock_widget, self._application_context.qtgui_path,
                hide_title=self._application_context.options.hide_title)
            dock_widget.setTitleBarWidget(title_bar)

            # connect extra buttons
            title_bar.connect_close_button(self._remove_widget_by_dock_widget)
            title_bar.connect_button('help', self._emit_help_signal)
            if hide_help:
                title_bar.show_button('help', not hide_help)
            title_bar.connect_button('reload', self._emit_reload_signal)
            if hide_reload:
                title_bar.show_button('reload', not hide_reload)
            title_bar.connect_button('configuration', self._trigger_configuration)
            title_bar.show_button('configuration', self._plugin_has_configuration)

    def _set_window_icon(self, widget):
        if self._plugin_descriptor:
            action_attributes = self._plugin_descriptor.action_attributes()
            if 'icon' in action_attributes and action_attributes['icon'] is not None:
                base_path = self._plugin_descriptor.attributes().get('plugin_path')
                icon = get_icon(
                    action_attributes['icon'], action_attributes.get('icontype', None), base_path)
                widget.setWindowIcon(icon)

    def _update_title_bars(self):
        if self._plugin_has_configuration:
            for dock_widget, _, _ in self._widgets.values():
                title_bar = dock_widget.titleBarWidget()
                title_bar.show_button('configuration')

    def _remove_widget_by_dock_widget(self, dock_widget):
        widget = [key for key, value in self._widgets.items() if value[0] == dock_widget][0]
        self.remove_widget(widget)

    def _emit_help_signal(self):
        self.help_signal.emit(str(self._instance_id))

    def _emit_reload_signal(self):
        self.reload_signal.emit(str(self._instance_id))

    def _trigger_configuration(self):
        self._plugin.trigger_configuration()

    def _add_dock_widget(self, dock_widget, widget):
        dock_widget.setWidget(widget)
        # every dock widget needs a unique name for save/restore geometry/state to work
        dock_widget.setObjectName(self._instance_id.tidy_str() + '__' + widget.objectName())
        self._add_dock_widget_to_main_window(dock_widget)
        signaler = WindowChangedSignaler(widget, widget)
        signaler.window_icon_changed_signal.connect(self._on_widget_icon_changed)
        signaler.window_title_changed_signal.connect(self._on_widget_title_changed)
        signaler2 = WindowChangedSignaler(dock_widget, dock_widget)
        signaler2.hide_signal.connect(self._on_dock_widget_hide)
        signaler2.show_signal.connect(self._on_dock_widget_show)
        signaler2.window_title_changed_signal.connect(self._on_dock_widget_title_changed)
        self._widgets[widget] = [dock_widget, signaler, signaler2]
        # trigger to update initial window icon and title
        signaler.emit_all()
        # trigger to update initial window state
        signaler2.emit_all()

    def _add_dock_widget_to_main_window(self, dock_widget):
        if self._main_window is not None:
            # warn about dock_widget with same object name
            old_dock_widget = self._main_window.findChild(DockWidget, dock_widget.objectName())
            if old_dock_widget is not None:
                qWarning('PluginHandler._add_dock_widget_to_main_window() duplicate object name ' +
                         '"%s", assign unique object names before adding widgets!' %
                         dock_widget.objectName())

            self._main_window.addDockWidget(Qt.BottomDockWidgetArea, dock_widget)

    def _on_widget_icon_changed(self, widget):
        dock_widget, _, _ = self._widgets[widget]
        dock_widget.setWindowIcon(widget.windowIcon())

    def _on_widget_title_changed(self, widget):
        dock_widget, _, _ = self._widgets[widget]
        dock_widget.setWindowTitle(widget.windowTitle())

    def _on_dock_widget_hide(self, dock_widget):
        if self._minimized_dock_widgets_toolbar:
            self._minimized_dock_widgets_toolbar.addDockWidget(dock_widget)

    def _on_dock_widget_show(self, dock_widget):
        if self._minimized_dock_widgets_toolbar:
            self._minimized_dock_widgets_toolbar.removeDockWidget(dock_widget)

    def _on_dock_widget_title_changed(self, dock_widget):
        self.label_updated.emit(str(self._instance_id), dock_widget.windowTitle())

    # pointer to QWidget must be used for PySide to work (at least with 1.0.1)
    @Slot('QWidget*')
    def remove_widget(self, widget):
        dock_widget, signaler, signaler2 = self._widgets[widget]
        self._widgets.pop(widget)
        if signaler is not None:
            signaler.window_icon_changed_signal.disconnect(self._on_widget_icon_changed)
            signaler.window_title_changed_signal.disconnect(self._on_widget_title_changed)
        if signaler2 is not None:
            # emit show signal to remove dock widget from minimized toolbar before removal
            signaler2.show_signal.emit(dock_widget)
            signaler2.hide_signal.disconnect(self._on_dock_widget_hide)
            signaler2.show_signal.disconnect(self._on_dock_widget_show)
        # remove dock widget from parent and delete later
        if self._main_window is not None:
            dock_widget.parent().removeDockWidget(dock_widget)
        # do not delete the widget, only the dock widget
        dock_widget.setParent(None)
        widget.setParent(None)
        dock_widget.deleteLater()
        # defer check for last widget closed to give plugin a chance to add
        # another widget right away
        self._defered_check_close.emit()

    def _add_toolbar(self, toolbar):
        # every toolbar needs a unique name for save/restore geometry/state to work
        toolbar_object_name = toolbar.objectName()
        prefix = self._instance_id.tidy_str() + '__'
        # when added, removed and readded the prefix should not be prepended multiple times
        if not toolbar_object_name.startswith(prefix):
            toolbar_object_name = prefix + toolbar_object_name
        toolbar.setObjectName(toolbar_object_name)

        if self._application_context.options.freeze_layout:
            toolbar.setMovable(False)

        self._toolbars.append(toolbar)
        if self._main_window is not None:
            # warn about toolbar with same object name
            old_toolbar = self._main_window.findChild(QToolBar, toolbar.objectName())
            if old_toolbar is not None:
                qWarning('PluginHandler._add_toolbar() duplicate object name "%s", '
                         'assign unique object names before adding toolbars!' %
                         toolbar.objectName())
            self._main_window.addToolBar(Qt.TopToolBarArea, toolbar)

    # pointer to QToolBar must be used for PySide to work (at least with 1.0.1)
    @Slot('QToolBar*')
    def remove_toolbar(self, toolbar):
        self._toolbars.remove(toolbar)
        # detach toolbar from parent
        if toolbar.parent():
            toolbar.parent().removeToolBar(toolbar)
        # defer check for last widget closed to give plugin a chance to add
        # another widget right away
        self._defered_check_close.emit()

    def _check_close(self):
        # close plugin when no widgets or toolbars are left
        if len(self._widgets) + len(self._toolbars) == 0:
            self._emit_close_plugin()

    def _emit_close_plugin(self):
        self.close_signal.emit(str(self._instance_id))
