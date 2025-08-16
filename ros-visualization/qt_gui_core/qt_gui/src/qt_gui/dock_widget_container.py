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

from python_qt_binding.QtGui import QIcon

from qt_gui.dock_widget import DockWidget
from qt_gui.dockable_main_window import DockableMainWindow


class DockWidgetContainer(DockWidget):
    """`DockWidget` containing a main window acting as a container for other dock widgets."""

    def __init__(self, container_manager, serial_number):
        super(DockWidgetContainer, self).__init__(container_manager)
        self._serial_number = serial_number
        self._settings = None

        self.main_window = DockableMainWindow()
        self.setWidget(self.main_window)
        self.setWindowIcon(QIcon.fromTheme('folder'))

    def serial_number(self):
        return self._serial_number

    def setObjectName(self, name):
        super(DockWidget, self).setObjectName(name)
        self.main_window.setObjectName(name + '__MainWindow')

    def save_settings(self, settings):
        mw_settings = settings.get_settings('mainwindow')
        self._save_geometry(mw_settings)
        self._save_state(mw_settings)
        super(DockWidgetContainer, self).save_settings(settings)

    def _save_geometry(self, settings):
        # unmaximizing widget before saveGeometry works around bug to restore dock-widgets
        # still the non-maximized size can not correctly be restored
        maximized = self.isMaximized()
        if maximized:
            self.showNormal()
        settings.set_value('geometry', self.main_window.saveGeometry())
        if maximized:
            self.showMaximized()

    def _save_state(self, settings):
        if self._settings is not None:
            self._settings.set_value('state', self.main_window.saveState())

    def restore_settings(self, settings):
        super(DockWidgetContainer, self).restore_settings(settings)
        mw_settings = settings.get_settings('mainwindow')
        self._settings = mw_settings
        # only restore geometry, restoring state is triggered after PluginManager has been updated
        self._restore_geometry(mw_settings)

    def _restore_geometry(self, settings):
        if settings.contains('geometry'):
            self.main_window.restoreGeometry(settings.value('geometry'))

    def restore_state(self):
        if self._settings.contains('state'):
            self.main_window.restoreState(self._settings.value('state'))
