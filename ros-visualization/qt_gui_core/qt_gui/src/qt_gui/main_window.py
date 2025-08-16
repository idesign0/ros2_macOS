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

from python_qt_binding.QtCore import qDebug, QRect, Qt, Signal
from python_qt_binding.QtGui import QFont
from python_qt_binding.QtWidgets import QTextBrowser, QToolBar

from qt_gui.dockable_main_window import DockableMainWindow
from qt_gui.settings import Settings


class MainWindow(DockableMainWindow):
    """Main window of the application managing the geometry and state of all top-level widgets."""

    save_settings_before_close_signal = Signal(Settings, Settings)

    def __init__(self, help_text=None):
        super(MainWindow, self).__init__()
        self.setObjectName('MainWindow')
        self._help_widget = None

        if help_text:
            font = QFont()
            font.setPointSize(14)

            self._help_widget = QTextBrowser(self)
            self._help_widget.setFont(font)
            self._help_widget.setReadOnly(True)
            self._help_widget.setTextInteractionFlags(Qt.TextBrowserInteraction)
            self._help_widget.setOpenExternalLinks(True)
            self._help_widget.setHtml(help_text)
            self._help_widget.setStyleSheet('background:transparent;')

        self._save_on_close_signaled = False
        self._global_settings = None
        self._perspective_settings = None
        self._settings = None

    def showHelpWidget(self, should_show):
        if self._help_widget:
            self._help_widget.show() if should_show else self._help_widget.hide()

    def resizeEvent(self, event):
        if self._help_widget:
            size = self.size()
            info_percentage = 0.75
            margin_percentage = (1.0 - info_percentage) / 2
            self._help_widget.setGeometry(
                QRect(int(size.width() * margin_percentage),
                      int(size.height() * margin_percentage),
                      int(size.width() * info_percentage),
                      int(size.height() * info_percentage)))

    def closeEvent(self, event):
        qDebug('MainWindow.closeEvent()')
        if not self._save_on_close_signaled:
            self._save_geometry_to_perspective()
            self._save_state_to_perspective()
            self._save_on_close_signaled = True
            self.save_settings_before_close_signal.emit(
                self._global_settings, self._perspective_settings)
            event.ignore()
        else:
            event.accept()

    def save_settings(self, global_settings, perspective_settings):
        qDebug('MainWindow.save_settings()')
        self._global_settings = global_settings
        self._perspective_settings = perspective_settings
        self._settings = self._perspective_settings.get_settings('mainwindow')
        # store current setup to current _settings
        self._save_geometry_to_perspective()
        self._save_state_to_perspective()

    def restore_settings(self, global_settings, perspective_settings):
        qDebug('MainWindow.restore_settings()')
        # remember new _settings
        self._global_settings = global_settings
        self._perspective_settings = perspective_settings
        self._settings = self._perspective_settings.get_settings('mainwindow')
        # only restore geometry, restoring state is triggered after PluginManager has been updated
        self._restore_geometry_from_perspective()

    def save_setup(self):
        qDebug('MainWindow.save_setup()')
        # store current setup to current _settings
        self._save_geometry_to_perspective()
        self._save_state_to_perspective()

    def restore_state(self):
        qDebug('MainWindow.restore_state()')
        # restore state from _settings
        self._restore_state_from_perspective()

    def perspective_changed(self, name):
        self.setWindowTitle('%s - rqt' % str(name))

    def _save_geometry_to_perspective(self):
        if self._settings is not None:
            # unmaximizing widget before saveGeometry works around bug to restore dock-widgets
            # still the non-maximized size can not correctly be restored
            maximized = self.isMaximized()
            if maximized:
                self.showNormal()
            self._settings.set_value('geometry', self.saveGeometry())
            if maximized:
                self.showMaximized()

    def _restore_geometry_from_perspective(self):
        if self._settings.contains('geometry'):
            self.restoreGeometry(self._settings.value('geometry'))

    def _save_state_to_perspective(self):
        if self._settings is not None:
            self._settings.set_value('state', self.saveState())
            # safe area for all toolbars
            toolbar_settings = self._settings.get_settings('toolbar_areas')
            for toolbar in self.findChildren(QToolBar):
                area = self.toolBarArea(toolbar)
                if area in [Qt.LeftToolBarArea,
                            Qt.RightToolBarArea,
                            Qt.TopToolBarArea,
                            Qt.BottomToolBarArea]:
                    toolbar_settings.set_value(toolbar.objectName(), area)

    def _restore_state_from_perspective(self):
        if self._settings.contains('state'):
            self.restoreState(self._settings.value('state'))
            # restore area for all toolbars
            toolbar_settings = self._settings.get_settings('toolbar_areas')
            for toolbar in self.findChildren(QToolBar):
                if not toolbar.objectName():
                    continue
                area = Qt.ToolBarArea(
                    int(toolbar_settings.value(toolbar.objectName(), Qt.NoToolBarArea)))
                if area in [Qt.LeftToolBarArea,
                            Qt.RightToolBarArea,
                            Qt.TopToolBarArea,
                            Qt.BottomToolBarArea]:
                    self.addToolBar(area, toolbar)
