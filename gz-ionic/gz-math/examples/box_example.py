# Copyright (C) 2024 Open Source Robotics Foundation
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

# This example will only work if the Python interface library was compiled and
# installed.
#
# Modify the PYTHONPATH environment variable to include the gz math
# library install path. For example, if you install to /usr:
#
# $ export PYTHONPATH=/usr/lib/python:$PYTHONPATH
#
from gz.math7 import (Boxd, MassMatrix3d, Material, MaterialType, Planed, Vector3d)

box = Boxd()
plane = Planed(Vector3d(0, 0, 1), 0.5)

size = box.size()
print("Default box size:\nLength: {} Width: {} Height: {}".format(size[0], size[1], size[2]))

# Set new size for the box
box.set_size(4.0, 4.0, 3.0)
size = box.size()
print("Updated box size:\nLength: {} Width: {} Height: {}".format(size[0], size[1], size[2]))

# Set the material for the box
print("Default box material: {}".format(box.material().name()))
wood = Material(MaterialType.WOOD)
box.set_material(wood)
print("Updated box material: {}".format(box.material().name()))

# Output the volume of the box
print("Volume: {}".format(box.volume()))

# Output the mass matrix of the box
mass_matrix = box.mass_matrix().moi()
print("Inertial matrix:")
for i in range(3):
    for j in range(3):
        print(mass_matrix(i, j), end=" ")
    print()

# Intersection edges of plane in box
intersection_points = box.intersections(plane)
print("Intersection points:")
for point in intersection_points:
    print("x: {} y: {} z: {}".format(point[0], point[1], point[2]))

# Vertices of the box below the plane
vertices_below = box.vertices_below(plane)
print("Vertices Below:")
for point in vertices_below:
    print("x: {} y: {} z: {}".format(point[0], point[1], point[2]))

# Center of volume below the plane
cov = box.center_of_volume_below(plane)
print("Center of volume below:")
print("x: {} y: {} z: {}".format(cov[0], cov[1], cov[2]))
