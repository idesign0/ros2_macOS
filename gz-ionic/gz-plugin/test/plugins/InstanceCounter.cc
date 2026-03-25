/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include <gz/plugin/Register.hh>

#include "InstanceCounter.hh"

namespace test
{
namespace util
{

namespace {

/////////////////////////////////////////////////
int numInstancesImpl() {
  // Use a static variable that never gets deleted as a way to demonstrate
  // that plugins with RTLD_NODELETE are not deleted when unloaded.
  static int *instances = new int(0);
  ++(*instances);
  return *instances;
}
}

/// \brief A class that counts the number of times a plugin has been 
/// instantiated.
class InstanceCounter : public InstanceCounterBase
{
  public: InstanceCounter() : instances(numInstancesImpl())
  {
  }

  public: ~InstanceCounter() override = default;

  public: int Instances() override
  {
    return this->instances;
  }

  private: int instances;
};

GZ_ADD_PLUGIN(InstanceCounter, InstanceCounterBase)
}  // namespace util
}  // namespace test
