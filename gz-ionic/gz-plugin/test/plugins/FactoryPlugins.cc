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

#include <gz/plugin/Register.hh>

#include "FactoryPlugins.hh"

namespace test
{
namespace util
{
/// \brief An implementation that takes in a std::string and returns that same
/// value.
class DummyNameForward : public DummyNameBase
{
  public: DummyNameForward(const std::string &_name)
    : name(_name)
  {
    // Do nothing
  }

  public: std::string MyNameIs() const override
  {
    return name;
  }

  private: const std::string name;
};
GZ_ADD_FACTORY(DummyNameForward, NameFactory)

/// \brief An implementation that takes in a std::string and says hello to it.
class DummyNameSayHello : public DummyNameBase
{
  public: DummyNameSayHello(const std::string &_name)
    : name(_name)
  {
    // Do nothing
  }

  public: std::string MyNameIs() const override
  {
    return "Hello, " + name + "!";
  }

  private: const std::string name;
};
GZ_ADD_FACTORY(DummyNameSayHello, NameFactory)

/// \brief An implementation that takes in a double and returns that same value.
class DummyDoubleForward : public DummyDoubleBase
{
  public: DummyDoubleForward(const double _value)
    : value(_value)
  {
    // Do nothing
  }

  public: double MyDoubleValueIs() const override
  {
    return value;
  }

  private: const double value;
};
GZ_ADD_FACTORY(DummyDoubleForward, DoubleFactory)

/// \brief An implementation takes in a double and returns that same value plus
/// one-half.
class DummyDoubleAddOneHalf : public DummyDoubleBase
{
  public: DummyDoubleAddOneHalf(const double _value)
    : value(_value + 0.5)
  {
    // Do nothing
  }

  public: double MyDoubleValueIs() const override
  {
    return value;
  }

  private: const double value;
};
GZ_ADD_FACTORY(DummyDoubleAddOneHalf, DoubleFactory)

/// \brief An implementation that takes in an int and returns that same value.
class DummyIntForward : public DummyIntBase
{
  public: DummyIntForward(const int _value)
    : value(_value)
  {
    // Do nothing
  }

  public: int MyIntegerValueIs() const override
  {
    return value;
  }

  private: const int value;
};
GZ_ADD_FACTORY(DummyIntForward, IntFactory)

/// \brief An implementation takes in an int and returns that same value plus
/// one.
class DummyIntAddOne : public DummyIntBase
{
  public: DummyIntAddOne(const int _value)
    : value(_value + 1)
  {
    // Do nothing
  }

  public: int MyIntegerValueIs() const override
  {
    return value;
  }

  private: const int value;
};
GZ_ADD_FACTORY(DummyIntAddOne, IntFactory)

/// \brief An implementation of SomeObject that just sets the values that get
/// passed to it.
class SomeObjectForward : public SomeObject
{
  public: SomeObjectForward(int _intValue, double _doubleValue)
    : SomeObject{_intValue, _doubleValue}
  {
    // Do nothing
  }

  public: double SomeOperation() const override
  {
    return this->someInt + this->someDouble;
  }
};
GZ_ADD_FACTORY(SomeObjectForward, SomeObjectFactory)

/// \brief An implementation of SomeObject that adds two to each value that gets
/// passed to it.
class SomeObjectAddTwo : public SomeObject
{
  public: SomeObjectAddTwo(int _intValue, double _doubleValue)
    : SomeObject{_intValue + 2, _doubleValue + 2.0}
  {
    // Do nothing
  }

  public: double SomeOperation() const override
  {
    return this->someInt + this->someDouble + 2;
  }
};
GZ_ADD_FACTORY(SomeObjectAddTwo, SomeObjectFactory)

GZ_ADD_FACTORY_ALIAS(
    SomeObjectAddTwo, SomeObjectFactory,
    "This factory has an alias", "and also a second alias")

}
}
