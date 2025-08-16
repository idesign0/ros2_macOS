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

from python_qt_binding.QtCore import QMutex, QMutexLocker, QObject


class SettingsProxy(QObject):
    """Wrapper of a QSettings instance simplifying access of grouped data."""

    def __init__(self, qsettings):
        super(SettingsProxy, self).__init__()
        self.setObjectName('SettingsProxy')

        self._qsettings = qsettings
        self._mutex = QMutex(QMutex.Recursive)

    def all_keys(self, group):
        locker = QMutexLocker(self._mutex)  # noqa: F841
        self._qsettings.beginGroup(group)
        keys = self._qsettings.allKeys()
        self._qsettings.endGroup()
        return keys

#    def begin_read_array(self, group):

#    def begin_write_array(self, group):

    def child_groups(self, group):
        locker = QMutexLocker(self._mutex)  # noqa: F841
        self._qsettings.beginGroup(group)
        groups = self._qsettings.childGroups()
        self._qsettings.endGroup()
        return groups

    def child_keys(self, group):
        locker = QMutexLocker(self._mutex)  # noqa: F841
        self._qsettings.beginGroup(group)
        keys = self._qsettings.childKeys()
        self._qsettings.endGroup()
        return keys

    def contains(self, group, key):
        locker = QMutexLocker(self._mutex)  # noqa: F841
        self._qsettings.beginGroup(group)
        key_exists = self._qsettings.contains(key)
        self._qsettings.endGroup()
        return key_exists

#    def end_array(self):

    def remove(self, group, key):
        locker = QMutexLocker(self._mutex)  # noqa: F841
        self._qsettings.beginGroup(group)
        self._qsettings.remove(key)
        self._qsettings.endGroup()

#    def set_array_index(self, i):

    def set_value(self, group, key, value):
        locker = QMutexLocker(self._mutex)  # noqa: F841
        self._qsettings.beginGroup(group)
        self._qsettings.setValue(key, value)
        self._qsettings.endGroup()

    def value(self, group, key, default_value=None):
        locker = QMutexLocker(self._mutex)  # noqa: F841
        self._qsettings.beginGroup(group)
        v = self._qsettings.value(key, default_value)
        self._qsettings.endGroup()
        return v
