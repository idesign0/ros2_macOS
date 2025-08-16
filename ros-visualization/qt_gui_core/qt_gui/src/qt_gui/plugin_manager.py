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

import time
import traceback

from python_qt_binding.QtCore import \
    qCritical, qDebug, QObject, Qt, qWarning, Signal, Slot

from qt_gui.errors import PluginLoadError
from qt_gui.plugin_handler_container import PluginHandlerContainer
from qt_gui.plugin_handler_direct import PluginHandlerDirect
from qt_gui.plugin_instance_id import PluginInstanceId
from qt_gui.plugin_menu import PluginMenu
from qt_gui.settings import Settings
from qt_gui.settings_proxy import SettingsProxy


class PluginManager(QObject):
    """
    Manager of plugin life cycle.

    It creates a specific `PluginHandler` for each plugin instance and maintains the perspective
    specific set of running plugins.
    """

    plugin_load_unload_signal = Signal(int)
    plugins_about_to_change_signal = Signal()
    plugins_changed_signal = Signal()
    plugin_help_signal = Signal(object)
    save_settings_completed_signal = Signal()
    close_application_signal = Signal()
    _deferred_reload_plugin_signal = Signal(str)

    discovery_cache_max_age = 60 * 60 * 24  # one day

    def __init__(self, plugin_provider, settings, application_context, settings_prefix=''):
        super(PluginManager, self).__init__()
        self.setObjectName('PluginManager')

        self._plugin_provider = plugin_provider
        self._settings = Settings(SettingsProxy(settings), '/'.join(
            [x for x in ['plugin_manager', settings_prefix] if x != '']))
        self._application_context = application_context

        self._main_window = None
        self._container_manager = None
        self._plugin_menu = None
        self._minimized_dock_widgets_toolbar = None

        self._global_settings = None
        self._perspective_settings = None
        self._plugin_descriptors = None
        self._running_plugins = {}

        self._number_of_ongoing_calls = None

        if self._application_context.options.multi_process or \
                self._application_context.options.embed_plugin:
            try:
                from qt_gui.plugin_handler_xembed import PluginHandlerXEmbed  # noqa: F401
            except ImportError:
                qCritical('PluginManager.__init__() multiprocess-mode only available under linux')
                exit(-1)

        # force connection type to queued, to delay the 'reloading' giving the
        # 'unloading' time to finish
        self._deferred_reload_plugin_signal.connect(
            self._reload_plugin_load, type=Qt.QueuedConnection)

        if self._application_context.provide_app_dbus_interfaces:
            from qt_gui.plugin_manager_dbus_interface import PluginManagerDBusInterface
            self._dbus_service = PluginManagerDBusInterface(self, self._application_context)

    def set_main_window(self, main_window, menu_bar, container_manager):
        self._main_window = main_window
        self._container_manager = container_manager
        self.plugins_changed_signal.connect(self._container_manager.restore_state_of_containers)
        if menu_bar is not None:
            self._plugin_menu = PluginMenu(menu_bar, self)
            self._plugin_menu.load_plugin_signal.connect(self.load_plugin)
            self._plugin_menu.unload_plugin_signal.connect(self.unload_plugin)

    def set_minimized_dock_widgets_toolbar(self, toolbar):
        self._minimized_dock_widgets_toolbar = toolbar

    def discover(self):
        # skip discover if called multiple times
        if self._plugin_descriptors is not None:
            return
        self._plugin_descriptors = {}

        # register discovered plugins
        plugin_descriptors = self._discover()
        for plugin_descriptor in plugin_descriptors:
            self._plugin_descriptors[plugin_descriptor.plugin_id()] = plugin_descriptor

            if self._plugin_menu is not None:
                self._plugin_menu.add_plugin(plugin_descriptor)

        if self._container_manager is not None:
            descriptor = self._container_manager.get_container_descriptor()
            self._plugin_descriptors[descriptor.plugin_id()] = descriptor
            if self._plugin_menu is not None:
                self._container_manager.add_to_plugin_menu(self._plugin_menu)

    def _discover(self):
        discovery_data = self._settings.get_settings('discovery_data')
        # check timestamp of the cached discovery data
        cache_stamp = self._settings.value('discovery_timestamp', default_value=None)
        if cache_stamp:
            cache_stamp = float(cache_stamp)

        # wipe cached data when forcing discovery or when cache is to old (or from the future)
        now = time.time()
        if self._application_context.options.force_discover or \
                not cache_stamp or \
                cache_stamp > now or \
                cache_stamp + PluginManager.discovery_cache_max_age < now:
            qDebug('PluginManager._discover() force discovery of plugins')
            cache_stamp = None
            for k in discovery_data.all_keys():
                discovery_data.remove(k)
        else:
            qDebug('PluginManager._discover() using cached plugin discovery information')

        plugin_descriptors = self._plugin_provider.discover(discovery_data)

        # store timestamp and newly discovered data
        if not cache_stamp:
            self._settings.set_value('discovery_timestamp', now)

        return plugin_descriptors

    def find_plugins_by_name(self, lookup_name):
        plugins = {}
        for plugin_id, plugin_full_name in self.get_plugins().items():
            if plugin_full_name.lower().find(lookup_name.lower()) >= 0 or \
                    plugin_id.lower().find(lookup_name.lower()) >= 0:
                plugins[plugin_id] = plugin_full_name
        return plugins

    def get_plugins(self):
        self.discover()
        plugins = {}
        for plugin_id, plugin_descriptor in self._plugin_descriptors.items():
            plugins[plugin_id] = '/'.join(
                plugin_descriptor.attributes().get('class_type', 'unknown').split('::'))
        return plugins

    def get_plugin_descriptor(self, plugin_id):
        return self._plugin_descriptors.get(plugin_id, None)

    def is_plugin_running(self, plugin_id, serial_number):
        instance_id = PluginInstanceId(plugin_id, serial_number)
        return str(instance_id) in self._running_plugins

    @Slot(str)
    @Slot(str, int)
    def load_plugin(self, plugin_id, serial_number=None, argv=None):
        qDebug('PluginManager.load_plugin(%s, %s)' %
               (str(plugin_id), str(serial_number) if serial_number is not None else ''))
        # save state of top-level widgets
        self.plugins_about_to_change_signal.emit()
        if serial_number is None:
            serial_number = self._next_serial_number(plugin_id)
        instance_id = PluginInstanceId(plugin_id, serial_number)
        self._load_plugin_load(instance_id, self._load_plugin_restore, argv)

    def _next_serial_number(self, plugin_id):
        # convert from unicode
        plugin_id = str(plugin_id)
        # collect serial numbers of all running instances of the specific plugin
        used_serial_numbers = {}
        for info in self._running_plugins.values():
            if info['instance_id'].plugin_id == plugin_id:
                used_serial_numbers[info['instance_id'].serial_number] = None

        # find first not used serial number
        serial_number = 1
        while serial_number in used_serial_numbers:
            serial_number = serial_number + 1
        return serial_number

    def _load_plugin_load(self, instance_id, callback, argv=None):
        # if the requested instance is already running, do nothing
        if str(instance_id) in self._running_plugins:
            raise Exception(
                'PluginManager._load_plugin(%s) instance already loaded' % str(instance_id))

        # containers are pseudo-plugins and handled by a special handler
        if self._container_manager is not None and \
            instance_id.plugin_id == \
                self._container_manager.get_container_descriptor().plugin_id():
            handler = PluginHandlerContainer(self, self._main_window, instance_id,
                                             self._application_context, self._container_manager)

        # use platform specific handler for multiprocess-mode if available
        elif self._application_context.options.multi_process or \
                self._application_context.options.embed_plugin:
            try:
                from qt_gui.plugin_handler_xembed import PluginHandlerXEmbed
                handler = \
                    PluginHandlerXEmbed(self, self._main_window, instance_id,
                                        self._application_context, self._container_manager, argv)
            except ImportError:
                qCritical(
                    'PluginManager._load_plugin() could not load plugin in a separate process')
                return

        # use direct handler for in-process plugins
        else:
            handler = PluginHandlerDirect(self, self._main_window, instance_id,
                                          self._application_context, self._container_manager, argv)

        handler.set_minimized_dock_widgets_toolbar(self._minimized_dock_widgets_toolbar)

        if instance_id.plugin_id not in self._plugin_descriptors.keys():
            qWarning(
                'PluginManager._load_plugin() could not load plugin "%s": plugin not available' %
                (instance_id.plugin_id))
            return
        plugin_descriptor = self._plugin_descriptors[instance_id.plugin_id]
        handler.set_plugin_descriptor(plugin_descriptor)

        self._add_running_plugin(instance_id, handler)
        handler.load(self._plugin_provider, callback)

    def _add_running_plugin(self, instance_id, handler):
        if self._plugin_menu is not None:
            plugin_descriptor = self._plugin_descriptors[instance_id.plugin_id]
            self._plugin_menu.add_instance(plugin_descriptor, instance_id)
            handler.label_updated.connect(self._plugin_menu.update_plugin_instance_label)

        info = {
            'handler': handler,
            'instance_id': instance_id
        }
        self._running_plugins[str(instance_id)] = info

    def _load_plugin_restore(self, handler, exception):
        qDebug('PluginManager._load_plugin_restore()')
        self._load_plugin_completed(handler, exception)
        if exception is None:
            # restore settings after load
            self._restore_plugin_settings(handler.instance_id(), self._emit_load_plugin_completed)

    def _load_plugin_completed(self, handler, exception):
        instance_id = handler.instance_id()
        if exception is not None:
            if isinstance(exception, PluginLoadError):
                qWarning('PluginManager._load_plugin() could not load plugin "%s": %s' %
                         (instance_id.plugin_id, exception))
            else:
                qCritical('PluginManager._load_plugin() could not load plugin "%s"%s' %
                          (instance_id.plugin_id, (':\n%s' % traceback.format_exc() if
                           isinstance(exception, Exception) else '')))
            self._remove_running_plugin(instance_id)
            # quit embed application
            if self._application_context.options.embed_plugin:
                exit(-1)
            return

        qDebug('PluginManager._load_plugin(%s) successful' % str(instance_id))

        handler.close_signal.connect(self.unload_plugin)
        handler.reload_signal.connect(self.reload_plugin)
        handler.help_signal.connect(self._emit_plugin_help_signal)
        self.plugin_load_unload_signal.emit(len(self._running_plugins) == 0)

    def _emit_plugin_help_signal(self, instance_id_str):
        instance_id = PluginInstanceId(instance_id=instance_id_str)
        plugin_descriptor = self._plugin_descriptors[instance_id.plugin_id]
        self.plugin_help_signal.emit(plugin_descriptor)

    def _restore_plugin_settings(self, instance_id, callback):
        if self._global_settings is not None and self._perspective_settings is not None:
            info = self._running_plugins[str(instance_id)]
            plugin_settings = self._global_settings.get_settings(
                'plugin__' + instance_id.tidy_plugin_str())
            instance_settings = self._perspective_settings.get_settings(
                'plugin__' + instance_id.tidy_str())
            handler = info['handler']
            handler.restore_settings(plugin_settings, instance_settings, callback)
        else:
            callback(instance_id)

    def _emit_load_plugin_completed(self, instance_id):
        qDebug('PluginManager._emit_load_plugin_completed()')
        # restore state of top-level widgets
        self.plugins_changed_signal.emit()

    @Slot(str)
    def unload_plugin(self, instance_id_str):
        # unloading a plugin with locked perspective or running standalone
        # triggers close of application
        if self._application_context.options.lock_perspective or \
                self._application_context.options.standalone_plugin:
            self._close_application_signal()
            return
        instance_id = PluginInstanceId(instance_id=instance_id_str)
        qDebug('PluginManager.unload_plugin(%s)' % str(instance_id))
        # save state of top-level widgets
        self.plugins_about_to_change_signal.emit()
        # save settings before shutdown and unloading
        self._save_plugin_settings(instance_id, self._unload_plugin_shutdown)

    def _save_plugin_settings(self, instance_id, callback):
        if self._global_settings is not None and self._perspective_settings is not None:
            info = self._running_plugins[str(instance_id)]
            plugin_settings = self._global_settings.get_settings(
                'plugin__' + instance_id.tidy_plugin_str())
            instance_settings = self._perspective_settings.get_settings(
                'plugin__' + instance_id.tidy_str())
            handler = info['handler']
            handler.save_settings(plugin_settings, instance_settings, callback)
        else:
            callback(instance_id)

    def _unload_plugin_shutdown(self, instance_id):
        qDebug('PluginManager._unload_plugin_shutdown(%s)' % str(instance_id))
        self._shutdown_plugin(instance_id, self._unload_plugin_unload)

    def _shutdown_plugin(self, instance_id, callback):
        # shutdown plugin before unloading
        info = self._running_plugins[str(instance_id)]
        handler = info['handler']
        handler.close_signal.disconnect(self.unload_plugin)
        handler.shutdown_plugin(callback)

    def _unload_plugin_unload(self, instance_id):
        qDebug('PluginManager._unload_plugin_unload(%s)' % str(instance_id))
        self._unload_plugin(instance_id, self._unload_plugin_completed)

    def _unload_plugin(self, instance_id, callback=None):
        # unload plugin
        info = self._running_plugins[str(instance_id)]
        handler = info['handler']
        handler.unload(callback)

    def _unload_plugin_completed(self, instance_id):
        qDebug('PluginManager._unload_plugin_completed(%s)' % str(instance_id))
        self._remove_running_plugin(instance_id)
        self.plugin_load_unload_signal.emit(len(self._running_plugins) == 0)

    def _remove_running_plugin(self, instance_id):
        info = self._running_plugins[str(instance_id)]
        if self._plugin_menu is not None:
            self._plugin_menu.remove_instance(instance_id)
            info['handler'].label_updated.disconnect(
                self._plugin_menu.update_plugin_instance_label)
        self._running_plugins.pop(str(instance_id))

    @Slot(str)
    def reload_plugin(self, instance_id_str):
        instance_id = PluginInstanceId(instance_id=instance_id_str)
        qDebug('PluginManager.reload_plugin(%s)' % str(instance_id))
        # save state of top-level widgets
        self.plugins_about_to_change_signal.emit()
        self._reload_plugin_save(instance_id)

    def _reload_plugin_save(self, instance_id):
        # save settings before unloading
        self._save_plugin_settings(instance_id, self._reload_plugin_shutdown)

    def _reload_plugin_shutdown(self, instance_id):
        qDebug('PluginManager._reload_plugin_shutdown(%s)' % str(instance_id))
        self._shutdown_plugin(instance_id, self._reload_plugin_unload)

    def _reload_plugin_unload(self, instance_id):
        qDebug('PluginManager._reload_plugin_unload(%s)' % str(instance_id))
        self._unload_plugin(instance_id, self._reload_plugin_schedule_load)

    def _reload_plugin_schedule_load(self, instance_id):
        qDebug('PluginManager._reload_plugin_schedule_load(%s)' % str(instance_id))
        self._remove_running_plugin(instance_id)
        self._deferred_reload_plugin_signal.emit(str(instance_id))

    def _reload_plugin_load(self, instance_id_str):
        instance_id = PluginInstanceId(instance_id=instance_id_str)
        qDebug('PluginManager._reload_plugin_load(%s)' % str(instance_id))
        self._load_plugin_load(instance_id, self._reload_plugin_restore)

    def _reload_plugin_restore(self, handler, exception):
        qDebug('PluginManager._reload_plugin_restore()')
        self._load_plugin_completed(handler, exception)
        if exception is None:
            # restore settings after load
            self._restore_plugin_settings(handler.instance_id(), self._emit_load_plugin_completed)

    def save_settings(self, global_settings, perspective_settings):
        self._save_settings(global_settings, perspective_settings, self._save_settings_callback)

    def _save_settings(self, global_settings, perspective_settings, callback):
        qDebug('PluginManager.save_settings()')
        self._global_settings = global_settings.get_settings('pluginmanager')
        self._perspective_settings = perspective_settings.get_settings('pluginmanager')
        self._store_running_plugins()
        # trigger async call on all running plugins
        self._number_of_ongoing_calls = len(self._running_plugins)
        if self._number_of_ongoing_calls > 0:
            # If changing perspectives, plugins may be removed from this dictionary during
            # _save_plugin_settings, so a shallow copy of the values is needed
            for info in list(self._running_plugins.values()):
                self._save_plugin_settings(info['instance_id'], callback)
        else:
            callback()

    def _store_running_plugins(self):
        if self._perspective_settings is not None:
            plugins = {}
            for info in self._running_plugins.values():
                instance_id = info['instance_id']
                plugin_id = instance_id.plugin_id
                if plugin_id not in plugins:
                    plugins[plugin_id] = []
                plugins[plugin_id].append(instance_id.serial_number)
            self._perspective_settings.set_value('running-plugins', plugins)

    def _save_settings_callback(self, instance_id=None):
        if instance_id is not None:
            self._number_of_ongoing_calls = self._number_of_ongoing_calls - 1
        if self._number_of_ongoing_calls == 0:
            qDebug('PluginManager.save_settings() completed')
            self._number_of_ongoing_calls = None
            self.save_settings_completed_signal.emit()

    def close_application(self, global_settings, perspective_settings):
        self._save_settings(
            global_settings, perspective_settings, self._close_application_save_callback)

    def _close_application_save_callback(self, instance_id=None):
        self._save_settings_callback(instance_id)
        if self._number_of_ongoing_calls is None:
            self._close_application_shutdown_plugins()

    def _close_application_shutdown_plugins(self):
        # trigger async call on all running plugins
        self._number_of_ongoing_calls = len(self._running_plugins)
        if self._number_of_ongoing_calls > 0:
            for info in self._running_plugins.values():
                self._shutdown_plugin(
                    info['instance_id'], self._close_application_shutdown_callback)
        else:
            self._close_application_shutdown_callback()

    def _close_application_shutdown_callback(self, instance_id=None):
        if instance_id is not None:
            self._number_of_ongoing_calls = self._number_of_ongoing_calls - 1
        if self._number_of_ongoing_calls == 0:
            qDebug('PluginManager.close_application() completed')
            self._number_of_ongoing_calls = None
            self._close_application_signal()

    def _close_application_signal(self):
        self._plugin_provider.shutdown()
        self.close_application_signal.emit()

    def restore_settings(self, global_settings, perspective_settings):
        qDebug('PluginManager.restore_settings()')
        self._global_settings = global_settings.get_settings('pluginmanager')
        self._perspective_settings = perspective_settings.get_settings('pluginmanager')
        self._restore_settings_save_obsolete()

    def _restore_settings_save_obsolete(self):
        # trigger shutdown of obsolete plugins
        plugins = self._restore_running_plugins_get_plugins()
        obsolete = []
        for instance_id in self._running_plugins.keys():
            if instance_id not in plugins.keys():
                obsolete.append(PluginInstanceId(instance_id=instance_id))
        self._number_of_ongoing_calls = len(obsolete)
        if self._number_of_ongoing_calls > 0:
            qDebug('PluginManager.restore_settings() unloading %d obsolete plugins' %
                   self._number_of_ongoing_calls)
            for instance_id in obsolete:
                self._shutdown_plugin(instance_id, self._restore_settings_unload_obsolete)
        else:
            self._restore_settings_unload_obsolete_callback()

    def _restore_running_plugins_get_plugins(self):
        plugins = {}
        if self._perspective_settings is not None:
            data = self._perspective_settings.value('running-plugins', {})
            for plugin_id, serial_numbers in data.items():
                for serial_number in serial_numbers:
                    instance_id = PluginInstanceId(plugin_id, serial_number)
                    plugins[str(instance_id)] = instance_id
        return plugins

    def _restore_settings_unload_obsolete(self, instance_id):
        # trigger unload of obsolete plugins
        self._unload_plugin(instance_id, self._restore_settings_unload_obsolete_callback)

    def _restore_settings_unload_obsolete_callback(self, instance_id=None):
        if instance_id is not None:
            self._number_of_ongoing_calls = self._number_of_ongoing_calls - 1
            self._remove_running_plugin(instance_id)
        if self._number_of_ongoing_calls == 0:
            if instance_id is not None:
                qDebug('PluginManager.restore_settings() all obsolete plugins unloaded')
            self._number_of_ongoing_calls = None
            self._restore_settings_load_missing()

    def _restore_settings_load_missing(self):
        # trigger_load of not yet loaded plugins
        plugins = self._restore_running_plugins_get_plugins()
        loading = []
        for instance_id_str, instance_id in plugins.items():
            if instance_id_str not in self._running_plugins.keys():
                loading.append(instance_id)
        self._number_of_ongoing_calls = len(loading)
        if self._number_of_ongoing_calls > 0:
            qDebug('PluginManager.restore_settings() loading %d plugins' %
                   self._number_of_ongoing_calls)
            for instance_id in loading:
                self._load_plugin_load(instance_id, self._restore_settings_load_missing_callback)
        else:
            self._restore_settings_load_missing_callback()

    def _restore_settings_load_missing_callback(self, handler=None, exception=None):
        if handler is not None:
            self._number_of_ongoing_calls = self._number_of_ongoing_calls - 1
            self._load_plugin_completed(handler, exception)
        if self._number_of_ongoing_calls == 0:
            if handler is not None:
                qDebug('PluginManager.restore_settings() all missing plugins loaded')
            self._number_of_ongoing_calls = None
            self._restore_settings_restore()

    def restore_settings_without_plugins(self, global_settings, perspective_settings):
        qDebug('PluginManager.restore_settings_without_plugins()')
        self._global_settings = global_settings.get_settings('pluginmanager')
        self._perspective_settings = perspective_settings.get_settings('pluginmanager')
        self._restore_settings_restore()

    def _restore_settings_restore(self):
        # trigger restore settings for all running plugins
        self._number_of_ongoing_calls = len(self._running_plugins)
        if self._number_of_ongoing_calls > 0:
            for info in self._running_plugins.values():
                self._restore_plugin_settings(
                    info['instance_id'], self._restore_settings_restore_callback)
        else:
            self._restore_settings_restore_callback()

    def _restore_settings_restore_callback(self, instance_id=None):
        if instance_id is not None:
            self._number_of_ongoing_calls = self._number_of_ongoing_calls - 1
        if self._number_of_ongoing_calls == 0:
            if instance_id is not None:
                qDebug('PluginManager.restore_settings() all plugin settings restored')
            self._number_of_ongoing_calls = None
            # restore state of top-level widgets
            self.plugins_changed_signal.emit()
