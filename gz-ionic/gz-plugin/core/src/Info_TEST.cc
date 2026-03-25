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

#include <gtest/gtest.h>

#include <gz/plugin/Info.hh>

struct SomeInterface
{
};

struct SomePlugin : public SomeInterface
{
};

TEST(Info, Clear)
{
  gz::plugin::Info info;
  info.name = typeid(SomePlugin).name();

  info.factory = [=]()
  {
    return static_cast<void*>(new SomePlugin);
  };

  info.deleter = [=](void *ptr)
  {
    delete static_cast<SomePlugin*>(ptr);
  };

  info.interfaces.insert(
      std::make_pair(
        typeid(SomeInterface).name(),
        [=](void *v_ptr)
  {
    SomePlugin *d_ptr = static_cast<SomePlugin*>(v_ptr);
    return static_cast<SomeInterface*>(d_ptr);
  }));

  info.aliases.insert("some alias");
  info.aliases.insert("another alias");

  for (const auto &interfaceName : info.interfaces)
  {
    info.demangledInterfaces.insert(interfaceName.first);
  }

  EXPECT_FALSE(info.name.empty());
  EXPECT_FALSE(info.aliases.empty());
  EXPECT_FALSE(info.interfaces.empty());
  EXPECT_FALSE(info.demangledInterfaces.empty());
  EXPECT_TRUE(static_cast<bool>(info.factory));
  EXPECT_TRUE(static_cast<bool>(info.deleter));

  info.Clear();

  EXPECT_TRUE(info.name.empty());
  EXPECT_TRUE(info.aliases.empty());
  EXPECT_TRUE(info.interfaces.empty());
  EXPECT_TRUE(info.demangledInterfaces.empty());
  EXPECT_FALSE(static_cast<bool>(info.factory));
  EXPECT_FALSE(static_cast<bool>(info.deleter));
}
