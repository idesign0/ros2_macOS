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

#include <atomic>
#include <thread>

#include "gz/common/Console.hh"
#include "gz/common/WorkerPool.hh"
#include "gz/utils/ExtraTestMacros.hh"

using namespace gz;

//////////////////////////////////////////////////
TEST(WorkerPool, OneWorkNoCallback)
{
  common::WorkerPool pool;
  int workSentinel = 0;

  pool.AddWork([&workSentinel] ()
      {
        workSentinel = 5;
      });
  EXPECT_TRUE(pool.WaitForResults());
  EXPECT_EQ(5, workSentinel);
}

//////////////////////////////////////////////////
TEST(WorkerPool, OneWorkWithCallback)
{
  common::WorkerPool pool;
  int workSentinel = 0;
  int cbSentinel = 0;

  pool.AddWork([&workSentinel] ()
      {
        workSentinel = 5;
      },
    [&cbSentinel] ()
      {
        cbSentinel = 10;
      });
  EXPECT_TRUE(pool.WaitForResults());
  EXPECT_EQ(5, workSentinel);
  EXPECT_EQ(10, cbSentinel);
}

//////////////////////////////////////////////////
TEST(WorkerPool, LotsOfWork)
{
  common::WorkerPool pool;
  std::atomic<int> workSentinel(0);
  std::atomic<int> cbSentinel(0);

  for (int i = 0; i < 1000; i++)
  {
    pool.AddWork([&workSentinel] ()
        {
          workSentinel += 1;
        },
      [&cbSentinel] ()
        {
          cbSentinel += 2;
        });
  }
  EXPECT_TRUE(pool.WaitForResults());
  EXPECT_EQ(1000, workSentinel);
  EXPECT_EQ(2000, cbSentinel);
}

//////////////////////////////////////////////////
TEST(WorkerPool, WaitWithTimeout)
{
  common::WorkerPool pool;
  int workSentinel = 0;
  pool.AddWork([&workSentinel] ()
      {
        workSentinel = 5;
      });
  EXPECT_TRUE(pool.WaitForResults(std::chrono::seconds(5)));
  EXPECT_EQ(5, workSentinel);
}

//////////////////////////////////////////////////
// /TODO(anyone) Deflake this test
// ref: https://github.com/gazebosim/gz-common/issues/52
TEST(WorkerPool,
     GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(WaitWithTimeoutThatTimesOut))
{
  common::WorkerPool pool;
  pool.AddWork([] ()
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      });
  EXPECT_FALSE(pool.WaitForResults(std::chrono::nanoseconds(100000)));
}

//////////////////////////////////////////////////
// /TODO(anyone) Deflake this test
// ref: https://github.com/gazebosim/gz-common/issues/53
TEST(WorkerPool,
     GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(ThingsRunInParallel))
{
  const unsigned int hc = std::thread::hardware_concurrency();
  if (2 > hc)
  {
    gzdbg << "Skipping the ThingsRunInParallel test because hardware "
           << "concurrency (" << hc << ") is too low (min: 2), making the test "
           << "less likely to succeed.\n";
    return;
  }

  common::WorkerPool pool;
  std::atomic<int> sentinel(0);
  pool.AddWork([&sentinel] ()
      {
        ++sentinel;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
      });
  pool.AddWork([&sentinel] ()
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        ++sentinel;
      });
  bool result = pool.WaitForResults(std::chrono::milliseconds(9));
  // the timing test is flaky on windows and mac
  EXPECT_TRUE(result);
  if (!result)
  {
    gzdbg << "WaitForResults failed" << std::endl;
  }
  EXPECT_EQ(2, sentinel);
}
