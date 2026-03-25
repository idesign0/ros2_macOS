/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <gz/plugin/utility.hh>

using namespace gz::plugin;

struct SomeType { };

/////////////////////////////////////////////////
TEST(TemplateHelpers, ConstCompatible)
{
  // Test that ConstCompatible returns the expected result. Expectations:
  //     const |     const --> true
  // non-const |     const --> false
  //     const | non-const --> true
  // non-const | non-const --> true
  //
  // The left column represents the type we want to cast to and the right
  // column is the type we want to cast from. If the left type is less
  // const-qualified than the right type, we return false; otherwise we return
  // true.

  // The extra pair of parentheses are so the comma between the template
  // arguments don't confuse the macro.
  EXPECT_TRUE((ConstCompatible<const int, const double>::value));
  EXPECT_FALSE((ConstCompatible<double, const int>::value));
  EXPECT_TRUE((ConstCompatible<const float, std::string>::value));
  EXPECT_TRUE((ConstCompatible<SomeType, int>::value));

  // Try to trick the template by hiding the const-qualification behind an alias
  using SomeAliasedType = const SomeType;
  EXPECT_TRUE((ConstCompatible<SomeAliasedType, SomeType>::value));
  EXPECT_FALSE((ConstCompatible<SomeType, SomeAliasedType>::value));
}

class SomeSymbol { };

/////////////////////////////////////////////////
TEST(Demangle, RealSymbol)
{
  EXPECT_EQ("SomeSymbol", DemangleSymbol(typeid(SomeSymbol).name()));
}

template <typename T>
class SomeTemplate { };

/////////////////////////////////////////////////
TEST(Demangle, TemplatedSymbol)
{
  EXPECT_EQ("SomeTemplate<SomeSymbol>",
            DemangleSymbol(typeid(SomeTemplate<SomeSymbol>).name()));
}

/////////////////////////////////////////////////
TEST(Demangle, FakeSymbol)
{
#ifdef NDEBUG
  // When a fake symbol is passed in, we expect it to be returned exactly as it
  // was provided.
  EXPECT_EQ("NotReallyASymbol!@#$", DemangleSymbol("NotReallyASymbol!@#$"));
#else
  // If NDEBUG is not defined, we expect an assertion to be triggered, so we
  // will skip this test.
#endif
}
