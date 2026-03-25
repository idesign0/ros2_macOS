/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include "bad_plugins.hh"
#include <gz/plugin/Register.hh>

#include <iostream>

SegfaultOnLoad::SegfaultOnLoad(): gz::launch::Plugin() {}

bool SegfaultOnLoad::Load(const tinyxml2::XMLElement * /*_elem*/)
{
  char* a = 0;
  // cppcheck-suppress nullPointer
  char b = a[42];
  std::cout << "result: " << b << std::endl;
  return true;
}

GZ_ADD_PLUGIN(SegfaultOnLoad, gz::launch::Plugin)
