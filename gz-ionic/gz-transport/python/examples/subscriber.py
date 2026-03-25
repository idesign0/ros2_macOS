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

def stringmsg_cb(msg: StringMsg):
    print("Received StringMsg: [{}]".format(msg.data))

def vector3_cb(msg: Vector3d):
    print("Received Vector3: [x: {}, y: {}, z: {}]".format(msg.x, msg.y, msg.z))

def main():
    # create a transport node
    node = Node()
    topic_stringmsg = "/example_stringmsg_topic"
    topic_vector3d = "/example_vector3d_topic"

    # subscribe to a topic by registering a callback
    if node.subscribe(StringMsg, topic_stringmsg, stringmsg_cb):
        print("Subscribing to type {} on topic [{}]".format(
            StringMsg, topic_stringmsg))
    else:
        print("Error subscribing to topic [{}]".format(topic_stringmsg))
        return

    # subscribe to a topic by registering a callback
    if node.subscribe(Vector3d, topic_vector3d, vector3_cb):
        print("Subscribing to type {} on topic [{}]".format(
            Vector3d, topic_vector3d))
    else:
        print("Error subscribing to topic [{}]".format(topic_vector3d))
        return

    # wait for shutdown
    try:
      while True:
        time.sleep(0.001)
    except KeyboardInterrupt:
      pass
    print("Done")

#! [complete]

if __name__ == "__main__":
    main()
