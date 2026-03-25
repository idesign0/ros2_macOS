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

#include <string>

#include "TemplatedPlugins.hh"

#include <gz/plugin/Register.hh>

namespace test
{
namespace plugins
{

/////////////////////////////////////////////////
template <typename T>
class GenericTemplatePlugin
    : public virtual TemplatedGetInterface<T>,
      public virtual TemplatedSetInterface<T>
{
  public: T Get() const override
  {
    return this->value;
  }

  public: void Set(const T &_value) override
  {
    this->value = _value;
  }

  private: T value;
};

/////////////////////////////////////////////////
GZ_ADD_PLUGIN(
    GenericTemplatePlugin<int>,
    TemplatedGetInterface<int>,
    TemplatedSetInterface<int>)

GZ_ADD_PLUGIN(
    GenericTemplatePlugin<std::string>,
    TemplatedGetInterface<std::string>,
    TemplatedSetInterface<std::string>)

/////////////////////////////////////////////////
class DoubleTemplatePlugin
    : public virtual TemplatedGetInterface<int>,
      public virtual TemplatedSetInterface<int>
{
  public: int Get() const override
  {
    return this->value;
  }

  public: void Set(const int &_value) override
  {
    this->value = _value;
  }

  private: int value = 0;
};

/////////////////////////////////////////////////
GZ_ADD_PLUGIN(
    DoubleTemplatePlugin,
    TemplatedGetInterface<int>,
    TemplatedSetInterface<int>)


/////////////////////////////////////////////////
class StringTemplatePlugin
  : public virtual TemplatedGetInterface<std::string>,
    public virtual TemplatedSetInterface<std::string>
{
  public: std::string Get() const override
  {
    return value;
  }

  public: void Set(const std::string &_value) override
  {
    this->value = _value;
  }

  private: std::string value;
};

/////////////////////////////////////////////////
GZ_ADD_PLUGIN(
    StringTemplatePlugin,
    TemplatedGetInterface<std::string>,
    TemplatedSetInterface<std::string>)

}
}
