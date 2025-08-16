# Copyright (c) 2011, Dirk Thomas, TU Darmstadt
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

from qt_gui.errors import PluginLoadError
from qt_gui.plugin_descriptor import PluginDescriptor
from qt_gui.plugin_provider import PluginProvider

from .cpp_binding_helper import qt_gui_cpp


class RosPluginlibPluginProvider(PluginProvider):

    def __init__(self, plugin_provider):
        super(RosPluginlibPluginProvider, self).__init__()
        self._plugin_provider = plugin_provider

    def discover(self, discovery_data):
        discovered_plugins = self._unfold(self._plugin_provider.discover(discovery_data))
        plugin_descriptors = []
        for plugin in discovered_plugins.values():
            plugin_descriptor = PluginDescriptor(plugin['plugin_id'], plugin['attributes'])

            action_attributes = plugin['action']
            plugin_descriptor.set_action_attributes(
                action_attributes['label'],
                action_attributes.get('statustip', None),
                action_attributes.get('icon', None),
                action_attributes.get('icontype', None))

            groups = plugin.get('groups', {})
            for group in groups.values():
                plugin_descriptor.add_group_attributes(
                    group['label'], group['statustip'], group['icon'], group['icontype'])

            plugin_descriptors.append(plugin_descriptor)
        return plugin_descriptors

    def load(self, plugin_id, plugin_context):
        if qt_gui_cpp is None:
            return None
        cpp_plugin_context = None
        if plugin_context is not None:
            cpp_plugin_context = qt_gui_cpp.PluginContext(
                plugin_context._handler, plugin_context.serial_number(), plugin_context.argv())
        bridge = qt_gui_cpp.PluginBridge()  # @UndefinedVariable
        loaded = bridge.load_plugin(self._plugin_provider, plugin_id, cpp_plugin_context)
        if not loaded:
            raise PluginLoadError(
                'RosPluginlibPluginProvider.load() could not load plugin "%s"' % plugin_id)
        return bridge

    def unload(self, bridge):
        return bridge.unload_plugin()

    def _unfold(self, flat_dict):
        dictionary = {}
        for key, value in flat_dict.items():
            keys = str(key).split('.')
            current_level = dictionary
            for i in keys[:-1]:
                if i not in current_level:
                    current_level[i] = {}
                current_level = current_level[i]
            current_level[keys[-1]] = str(value) if value != '' else None
        return dictionary

    def shutdown(self):
        self._plugin_provider.shutdown()
