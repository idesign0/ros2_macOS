/*
 * Copyright (C) 2024 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <gz/math/Box.hh>
#include <gz/math/Plane.hh>
#include <iostream>

int main(int argc, char **argv) {

  // Create a box
  gz::math::Boxd box;

  // A default constructed box size should be zero.
  auto size = box.Size();
  std::cout << "Default box size: \nLength: " << size[0]
            << " Width: " << size[1] << " Height: " << size[2] << std::endl;

  // Set new size for the box
  box.SetSize(4.0, 4.0, 3.0);
  size = box.Size();
  std::cout << "Updated box size: \nLength: " << size[0]
            << " Width: " << size[1] << " Height: " << size[2] << std::endl;
  size = box.Size();

  // Set the material for the box
  box.Material();
  std::cout << "Default box material: " << box.Material().Name() << std::endl;
  box.SetMaterial(gz::math::Material("wood"));
  std::cout << "Updated box material: " << box.Material().Name() << std::endl;

  // Output the volume of the box
  std::cout << "Volume: " << box.Volume() << std::endl;

  // Output the mass matrix of the box
  gz::math::Matrix3 massMatrix = box.MassMatrix()->Moi();
  std::cout << "Inertial matrix: " << std::endl;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      std::cout << massMatrix(i, j) << " ";
    }
    std::cout << std::endl;
  }

  // Intersection edges of plane in box
  auto plane = gz::math::Plane<double>({0, 0, 1}, 0.5);
  auto intersectionPoints = box.Intersections(plane);
  std::cout << "Intersection points: " << std::endl;
  for (auto point : intersectionPoints) {
    std::cout << "x: " << point[0] << " y: " << point[1] << " z: " << point[2] << std::endl;
  }

  // Vertices of the box below the plane
  auto vertices = box.VerticesBelow(plane);
  std::cout << "Vertices Below: " << std::endl;
  for (auto point : vertices) {
    std::cout << "x: " << point[0] << " y: " << point[1] << " z: " << point[2] << std::endl;
  }

  // Center of volume below the plain
  auto cov = box.CenterOfVolumeBelow(plane);
  std::cout << "Center of volume below: " << std::endl;
  std::cout << "x: " << cov.value()[0] << " y: " << cov.value()[1]
            << " z: " << cov.value()[2] << std::endl;
}
