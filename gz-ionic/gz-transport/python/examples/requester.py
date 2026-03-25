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

#! [complete]
from gz.msgs11.stringmsg_pb2 import StringMsg
from gz.transport14 import Node

def main():
    node = Node()
    service_name = "/echo"
    request = StringMsg()
    request.data = "Hello world"
    response = StringMsg()
    timeout = 5000

    result, response = node.request(service_name, request, StringMsg, StringMsg, timeout)
    print("Result:", result, "\nResponse:", response.data)

#! [complete]

if __name__ == "__main__":
    main()
