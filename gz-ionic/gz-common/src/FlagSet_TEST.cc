/*
 * MIT License
 *
 * Copyright (c) 2019 Arnaud Kapp (Xaqq), Barry Revzin, Mart Sõmermaa
 * Copyright (c) 2020 Martin Pecka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

// gz::common::FlagSet is a type-safe class for using enums as flags in
// C++14 with an underlying std::bitset.
// See https://github.com/mrts/flag-set-cpp
// Licence: MIT

#include <gz/common/FlagSet.hh>

#include <cstdint>
#include <unordered_set>

#include "gtest/gtest.h"

using namespace gz;

enum class Options : uint8_t
{
  FULLSCREEN,
  INVERT_MOUSE,
  FLASH,
  RED_BACKGROUND,
  RED_FOREGROUND,
  _
};

enum class OptionsNoUnderscore : uint8_t
{
  FULLSCREEN,
  INVERT_MOUSE,
  FLASH,
  RED_BACKGROUND,
  RED_FOREGROUND,
  END
};

enum class OptionsNoEnd : uint8_t
{
  FULLSCREEN,
  INVERT_MOUSE,
  FLASH,
  RED_BACKGROUND,
  RED_FOREGROUND
};

TEST(FlagSet, TestAnd)
{
  common::FlagSet<Options> red(
      Options::RED_FOREGROUND | Options::RED_BACKGROUND);

  auto result = red & Options::RED_BACKGROUND;
  EXPECT_TRUE(result);
  EXPECT_EQ(result.Count(), 1u);

  result = red & Options::RED_FOREGROUND;
  EXPECT_TRUE(result);
  EXPECT_EQ(result.Count(), 1u);

  result = red & (Options::RED_FOREGROUND | Options::RED_BACKGROUND);
  EXPECT_TRUE(result);
  EXPECT_EQ(result.Count(), 2u);

  result = ~red & Options::RED_BACKGROUND;
  EXPECT_FALSE(result);
  EXPECT_EQ(result.Count(), 0u);

  common::FlagSet<Options> red_foreground(Options::RED_FOREGROUND);
  red &= Options::RED_FOREGROUND;
  EXPECT_TRUE(red == red_foreground);
}

TEST(FlagSet, TestOr)
{
  common::FlagSet<Options> red;
  red |= Options::RED_FOREGROUND | Options::RED_BACKGROUND;
  EXPECT_TRUE(red);
  EXPECT_EQ(red.Count(), 2u);

  common::FlagSet<Options> options;
  options |= (Options::FULLSCREEN | Options::FLASH);

  auto result = options & (Options::FULLSCREEN | Options::FLASH |
                           Options::RED_FOREGROUND);
  EXPECT_TRUE(result);

  common::FlagSet<Options> expected;
  expected |= Options::FULLSCREEN;
  EXPECT_EQ(options & expected, expected);

  result = options & (Options::RED_FOREGROUND | Options::RED_BACKGROUND);
  EXPECT_FALSE(result);

  EXPECT_FALSE(options & red);

  EXPECT_TRUE(!(options & Options::INVERT_MOUSE));

  options |= ~red;

  EXPECT_TRUE(options & Options::INVERT_MOUSE);
}

TEST(FlagSet, TestSetReset)
{
  common::FlagSet<Options> options;
  EXPECT_EQ(options.Count(), 0u);

  options.Set();
  EXPECT_EQ(options.Count(), 5u);
  EXPECT_EQ(options.Size(), 5u);
  EXPECT_EQ(options.String(), "11111");

  options.Reset();
  EXPECT_EQ(options.Count(), 0u);

  options.Set(Options::FLASH);
  EXPECT_EQ(options.Count(), 1u);
  EXPECT_TRUE(options[Options::FLASH]);

  options.Set(Options::FLASH, false);
  EXPECT_EQ(options.Count(), 0u);

  options.Set(Options::FLASH);
  options.Set(Options::INVERT_MOUSE);
  EXPECT_EQ(options.Count(), 2u);
}

TEST(FlagSet, TestAnyAllNone)
{
  common::FlagSet<Options> options;
  EXPECT_TRUE(options.None());
  EXPECT_FALSE(options.Any());
  EXPECT_FALSE(options.All());

  options.Set();
  EXPECT_FALSE(options.None());
  EXPECT_TRUE(options.Any());
  EXPECT_TRUE(options.All());

  options.Reset();
  EXPECT_TRUE(options.None());
  EXPECT_FALSE(options.Any());
  EXPECT_FALSE(options.All());

  options.Set(Options::FLASH);
  EXPECT_FALSE(options.None());
  EXPECT_TRUE(options.Any());
  EXPECT_FALSE(options.All());

  options.Set(Options::FLASH, false);
  EXPECT_TRUE(options.None());
  EXPECT_FALSE(options.Any());
  EXPECT_FALSE(options.All());

  options.Set(Options::FLASH);
  options.Set(Options::INVERT_MOUSE);
  EXPECT_FALSE(options.None());
  EXPECT_TRUE(options.Any());
  EXPECT_FALSE(options.All());
}

TEST(FlagSet, TestConstructors)
{
  using namespace gz::common;

  ASSERT_EQ(5u, FlagSet<Options>::numElements);

  EXPECT_EQ(FlagSet<Options>::NoneSet(), FlagSet<Options>());
  EXPECT_EQ(FlagSet<Options>::AllSet(), FlagSet<Options>({
    Options::FULLSCREEN,
    Options::FLASH,
    Options::INVERT_MOUSE,
    Options::RED_BACKGROUND,
    Options::RED_FOREGROUND
  }));

  FlagSet<Options> options(Options::RED_FOREGROUND);
  EXPECT_TRUE(options[Options::RED_FOREGROUND]);
  EXPECT_FALSE(options[Options::RED_BACKGROUND]);
  EXPECT_FALSE(options[Options::INVERT_MOUSE]);
  EXPECT_FALSE(options[Options::FLASH]);
  EXPECT_FALSE(options[Options::FULLSCREEN]);
}

TEST(FlagSet, TestStaticConstructors)
{
  EXPECT_TRUE(common::FlagSet<Options>::AllSet().All());
  EXPECT_FALSE(common::FlagSet<Options>::AllSet().None());
  EXPECT_TRUE(common::FlagSet<Options>::AllSet().Any());

  EXPECT_FALSE(common::FlagSet<Options>::NoneSet().All());
  EXPECT_TRUE(common::FlagSet<Options>::NoneSet().None());
  EXPECT_FALSE(common::FlagSet<Options>::NoneSet().Any());
}


TEST(FlagSet, TestHash)
{
  using namespace gz::common;

  const auto options1 = FlagSet<Options>::AllSet();
  const auto options2 = FlagSet<Options>::NoneSet();

  std::hash<FlagSet<Options>> hash {};

  EXPECT_NE(hash(options1), hash(options2));
  EXPECT_EQ(hash(options1), hash(options1));
  EXPECT_EQ(hash(options2), hash(options2));

  std::unordered_set<FlagSet<Options>> valid;
  EXPECT_EQ(valid.find(FlagSet<Options>::AllSet()), valid.end());
  valid.insert(FlagSet<Options>::AllSet());
  EXPECT_NE(valid.find(FlagSet<Options>::AllSet()), valid.end());
}

TEST(FlagSet, TestEnumWithoutUnderscore)
{
  using ONU = OptionsNoUnderscore;

  using TestSet = common::FlagSet<ONU, ONU::END>;

  ASSERT_EQ(5u, TestSet::numElements);

  EXPECT_EQ(TestSet::NoneSet(), TestSet());
  EXPECT_EQ(TestSet::AllSet(), TestSet({
    ONU::FULLSCREEN,
    ONU::FLASH,
    ONU::INVERT_MOUSE,
    ONU::RED_BACKGROUND,
    ONU::RED_FOREGROUND
  }));

  TestSet options(ONU::RED_FOREGROUND);
  EXPECT_TRUE(options[ONU::RED_FOREGROUND]);
  EXPECT_FALSE(options[ONU::RED_BACKGROUND]);
  EXPECT_FALSE(options[ONU::INVERT_MOUSE]);
  EXPECT_FALSE(options[ONU::FLASH]);
  EXPECT_FALSE(options[ONU::FULLSCREEN]);

  EXPECT_EQ(
    TestSet({ONU::FULLSCREEN, ONU::FLASH}),
    TestSet(ONU::FLASH) | TestSet(ONU::FULLSCREEN));

  const auto options1 = TestSet::AllSet();
  const auto options2 = TestSet::NoneSet();

  std::hash<TestSet> hash {};
  EXPECT_NE(hash(options1), hash(options2));
}

TEST(FlagSet, TestEnumWithoutEnd)
{
  using ONE = OptionsNoEnd;

  using TestSet = common::FlagSet<ONE, ONE::RED_FOREGROUND, false>;

  ASSERT_EQ(5u, TestSet::numElements);

  EXPECT_EQ(TestSet::NoneSet(), TestSet());
  EXPECT_EQ(TestSet::AllSet(), TestSet({
      ONE::FULLSCREEN,
      ONE::FLASH,
      ONE::INVERT_MOUSE,
      ONE::RED_BACKGROUND,
      ONE::RED_FOREGROUND
    }));

  TestSet options(ONE::RED_FOREGROUND);
  EXPECT_TRUE(options[ONE::RED_FOREGROUND]);
  EXPECT_FALSE(options[ONE::RED_BACKGROUND]);
  EXPECT_FALSE(options[ONE::INVERT_MOUSE]);
  EXPECT_FALSE(options[ONE::FLASH]);
  EXPECT_FALSE(options[ONE::FULLSCREEN]);

  EXPECT_EQ(
    TestSet({ONE::FULLSCREEN, ONE::FLASH}),
    TestSet(ONE::FLASH) | TestSet(ONE::FULLSCREEN));

  const auto options1 = TestSet::AllSet();
  const auto options2 = TestSet::NoneSet();

  std::hash<TestSet> hash {};
  EXPECT_NE(hash(options1), hash(options2));
}

TEST(FlagSet, TestUnderlyingTypes)
{
  enum class CharEnum : char
  {
    A,
    _
  };
  enum class UCharEnum : unsigned char
  {
    A,
    _
  };
  enum class ShortEnum : int16_t
  {
    A,
    _
  };
  enum class UShortEnum : uint16_t
  {
    A,
    _
  };
  enum class IntEnum : int
  {
    A,
    _
  };
  enum class UIntEnum : unsigned int
  {
    A,
    _
  };
  enum class LongEnum : int32_t
  {
    A,
    _
  };
  enum class ULongEnum : uint32_t
  {
    A,
    _
  };
  enum class LongLongEnum : int64_t
  {
    A,
    _
  };
  enum class ULongLongEnum : uint64_t
  {
    A,
    _
  };
  enum OldEnum
  {
    A,
    _
  };
  enum class TypedEnum
  {
    A,
    _
  };

  using namespace gz::common;

  EXPECT_TRUE(FlagSet<CharEnum>(CharEnum::A).All());
  EXPECT_TRUE(FlagSet<UCharEnum>(UCharEnum::A).All());
  EXPECT_TRUE(FlagSet<ShortEnum>(ShortEnum::A).All());
  EXPECT_TRUE(FlagSet<UShortEnum>(UShortEnum::A).All());
  EXPECT_TRUE(FlagSet<IntEnum>(IntEnum::A).All());
  EXPECT_TRUE(FlagSet<UIntEnum>(UIntEnum::A).All());
  EXPECT_TRUE(FlagSet<LongEnum>(LongEnum::A).All());
  EXPECT_TRUE(FlagSet<ULongEnum>(ULongEnum::A).All());
  EXPECT_TRUE(FlagSet<LongLongEnum>(LongLongEnum::A).All());
  EXPECT_TRUE(FlagSet<ULongLongEnum>(ULongLongEnum::A).All());
  EXPECT_TRUE(FlagSet<OldEnum>(OldEnum::A).All());
  EXPECT_TRUE(FlagSet<TypedEnum>(TypedEnum::A).All());
}
