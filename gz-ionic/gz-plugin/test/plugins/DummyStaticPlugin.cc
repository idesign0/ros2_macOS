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

#include "gz/plugin/RegisterStatic.hh"
#include "DummyPlugins.hh"

namespace test
{
namespace util
{

class DummySinglePlugin : public DummyNameBase
{
  public: virtual std::string MyNameIs() const override
  {
    return std::string("DummySinglePlugin");
  }
};

}
}

GZ_ADD_STATIC_PLUGIN(test::util::DummySinglePlugin, test::util::DummyNameBase)
GZ_ADD_STATIC_PLUGIN_ALIAS(test::util::DummySinglePlugin, "Alternative name")
GZ_ADD_STATIC_PLUGIN_ALIAS(test::util::DummySinglePlugin, "Bar", "Baz")
