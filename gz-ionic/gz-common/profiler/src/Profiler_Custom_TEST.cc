/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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

#include "gz/common/Profiler.hh" // NOLINT(*)
#include <gtest/gtest.h> // NOLINT(*)

#include <atomic> // NOLINT(*)
#include <thread> // NOLINT(*)
#include "gz/common/Console.hh"
#include "gz/common/ProfilerImpl.hh"
#include "gz/common/Util.hh" // NOLINT(*)

using namespace gz;
using namespace common;

class CustomProfilerImpl final: public ProfilerImpl
{
  public: std::string Name() const final
  {
    return "test_profiler";
  }

  public: void SetThreadName(const char *_name) final {}

  public: void LogText(const char *_text) final {}

  public: void BeginSample(const char *_name, uint32_t *_hash) final
  {
    beginSampleCallCount++;
  }

  public: void EndSample() final
  {
    endSampleCallCount++;
  }

  /// \brief Number of times `BeginSample` was called.
  public: uint64_t beginSampleCallCount = 0;

  /// \brief Number of times `EndSample` was called.
  public: uint64_t endSampleCallCount = 0;
};

/////////////////////////////////////////////////
TEST(Profiler, CustomProfilerImpl)
{
#ifdef BAZEL_SKIP_PROFILER_TEST
  gzerr << "Test case is disabled for current bazel build config." << std::endl;
#else
  EXPECT_TRUE(GZ_PROFILER_ENABLE);
  auto profiler = std::make_unique<CustomProfilerImpl>();
  auto* profilerRawPtr = profiler.get();
  Profiler::Instance()->SetImplementation(std::move(profiler));
  EXPECT_TRUE(GZ_PROFILER_VALID);
  EXPECT_EQ(Profiler::Instance()->ImplementationName(),
            "test_profiler");
  {
    GZ_PROFILE("Test");
    EXPECT_EQ(1, profilerRawPtr->beginSampleCallCount);
  }
  EXPECT_EQ(1, profilerRawPtr->endSampleCallCount);
#endif
}
