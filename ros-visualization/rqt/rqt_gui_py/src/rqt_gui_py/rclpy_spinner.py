# Copyright 2018, PickNik Consulting
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from python_qt_binding.QtCore import qDebug, QThread
import rclpy
from rclpy.executors import MultiThreadedExecutor


class RclpySpinner(QThread):

    def __init__(self, node):
        super().__init__()
        self._node = node
        self._abort = False

    def run(self):
        qDebug('Start called on RclpySpinner, spinning ros2 node')
        executor = MultiThreadedExecutor()
        executor.add_node(self._node)
        while rclpy.ok() and not self._abort:
            executor.spin_once(timeout_sec=1.0)

    def quit(self):  # noqa: A003
        qDebug('Quit called on RclpySpinner')
        self._abort = True
        super().quit()
