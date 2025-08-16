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


class SettingsProxyDBusService(dbus.service.Object):
    """DBus service of a 'Settings' instance."""

    def __init__(self, object_path):
        super(SettingsProxyDBusService, self).__init__(object_path)
        self._settings_proxy = None
        self._group = None

    def set_settings(self, settings):
        if settings is not None:
            self._settings_proxy = settings._settings_proxy
            self._group = settings._group
        else:
            self._settings_proxy = None
            self._group = None

    @dbus.service.method('org.ros.qt_gui.Settings', in_signature='s', out_signature='av')
    def all_keys(self, group):
        return self._settings_proxy.all_keys(self._group + '/' + group)

    @dbus.service.method('org.ros.qt_gui.Settings', in_signature='s', out_signature='as')
    def child_groups(self, group):
        return self._settings_proxy.child_groups(self._group + '/' + group)

    @dbus.service.method('org.ros.qt_gui.Settings', in_signature='s', out_signature='as')
    def child_keys(self, group):
        return self._settings_proxy.child_keys(self._group + '/' + group)

    @dbus.service.method('org.ros.qt_gui.Settings', in_signature='ss', out_signature='b')
    def contains(self, group, key):
        return self._settings_proxy.contains(self._group + '/' + group, key)

    @dbus.service.method('org.ros.qt_gui.Settings', in_signature='ss', out_signature='')
    def remove(self, group, key):
        self._settings_proxy.remove(self._group + '/' + group, key)

    @dbus.service.method('org.ros.qt_gui.Settings', in_signature='ssv', out_signature='')
    def set_value(self, group, key, value):
        value = self._sanitize_value(value)
        self._settings_proxy.set_value(self._group + '/' + group, key, value)

    @dbus.service.method('org.ros.qt_gui.Settings', in_signature='ssv', out_signature='v')
    def value(self, group, key, default_value=None):
        return self._settings_proxy.value(self._group + '/' + group, key, default_value)

    def _sanitize_value(self, value):
        # transform DBus types to Python types to work with Pickle
        if isinstance(value, dbus.Boolean):
            value = bool(value)
        elif isinstance(value,
                        (dbus.Byte, dbus.Int16, dbus.Int32, dbus.Int64,
                            dbus.UInt16, dbus.UInt32, dbus.UInt64)):
            value = int(value)
        elif isinstance(value, dbus.Double):
            value = float(value)
        elif isinstance(value, dbus.String):
            value = str(value)
        elif isinstance(value, dbus.UTF8String):
            try:
                value = unicode(value)
            except NameError:
                value = str(value)
        return value
