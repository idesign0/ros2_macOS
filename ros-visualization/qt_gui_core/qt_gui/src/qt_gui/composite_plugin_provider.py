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

import traceback

from python_qt_binding.QtCore import qCritical

from qt_gui.plugin_provider import PluginProvider


class CompositePluginProvider(PluginProvider):
    """Composite of multiple `PluginProvider`s."""

    def __init__(self, plugin_providers=None):
        super(CompositePluginProvider, self).__init__()
        self.setObjectName('CompositePluginProvider')

        self._plugin_providers = plugin_providers or []
        self._discovered_plugins = {}
        self._running_plugins = {}

    def set_plugin_providers(self, plugin_providers):
        self._plugin_providers = plugin_providers

    def discover(self, discovery_data):
        # discover plugins from all providers
        discovered_plugins = []
        for plugin_provider in self._plugin_providers:
            try:
                plugin_descriptors = plugin_provider.discover(discovery_data)
            except Exception:
                qCritical(
                    'CompositePluginProvider.discover() could not discover plugins from '
                    'provider "%s":\n%s' % (type(plugin_provider), traceback.format_exc()))
            else:
                self._discovered_plugins[plugin_provider] = plugin_descriptors
                discovered_plugins += plugin_descriptors
        return discovered_plugins

    def load(self, plugin_id, plugin_context):
        # dispatch load to appropriate provider
        for plugin_provider, plugin_descriptors in self._discovered_plugins.items():
            for plugin_descriptor in plugin_descriptors:
                if plugin_descriptor.plugin_id() == plugin_id:
                    instance = plugin_provider.load(plugin_id, plugin_context)
                    self._running_plugins[instance] = plugin_provider
                    return instance
        raise UserWarning('plugin_id "%s" not found' % plugin_id)

    def unload(self, plugin_instance):
        # dispatch unload to appropriate provider
        if plugin_instance in self._running_plugins:
            self._running_plugins[plugin_instance].unload(plugin_instance)
            self._running_plugins.pop(plugin_instance)
            return
        raise UserWarning('plugin_instance not found')

    def shutdown(self):
        for plugin_provider in self._plugin_providers:
            plugin_provider.shutdown()
