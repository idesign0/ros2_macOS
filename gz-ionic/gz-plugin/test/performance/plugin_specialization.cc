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

#include <chrono>
#include <iomanip>
#include <cmath>

#include <gz/plugin/Loader.hh>
#include <gz/plugin/SpecializedPluginPtr.hh>

#include "../plugins/DummyPlugins.hh"


#define GZ_CREATE_INTERFACE(name)\
  class name {  };

GZ_CREATE_INTERFACE(Interface1)
GZ_CREATE_INTERFACE(Interface2)
GZ_CREATE_INTERFACE(Interface3)
GZ_CREATE_INTERFACE(Interface4)
GZ_CREATE_INTERFACE(Interface5)
GZ_CREATE_INTERFACE(Interface6)
GZ_CREATE_INTERFACE(Interface7)
GZ_CREATE_INTERFACE(Interface8)
GZ_CREATE_INTERFACE(Interface9)
GZ_CREATE_INTERFACE(Interface10)
GZ_CREATE_INTERFACE(Interface11)
GZ_CREATE_INTERFACE(Interface12)
GZ_CREATE_INTERFACE(Interface13)
GZ_CREATE_INTERFACE(Interface14)
GZ_CREATE_INTERFACE(Interface15)
GZ_CREATE_INTERFACE(Interface16)
GZ_CREATE_INTERFACE(Interface17)
GZ_CREATE_INTERFACE(Interface18)
GZ_CREATE_INTERFACE(Interface19)

// Specialize for only 1 type
using Specialize1Type =
    gz::plugin::SpecializedPluginPtr<test::util::DummySetterBase>;

// Specialize for 3 different types, and put the type we care about first in
// the list.
using Specialize3Types_Leading =
    gz::plugin::SpecializedPluginPtr<
        test::util::DummySetterBase,
        Interface1, Interface2>;

// Specialize for 3 different types, and put the type we care about last in
// the list.
using Specialize3Types_Trailing =
    gz::plugin::SpecializedPluginPtr<
        Interface1, Interface2,
        test::util::DummySetterBase>;

// Specialize for 10 different types, and put the type we care about first in
// the list.
using Specialize10Types_Leading =
    gz::plugin::SpecializedPluginPtr<
        test::util::DummySetterBase,
        Interface1, Interface2, Interface3, Interface4, Interface5,
        Interface6, Interface7, Interface8, Interface9>;

// Specialize for 10 different types, and put the type we care about last in
// the list.
using Specialize10Types_Trailing =
    gz::plugin::SpecializedPluginPtr<
        Interface1, Interface2, Interface3, Interface4, Interface5,
        Interface6, Interface7, Interface8, Interface9,
        test::util::DummySetterBase>;

// Specialize for 20 different types, and put the type we care about first in
// the list.
using Specialize20Types_Leading =
    gz::plugin::SpecializedPluginPtr<
        test::util::DummySetterBase,
        Interface1, Interface2, Interface3, Interface4, Interface5,
        Interface6, Interface7, Interface8, Interface9, Interface10,
        Interface11, Interface12, Interface13, Interface14, Interface15,
        Interface16, Interface17, Interface18, Interface19>;

// Specialize for 20 different types, and put the type we care about last in
// the list.
using Specialize20Types_Trailing =
    gz::plugin::SpecializedPluginPtr<
        Interface1, Interface2, Interface3, Interface4, Interface5,
        Interface6, Interface7, Interface8, Interface9, Interface10,
        Interface11, Interface12, Interface13, Interface14, Interface15,
        Interface16, Interface17, Interface18, Interface19,
        test::util::DummySetterBase>;


template <typename PluginType>
double RunPerformanceTest(const PluginType &plugin)
{
  const std::size_t NumTests = 10000;
  const auto start = std::chrono::high_resolution_clock::now();
  for (std::size_t i = 0; i < NumTests; ++i)
  {
    plugin->template QueryInterface<test::util::DummySetterBase>();
  }
  const auto finish = std::chrono::high_resolution_clock::now();

  const auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(
        finish - start).count();

  const double avg = static_cast<double>(time)/static_cast<double>(NumTests);

  return avg;
}

/////////////////////////////////////////////////
TEST(PluginSpecialization, AccessTime)
{
  gz::plugin::Loader pl;
  pl.LoadLib(GzDummyPlugin_LIB);

  // Load up the generic plugin
  gz::plugin::PluginPtr plugin =
      pl.Instantiate("::test::util::DummyMultiPlugin");

  // Create specialized versions
  Specialize1Type spec_1 = plugin;
  Specialize3Types_Leading spec_3_leading = plugin;
  Specialize3Types_Trailing spec_3_trailing = plugin;
  Specialize10Types_Leading spec_10_leading = plugin;
  Specialize10Types_Trailing spec_10_trailing = plugin;
  Specialize20Types_Leading spec_20_leading = plugin;
  Specialize20Types_Trailing spec_20_trailing = plugin;

  struct TestData
  {
    std::string label;
    double avg;

    explicit TestData(const std::string &_label)
      : label(_label), avg(0.0) { }
  };

  std::vector<TestData> tests;
  tests.push_back(TestData("1 specialization"));
  tests.push_back(TestData("3 specializations (leading)"));
  tests.push_back(TestData("3 specializations (trailing)"));
  tests.push_back(TestData("10 specializations (leading)"));
  tests.push_back(TestData("10 specializations (trailing)"));
  tests.push_back(TestData("20 specializations (leading)"));
  tests.push_back(TestData("20 specializations (trailing)"));
  tests.push_back(TestData("No specialization"));

  const std::size_t NumTrials = 1000;
  const std::size_t Warmup = 50;

  for (std::size_t i = 0; i < Warmup + NumTrials; ++i)
  {
    std::size_t t = 0;
    tests[t++].avg += RunPerformanceTest(spec_1);
    tests[t++].avg += RunPerformanceTest(spec_3_leading);
    tests[t++].avg += RunPerformanceTest(spec_3_trailing);
    tests[t++].avg += RunPerformanceTest(spec_10_leading);
    tests[t++].avg += RunPerformanceTest(spec_10_trailing);
    tests[t++].avg += RunPerformanceTest(spec_20_leading);
    tests[t++].avg += RunPerformanceTest(spec_20_trailing);
    tests[t++].avg += RunPerformanceTest(plugin);

    // Note that whichever test is listed first in the for-loop will have the
    // worst performance, probably due to weird register behavior. We run
    // through the cycle repeatedly to wash out that strange effect, but the
    // effect is still noticeable, so we run a warmup phase that does not get
    // recorded. This seems to be helping to make the results fair, but
    // inconsistencies are still possible.
    if (i < Warmup)
    {
      for (TestData &test : tests)
        test.avg = 0.0;
    }
  }

  // Test that none of the specialized results are double the baseline (or
  // vice-versa)
  const double baseline = tests[1].avg;
  EXPECT_LT(std::abs(tests[2].avg - baseline), baseline);
  EXPECT_LT(std::abs(tests[3].avg - baseline), baseline);
  EXPECT_LT(std::abs(tests[4].avg - baseline), baseline);
  EXPECT_LT(std::abs(tests[5].avg - baseline), baseline);
  EXPECT_LT(std::abs(tests[6].avg - baseline), baseline);

  // Test that the specialized results are always better than the generic result
  EXPECT_LT(tests[0].avg, tests.back().avg);
  EXPECT_LT(tests[1].avg, tests.back().avg);
  EXPECT_LT(tests[2].avg, tests.back().avg);
  EXPECT_LT(tests[3].avg, tests.back().avg);
  EXPECT_LT(tests[4].avg, tests.back().avg);
  EXPECT_LT(tests[5].avg, tests.back().avg);
  EXPECT_LT(tests[6].avg, tests.back().avg);

  for (const TestData &test : tests)
  {
    std::cout << std::fixed;
    std::cout << std::setprecision(6);
    std::cout << std::right;

    std::cout << " --- " << test.label << " result ---\n"
              << "Avg time: " << std::setw(11) << std::right
              << test.avg/static_cast<double>(NumTrials)
              << "ns\n" << std::endl;
  }
}
