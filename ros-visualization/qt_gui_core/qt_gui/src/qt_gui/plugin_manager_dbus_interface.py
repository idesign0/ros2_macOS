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

import dbus
from dbus.service import BusName, Object

from python_qt_binding.QtCore import qDebug, qWarning


class PluginManagerDBusInterface(Object):
    """DBus service of the `PluginManager` available on the unique bus name."""

    def __init__(self, plugin_manager, application_context):
        bus_name = BusName(application_context.dbus_unique_bus_name, dbus.SessionBus())
        super(PluginManagerDBusInterface, self).__init__(bus_name, '/PluginManager')
        self._plugin_manager = plugin_manager

    @dbus.service.method('org.ros.qt_gui.PluginManager', in_signature='ss', out_signature='is')
    def start_plugin(self, plugin_name, argv):
        qDebug('PluginManagerDBusInterface.start_plugin(%s)' % plugin_name)
        plugins = self._plugin_manager.find_plugins_by_name(plugin_name)
        if len(plugins) == 0:
            msg = 'PluginManagerDBusInterface.start_plugin() found no plugin matching "%s"' % \
                  plugin_name
            qWarning(msg)
            return (1, msg)
        elif len(plugins) > 1:
            msg = 'PluginManagerDBusInterface.start_plugin() found multiple plugins ' \
                  'matching "%s"\n%s' % (plugin_name, '\n'.join(plugins.values()))
            qWarning(msg)
            return (1, msg)
        plugin_id = next(iter(plugins))
        self._plugin_manager.load_plugin(plugin_id, argv=argv.split(' ') if argv else [])
        return (0, plugin_id)
