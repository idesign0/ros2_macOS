# Copyright (C) 2023 Open Source Robotics Foundation
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
#

from gz.msgs11.int32_pb2 import Int32
from gz.msgs11.stringmsg_pb2 import StringMsg
from gz.transport14 import Node

import os
import subprocess
import unittest


class RequesterTEST(unittest.TestCase):
    def setUp(self):
        # Environment Setup
        gz_partition = "python_requester_test"
        os.environ["GZ_PARTITION"] = gz_partition

        # Subprocess Setup
        cmd = f"{os.getenv('CMAKE_BINARY_DIR')}/twoProcsSrvCallReplier_aux {gz_partition}"
        self.service_process = subprocess.Popen(cmd, shell=True)

        # Requester Setup
        self.node = Node()
        self.service_name = "/foo"
        self.request = Int32()
        self.request.data = 100
        self.timeout = 2000

    def tearDown(self):
        self.service_process.kill()

    # Checks that the node is able to create a service request.
    def test_request(self):
        result, response = self.node.request(
            self.service_name, self.request, Int32, Int32, self.timeout
        )
        self.assertTrue(result)
        self.assertEqual(response.data, self.request.data)

    # Checks that the node is unable to create a service request if the types
    # of the request and response are wrong.
    def test_wrong_type(self):
        result, response = self.node.request(
            self.service_name, self.request, StringMsg, Int32, self.timeout
        )
        self.assertFalse(result)
        self.assertNotEqual(response.data, self.request.data)

        result, response = self.node.request(
            self.service_name, self.request, Int32, StringMsg, self.timeout
        )
        self.assertFalse(result)
        self.assertNotEqual(response.data, self.request.data)

        result, response = self.node.request(
            self.service_name, self.request, StringMsg, StringMsg, self.timeout
        )
        self.assertFalse(result)
        self.assertNotEqual(response.data, self.request.data)

    # Checks that the node is able to retrieve the list of services.
    def test_service_list(self):
        services = self.node.service_list()
        self.assertTrue(services)
        self.assertEqual(len(services), 1)

    # Checks that the node is able to retrieve the information of a service.
    def test_service_info(self):
        service_info_list = self.node.service_info('/service_no_responser')
        self.assertEqual(len(service_info_list), 0)
        service_info_list = self.node.service_info(self.service_name)
        self.assertEqual(service_info_list[0].req_type_name, 'gz.msgs.Int32')
        self.assertEqual(service_info_list[0].rep_type_name, 'gz.msgs.Int32')
