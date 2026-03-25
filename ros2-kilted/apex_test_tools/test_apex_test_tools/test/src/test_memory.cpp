// Copyright 2020 Apex.AI, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "osrf_testing_tools_cpp/memory_tools/memory_tools.hpp"
#include "apex_test_tools/apex_test_tools.hpp"
#include <gtest/gtest.h>

TEST(apex_test_tools, test_memory) {
  apex_test_tools::memory_test::start();
  apex_test_tools::memory_test::pause();
  ASSERT_TRUE(osrf_testing_tools_cpp::memory_tools::is_working());
  apex_test_tools::memory_test::stop();
  ASSERT_FALSE(osrf_testing_tools_cpp::memory_tools::is_working());
}
