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


class Plugin(QObject):
    """
    Interface for Python plugins.

    User-defined plugins may either subclass `qt_gui.plugin.Plugin` or according to duck typing
    implement only the needed methods.
    """

    def __init__(self, context):
        """Instantiate the plugin and pass the `PluginContext`."""
        super(Plugin, self).__init__(context)
        self.setObjectName('Plugin')

    def shutdown_plugin(self):
        """Shutdown and clean up the plugin before unloading."""
        pass

    def save_settings(self, plugin_settings, instance_settings):
        """
        Save the intrinsic state of the plugin to the plugin or instance specific `Settings`.

        @param plugin_settings: The plugin-specific settings
        @type plugin_settings: qt_gui.settings.Settings
        @param instance_settings: The instance-specific settings
        @type instance_settings: qt_gui.settings.Settings
        """
        pass

    def restore_settings(self, plugin_settings, instance_settings):
        """
        Restore the intrinsic state of the plugin from the plugin or instance specific `Settings`.

        @param plugin_settings: The plugin-specific settings
        @type plugin_settings: qt_gui.settings.Settings
        @param instance_settings: The instance-specific settings
        @type instance_settings: qt_gui.settings.Settings
        """
        pass

    # def trigger_configuration(self):
        # """
        # Trigger a configuration request from the title bar of one of the dock widgets.
        # If this method is available the `DockWidgetTitleBar` will show the configuration action.
        # """
        # pass
