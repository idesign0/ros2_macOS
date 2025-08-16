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

from qt_gui.composite_plugin_provider import CompositePluginProvider


class RecursivePluginProvider(CompositePluginProvider):
    """
    Plugin provider which directly loads and returns all discovered plugins.

    Discovered plugins should be plugin providers themselves.
    """

    def __init__(self, plugin_provider):
        super(RecursivePluginProvider, self).__init__([])
        self.setObjectName('RecursivePluginProvider')

        self._plugin_provider = plugin_provider

    def discover(self, discovery_data):
        # discover plugins, which are providers themselves
        plugin_descriptors = self._plugin_provider.discover(discovery_data)

        # instantiate plugins
        plugin_providers = []
        for plugin_descriptor in plugin_descriptors:
            try:
                # pass None as PluginContext for PluginProviders
                instance = self._plugin_provider.load(plugin_descriptor.plugin_id(), None)
            except Exception:
                qCritical('RecursivePluginProvider.discover() loading plugin "%s" failed:\n%s' %
                          (str(plugin_descriptor.plugin_id()), traceback.format_exc()))
            else:
                if instance is not None:
                    plugin_providers.append(instance)

        # delegate discovery through instantiated plugin providers to base class
        self.set_plugin_providers(plugin_providers)
        return CompositePluginProvider.discover(self, discovery_data)
