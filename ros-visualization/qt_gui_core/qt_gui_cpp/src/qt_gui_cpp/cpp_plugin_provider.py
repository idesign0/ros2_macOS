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

from qt_gui.composite_plugin_provider import CompositePluginProvider

from .cpp_binding_helper import qt_gui_cpp
from .ros_pluginlib_plugin_provider import RosPluginlibPluginProvider


class CppPluginProvider(CompositePluginProvider):

    def __init__(self):
        plugin_providers = None
        if qt_gui_cpp is not None:
            plugin_providers = [
                RosPluginlibPluginProvider(
                    qt_gui_cpp.RosPluginlibPluginProvider_ForPlugins(
                        'qt_gui', 'qt_gui_cpp::Plugin')),
                RosPluginlibPluginProvider(qt_gui_cpp.RecursivePluginProvider(
                    qt_gui_cpp.RosPluginlibPluginProvider_ForPluginProviders.create_instance(
                        'qt_gui', 'qt_gui_cpp::PluginProvider'))),
            ]
        super(CppPluginProvider, self).__init__(plugin_providers)
