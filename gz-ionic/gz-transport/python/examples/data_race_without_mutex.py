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

from gz.msgs11.vector3d_pb2 import Vector3d
from gz.transport14 import Node

import time

x = 1
y = 2
z = 3

def vector3_cb(msg: Vector3d):
    global x, y, z
    x = msg.x
    y = msg.y
    z = msg.z

def main():
    global x, y, z
    # create a transport node
    node = Node()
    topic_vector3d = "/vector3d_topic"

    pub_vector3d = node.advertise(topic_vector3d, Vector3d)

    vector3d_msg = Vector3d()
    vector3d_msg.x = 1
    vector3d_msg.y = 2
    vector3d_msg.z = 3

    # subscribe to a topic by registering a callback
    if node.subscribe(Vector3d, topic_vector3d, vector3_cb):
        print("Subscribing to type {} on topic [{}]".format(
            Vector3d, topic_vector3d))
    else:
        print("Error subscribing to topic [{}]".format(topic_vector3d))
        return

    # wait for shutdown
    try:
        count = 1
        while True:
            count += 1
            vector3d_msg.x = vector3d_msg.x*count
            vector3d_msg.y = vector3d_msg.y*count
            vector3d_msg.z = vector3d_msg.z*count
            pub_vector3d.publish(vector3d_msg)
            if ((vector3d_msg.x - x) != 0) or ((vector3d_msg.y - y) != 0) or ((vector3d_msg.z - z) != 0):
                print("Race Condition happened")
            time.sleep(0.01)
    except KeyboardInterrupt:
      pass
    print("Done")

if __name__ == "__main__":
    main()
