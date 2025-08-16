# Software License Agreement (BSD License)
#
# Copyright (c) 2014, Andrew Wilson
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

from python_qt_binding.QtWidgets import \
    QButtonGroup, QCheckBox, QGroupBox, QLabel, QVBoxLayout


class CheckBoxGroup(QGroupBox):
    """
    Creates a button group of non-exclusive checkbox options.

    Options must be a dict with following keys: 'enabled','title','description','tooltip'
    """

    def __init__(self, options, title='Checkboxes', selected_indexes=[], parent=None):
        super(CheckBoxGroup, self).__init__()
        self.setTitle(title)
        self.setLayout(QVBoxLayout())
        self._button_group = QButtonGroup()
        self._button_group.setExclusive(False)
        self._options = options
        if parent is None:
            parent = self

        for (button_id, option) in enumerate(self._options):

            checkbox = QCheckBox(option.get('title', 'option %d' % button_id))
            checkbox.setEnabled(option.get('enabled', True))
            checkbox.setChecked(button_id in selected_indexes)
            checkbox.setToolTip(option.get('tooltip', ''))

            self._button_group.addButton(checkbox, button_id)
            parent.layout().addWidget(checkbox)
            if 'description' in option:
                parent.layout().addWidget(QLabel(option['description']))

    def get_settings(self):
        """Return dictionary with selected_indexes (array) and selected_options (array) keys."""
        selected_indexes = []
        selected_options = []
        for button in self._button_group.buttons():
            if button.isChecked():
                selected_indexes.append(self._button_group.id(button))
                selected_options.append(self._options[self._button_group.id(button)])
        return {'selected_indexes': selected_indexes, 'selected_options': selected_options}
