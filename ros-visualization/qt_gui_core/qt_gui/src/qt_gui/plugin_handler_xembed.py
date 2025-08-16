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

from qt_gui.plugin_handler_xembed_client import PluginHandlerXEmbedClient
from qt_gui.plugin_handler_xembed_container import PluginHandlerXEmbedContainer


class PluginHandlerXEmbed():
    """
    Handler for forwarding invocations between the framework and one `Plugin` instance.

    Handler uses peer-to-peer DBus connection. The both DBus endpoints are realized by the
    `PluginHandlerXEmbedContainer` and the `PluginHandlerXEmbedClient`.
    """

    def __init__(self, parent, main_window, instance_id,
                 application_context, container_manager, argv):
        dbus_object_path = '/PluginHandlerXEmbed/plugin/' + instance_id.tidy_str()
        if application_context.options.embed_plugin is None:
            self._handler = PluginHandlerXEmbedContainer(
                parent, main_window, instance_id, application_context,
                container_manager, argv, dbus_object_path)
        else:
            self._handler = PluginHandlerXEmbedClient(
                parent, main_window, instance_id, application_context,
                container_manager, argv, dbus_object_path)

    def __getattr__(self, name):
        return getattr(self._handler, name)
