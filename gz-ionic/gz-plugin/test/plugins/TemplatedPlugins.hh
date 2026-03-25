/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#ifndef GZ_PLUGIN_TEST_PLUGINS_TEMPLATEPLUGIN_HH_
#define GZ_PLUGIN_TEST_PLUGINS_TEMPLATEPLUGIN_HH_

namespace test
{
namespace plugins
{

// Interface class to test output from templated interfaces
template <typename T>
class TemplatedGetInterface
{
  public: virtual T Get() const = 0;
};

// Interface class to test input to templated interfaces
template <typename T>
class TemplatedSetInterface
{
  public: virtual void Set(const T &_value) = 0;
};

}
}

#endif
