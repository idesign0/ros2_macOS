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

from python_qt_binding.QtCore import QObject, Slot


class Settings(QObject):
    """Storage of key-value data with a QSettings-like interface."""

    def __init__(self, settings_proxy, group):
        super(Settings, self).__init__()
        self.setObjectName('Settings')

        self._settings_proxy = settings_proxy
        self._group = group

    def get_settings(self, group):
        prefix = self._group
        if prefix != '':
            prefix += '/'
        return Settings(self._settings_proxy, prefix + group)

    @Slot(result='QStringList')
    def all_keys(self):
        return self._settings_proxy.all_keys(self._group)

#    def begin_read_array(self):

#    def begin_write_array(self):

    @Slot(result='QStringList')
    def child_groups(self):
        return self._settings_proxy.child_groups(self._group)

    @Slot(result='QStringList')
    def child_keys(self):
        return self._settings_proxy.child_keys(self._group)

    @Slot(str, result=bool)
    def contains(self, key):
        return self._settings_proxy.contains(self._group, key)

#    def end_array(self):

    @Slot(str)
    def remove(self, key):
        self._settings_proxy.remove(self._group, key)

#    def set_array_index(self, i):

    @Slot(str, 'QVariant')
    def set_value(self, key, value):
        # work around for NoneType values via DBus
        if value is None:
            value = '__NoneType__'
        self._settings_proxy.set_value(self._group, key, value)

    @Slot(str, 'QVariant', result='QVariant')
    def value(self, key, default_value=None):
        # work around for passing NoneType (default_)values via DBus
        if default_value is None:
            default_value = '__NoneType__'
        value = self._settings_proxy.value(self._group, key, default_value)
        if value == '__NoneType__':
            value = None
        return value
