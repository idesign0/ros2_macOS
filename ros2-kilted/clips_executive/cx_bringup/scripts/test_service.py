#!/usr/bin/env python3
# Copyright (c) 2024-2026 Carologistics
# SPDX-License-Identifier: Apache-2.0
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

import rclpy
from rclpy.node import Node
from std_srvs.srv import SetBool


class SetBoolService(Node):

    def __init__(self):
        super().__init__('set_bool_service')
        self.srv = self.create_service(SetBool, 'ros_cx_client', self.set_bool_callback)

    def set_bool_callback(self, request, response):
        if request.data:
            response.success = True
            response.message = 'The request was true!'
        else:
            response.success = False
            response.message = 'The request was false!'
        self.get_logger().info(f'Received request: {request.data}')
        return response


def main(args=None):
    rclpy.init(args=args)
    node = SetBoolService()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
