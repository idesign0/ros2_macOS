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

import os
import platform
import sys

from python_qt_binding import QT_BINDING, QT_BINDING_MODULES, QT_BINDING_VERSION
from python_qt_binding.QtCore import QObject, qVersion
from python_qt_binding.QtWidgets import QMessageBox

from .ros_package_helper import get_package_path


class AboutHandler(QObject):
    """
    Handler for the 'About action' menu bar item.

    This handler shows a message box with details on the used libraries and their versions.
    """

    def __init__(self, qtgui_path, parent=None):
        super(AboutHandler, self).__init__(parent)
        self._qtgui_path = qtgui_path

    def show(self):
        try:
            # append folder of 'qt_gui_cpp/lib' to module search path
            qt_gui_cpp_path = os.path.realpath(get_package_path('qt_gui_cpp'))
        except Exception:
            qt_gui_cpp = None
        else:
            sys.path.append(os.path.join(qt_gui_cpp_path, 'lib'))
            sys.path.append(os.path.join(qt_gui_cpp_path, 'src'))
            from qt_gui_cpp.cpp_binding_helper import qt_gui_cpp

        logo = os.path.join(
            self._qtgui_path, 'share', 'qt_gui', 'resource', 'ros_org_vertical.png')
        text = '<img src="%s" width="56" height="200" style="float: left;"/>' % logo

        text += '<h3 style="margin-top: 1px;">%s</h3>' % self.tr('rqt')

        text += '<p>%s %s</p>' % (
            self.tr('rqt is a framework for graphical user interfaces.'),
            self.tr('It is extensible with plugins which can be written in either Python or C++.'))
        text += '<p>%s</p>' % (
            self.tr(
                'Please see the <a href="%s">Wiki</a> for more information on rqt and available '
                'plugins.' % 'http://wiki.ros.org/rqt'))

        text += '<p>%s: ' % self.tr('Utilized libraries:')

        text += 'Python %s, ' % platform.python_version()

        if QT_BINDING == 'pyside':
            text += 'PySide'
        elif QT_BINDING == 'pyqt':
            text += 'PyQt'
        text += ' %s (%s), ' % (QT_BINDING_VERSION, ', '.join(sorted(QT_BINDING_MODULES)))

        text += 'Qt %s, ' % qVersion()

        if qt_gui_cpp is not None:
            if QT_BINDING == 'pyside':
                text += '%s' % (self.tr('%s C++ bindings available') % 'Shiboken')
            elif QT_BINDING == 'pyqt':
                text += '%s' % (self.tr('%s C++ bindings available') % 'SIP')
            else:
                text += '%s' % self.tr('C++ bindings available')
        else:
            text += '%s' % self.tr('no C++ bindings found - no C++ plugins available')

        text += '.</p>'

        mb = QMessageBox(
            QMessageBox.NoIcon, self.tr('About rqt'), text, QMessageBox.Ok, self.parent())
        mb.exec_()
