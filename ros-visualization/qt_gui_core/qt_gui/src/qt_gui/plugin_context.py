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

from python_qt_binding.QtCore import QObject


class PluginContext(QObject):
    """
    Provides information to the plugin and exposes methods to interact with the framework.

    PluginContext relays all methods to the corresponding `PluginHandler`.
    """

    def __init__(self, handler):
        super(PluginContext, self).__init__(handler)
        self.setObjectName('PluginContext')

        self._handler = handler

    def serial_number(self):
        """
        Return the serial number of the plugin.

        For a specific type of plugin each instance gets a serial number (which is the first
        currently not used positive integer at construction time).
        @return: The serial number
        @rtype: int
        """
        return self._handler.instance_id().serial_number

    def argv(self):
        """
        Return the command line arguments of the plugin.

        @return: The arguments without a program name at the beginning
        @rtype: list
        """
        return self._handler.argv()

    def add_widget(self, widget):
        """
        Add a widget to the UI.

        The widget is embedded into a new QDockWidget which itself is added to the QMainWindow.
        This method can be called once for each widget a plugin would like to add and at any point
        in time (until the calling plugin has been shutdown).

        Note: The ownership of the widget is transferred to the callee which will delete it when
            the plugin is shut down.
        @param widget: The widget to add
        @type widget: QWidget
        """
        self._handler.add_widget(widget)

    def remove_widget(self, widget):
        """
        Remove a previously added widget from the UI.

        Note: The ownership of the widget is transferred to the caller.
        @param widget: The widget to remove
        @type widget: QWidget
        """
        self._handler.remove_widget(widget)

    def add_toolbar(self, toolbar):
        """
        Add a toolbar to the UI, which is directly added to the QMainWindow.

        This method can be called once for each toolbar a plugin would like to add and at any point
        in time (until the calling plugin has been shutdown).
        Note: The ownership of the toolbar is transferred to the callee which will delete it when
              the plugin is shut down.
        @param widget: The toolbar to add
        @type widget: QToolBar
        """
        self._handler.add_toolbar(toolbar)

    def remove_toolbar(self, toolbar):
        """
        Remove a previously added toolbar from the UI.

        Note: The ownership of the toolbar is transferred to the caller.
        @param widget: The toolbar to remove
        @type widget: QToolBar
        """
        self._handler.remove_toolbar(toolbar)

    def close_plugin(self):
        """
        Close the plugin.

        The framework will call `Plugin.shutdown_plugin()` and unload it afterwards.
        """
        self._handler.close_plugin()

    def reload_plugin(self):
        """Reload the plugin."""
        self._handler.reload_plugin()
