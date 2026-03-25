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

#ifndef GZ_PLUGIN_TEST_PLUGINS_FACTORYPLUGINS_HH_
#define GZ_PLUGIN_TEST_PLUGINS_FACTORYPLUGINS_HH_

#include <gz/plugin/Factory.hh>

#include "DummyPlugins.hh"

namespace test
{
namespace util
{
/// \brief A signature for factories that take a std::string and return a
/// DummyNameBase implementation
using NameFactory =
    gz::plugin::Factory<DummyNameBase, const std::string&>;

/// \brief A signature for factories that take a double and return a
/// DummyDoubleBase implementation
using DoubleFactory = gz::plugin::Factory<DummyDoubleBase, double>;

/// \brief A signature for factories that take an int and return a DummyIntBase
/// implementation
using IntFactory = gz::plugin::Factory<DummyIntBase, int>;

/// \brief A class whose constructor takes in arguments
class SomeObject
{
  public: inline SomeObject(int _iValue, double _dValue)
    : someInt(_iValue),
      someDouble(_dValue)
  {
    // Do nothing
  }

  public: virtual double SomeOperation() const = 0;

  public: virtual ~SomeObject() = default;

  public: int someInt;
  public: double someDouble;
};

/// \brief A signature for factories that take an int and double and then return
/// a SomeObject instance
using SomeObjectFactory = gz::plugin::Factory<SomeObject, int, double>;
}
}

#endif
