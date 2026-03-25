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
from gz.msgs11.vector3d_pb2 import Vector3d
from gz.transport14 import Node

import time

def main():
    node = Node()
    stringmsg_topic = "/example_stringmsg_topic"
    vector3d_topic = "/example_vector3d_topic"
    pub_stringmsg = node.advertise(stringmsg_topic, StringMsg)
    pub_vector3d = node.advertise(vector3d_topic, Vector3d)

    vector3d_msg = Vector3d()
    vector3d_msg.x = 10
    vector3d_msg.y = 15
    vector3d_msg.z = 20

    stringmsg_msg = StringMsg()
    stringmsg_msg.data = "Hello" 
    try:
        count = 0
        while True:
          count += 1
          vector3d_msg.x = count
          if not pub_stringmsg.publish(stringmsg_msg):
              break
          print("Publishing 'Hello' on topic [{}]".format(stringmsg_topic))
          if not pub_vector3d.publish(vector3d_msg):
              break
          print("Publishing a Vector3d on topic [{}]".format(vector3d_topic))
          time.sleep(0.1)

    except KeyboardInterrupt:
        pass

#! [complete]

if __name__ == "__main__":
    main()
