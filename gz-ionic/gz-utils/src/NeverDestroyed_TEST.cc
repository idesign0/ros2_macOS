/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include "gz/utils/NeverDestroyed.hh"

#include <gtest/gtest.h>

#include <cstdint>
#include <exception>
#include <initializer_list>
#include <random>
#include <string>
#include <vector>
#include <unordered_map>

using namespace gz;

class Boom : public std::exception
{
};
struct DtorGoesBoom
{
#ifdef _WIN32
// Disable warning C4722 which is triggered by the exception thrown in the dtor
#pragma warning(push)
#pragma warning(disable : 4722)
#endif
  ~DtorGoesBoom() noexcept(false)
  {
    throw Boom();
  }
#ifdef _WIN32
#pragma warning(pop)
#endif
};

// Confirm that we see booms by default.
GTEST_TEST(NeverDestroyed, BoomTest)
{
  try
  {
    {
      DtorGoesBoom foo;
    }
    GTEST_FAIL();
  }
  catch (const Boom &)
  {
    ASSERT_TRUE(true);
  }
}

// Confirm that our wrapper stops the booms.
GTEST_TEST(NeverDestroyed, NoBoomTest)
{
  try
  {
    {
      utils::NeverDestroyed<DtorGoesBoom> foo;
    }
    ASSERT_TRUE(true);
  }
  catch (const Boom &e)
  {
    GTEST_FAIL();
  }
}

// This is an example from the class overview API docs; we repeat it here to
// ensure it remains valid.
class Singleton
{
  public: Singleton(const Singleton &) = delete;
  public: void operator=(const Singleton &) = delete;
  public: Singleton(Singleton &&) = delete;
  public: void operator=(Singleton &&) = delete;
  public: static Singleton &getInstance()
  {
    static utils::NeverDestroyed<Singleton> instance;
    return instance.Access();
  }

  private: friend utils::NeverDestroyed<Singleton>;
  private: Singleton() = default;
};

GTEST_TEST(NeverDestroyedExample, Singleton)
{
  const Singleton *get1 = &Singleton::getInstance();
  const Singleton *get2 = &Singleton::getInstance();
  EXPECT_EQ(get1, get2);
}

// This is an example from the class overview API docs; we repeat it here to
// ensure it remains valid.
enum class Foo
{
  kBar,
  kBaz
};
Foo ParseFoo(const std::string &foo_string)
{
  using Dict = std::unordered_map<std::string, Foo>;
  static const utils::NeverDestroyed<Dict> string_to_enum{
      std::initializer_list<Dict::value_type>{
          {"bar", Foo::kBar},
          {"baz", Foo::kBaz},
      }};
  return string_to_enum.Access().at(foo_string);
}

GTEST_TEST(NeverDestroyedExample, ParseFoo)
{
  EXPECT_EQ(ParseFoo("bar"), Foo::kBar);
  EXPECT_EQ(ParseFoo("baz"), Foo::kBaz);
}

// This is an example from the class overview API docs; we repeat it here to
// ensure it remains valid.
using Result = std::vector<std::uint_fast32_t>;
const Result &GetConstantMagicNumbers()
{
  static const utils::NeverDestroyed<Result> result{
      []()
      {
        Result prototype;
        std::mt19937 random_generator;
        for (int i = 0; i < 10; ++i)
        {
          auto new_value = random_generator();
          prototype.push_back(new_value);
        }
        return prototype;
      }()};
  return result.Access();
}

GTEST_TEST(NeverDestroyedExample, GetConstantMagicNumbers)
{
  const auto &numbers = GetConstantMagicNumbers();
  EXPECT_EQ(numbers.size(), 10u);
}
