# Software License Agreement (BSD License)
#
# Copyright (c) 2012, Dorian Scholz
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of Willow Garage, Inc. nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import os

from ament_index_python.resources import get_resource
from python_qt_binding import loadUi
from python_qt_binding.QtCore import qWarning
from python_qt_binding.QtWidgets import QDialog, QLabel

from .checkbox_group import CheckBoxGroup
from .exclusive_options_group import ExclusiveOptionGroup


class SimpleSettingsDialog(QDialog):
    """Simple dialog that can show multiple settings groups and returns their combined results."""

    def __init__(self, title='Options', description=None):
        super(SimpleSettingsDialog, self).__init__()
        self.setObjectName('SimpleSettingsDialog')

        _, package_path = get_resource('packages', 'qt_gui_py_common')
        ui_file = os.path.join(
            package_path, 'share', 'qt_gui_py_common', 'resource', 'simple_settings_dialog.ui')
        loadUi(ui_file, self)

        self.setWindowTitle(title)
        self._settings_groups = []

        if description is not None:
            self.add_label(description)

    def add_label(self, text):
        self.group_area.layout().addWidget(QLabel(text))

    def add_exclusive_option_group(self, *args, **kwargs):
        """Add an ExclusiveOptionGroup."""
        self.add_settings_group(ExclusiveOptionGroup(*args, **kwargs))

    def add_checkbox_group(self, *args, **kwargs):
        """Add a CheckBoxGroup."""
        self.add_settings_group(CheckBoxGroup(*args, **kwargs))

    def add_settings_group(self, settings_group):
        """Add a settings group, which is any widget with a get_settings method."""
        if not hasattr(settings_group, 'get_settings'):
            qWarning('add_settings_group(): this settings group has no get_settings method to ' +
                     'collect the settings!')
        self._settings_groups.append(settings_group)
        self.group_area.layout().addWidget(settings_group)

    def get_settings(self):
        """Return the combined settings from all settings groups as a list."""
        if self.exec_() == QDialog.Accepted:
            results = []
            for settings_group in self._settings_groups:
                if hasattr(settings_group, 'get_settings'):
                    results.append(settings_group.get_settings())
                else:
                    results.append(None)
            return results
        return [None] * len(self._settings_groups)
