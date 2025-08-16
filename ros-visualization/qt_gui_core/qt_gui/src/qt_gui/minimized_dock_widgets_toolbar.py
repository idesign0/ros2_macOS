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

from python_qt_binding.QtCore import QSignalMapper, Qt
from python_qt_binding.QtGui import QIcon
from python_qt_binding.QtWidgets import QAction, QToolBar, QWidget


class MinimizedDockWidgetsToolbar(QToolBar):

    max_label_length = 15

    def __init__(self, container_manager, parent=None):
        super(MinimizedDockWidgetsToolbar, self).__init__(parent=parent)
        self.setWindowTitle(self.tr('Minimized dock widgets'))
        self.setObjectName('MinimizedDockWidgetsToolbar')
        self.setToolButtonStyle(Qt.ToolButtonTextBesideIcon)
        self._container_manager = container_manager
        self._signal_mapper = QSignalMapper(self)
        self._signal_mapper.mapped[QWidget].connect(self._on_action_triggered)
        self._dock_widgets = {}

        self.hide()

    def addDockWidget(self, dock_widget):
        # remove action for same dock widget if exists
        self.removeDockWidget(dock_widget)

        icon = dock_widget.windowIcon()
        if icon.isNull():
            icon = QIcon.fromTheme('folder')
        title = dock_widget.windowTitle()
        action = QAction(icon, title, self)
        # truncate label if necessary
        if len(title) > MinimizedDockWidgetsToolbar.max_label_length:
            action.setToolTip(title)
            action.setIconText(title[0:MinimizedDockWidgetsToolbar.max_label_length] + '...')
        self._signal_mapper.setMapping(action, dock_widget)
        action.triggered.connect(self._signal_mapper.map)
        self._dock_widgets[dock_widget] = action
        self.addAction(action)

        self.show()

    def removeDockWidget(self, dock_widget):
        if dock_widget in self._dock_widgets:
            action = self._dock_widgets[dock_widget]
            self.removeAction(action)
            del self._dock_widgets[dock_widget]
            self._signal_mapper.removeMappings(action)

        if not self._dock_widgets:
            self.hide()

    def _on_action_triggered(self, dock_widget):
        # if the dock widget is nested inside a container also show the container
        # do this recursively for nested containers
        while True:
            parent = self._container_manager.get_container_of_dock_widget(dock_widget)
            if parent is None:
                break
            dock_widget.show()
            dock_widget = parent
        dock_widget.show()
