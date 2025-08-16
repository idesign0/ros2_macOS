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

import os

from python_qt_binding.QtCore import qDebug, qWarning
from qt_gui.composite_plugin_provider import CompositePluginProvider

import rclpy
from rqt_gui.ros2_plugin_context import Ros2PluginContext
from rqt_gui.rospkg_plugin_provider import RospkgPluginProvider
from rqt_gui_py.rclpy_spinner import RclpySpinner


class RosPyPluginProvider(CompositePluginProvider):

    def __init__(self):
        super(RosPyPluginProvider, self).__init__(
            [RospkgPluginProvider('rqt_gui', 'rqt_gui_py::Plugin')])
        self.setObjectName('RosPyPluginProvider')
        self._node_initialized = False
        self._node = None
        self._spinner = None
        self._shutdown_timeout = 2000

    def shutdown(self):
        qDebug('Shutting down RosPyPluginProvider')
        if self._spinner:
            self._spinner.quit()
            joined = self._spinner.wait(msecs=self._shutdown_timeout)
            if not joined:
                qWarning('Timed out attempting to join the RclpySpinner thread')
                return
        if self._node:
            self._destroy_node()
        super().shutdown()

    def load(self, plugin_id, plugin_context):
        self._init_node()
        ros_plugin_context = Ros2PluginContext(handler=plugin_context._handler, node=self._node)

        return super(RosPyPluginProvider, self).load(plugin_id, ros_plugin_context)

    def unload(self, plugin_instance):
        return super(RosPyPluginProvider, self).unload(plugin_instance)

    def _init_node(self):
        # initialize node once
        if not self._node_initialized:
            name = 'rqt_gui_py_node_%d' % os.getpid()
            qDebug('RosPyPluginProvider._init_node() initialize ROS node "%s"' % name)
            if not rclpy.ok():
                rclpy.init()
            self._node = rclpy.create_node(name)
            self._spinner = RclpySpinner(self._node)
            self._spinner.start()
            self._node_initialized = True

    def _destroy_node(self):
        if self._node_initialized:
            self._node.destroy_node()
            rclpy.try_shutdown()
            self._node_initialized = False
