/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include <thread>

#include <gz/common/Console.hh>
#include <gz/common/Timer.hh>

/////////////////////////////////////////////////
TEST(Timer_TEST, Sequence)
{
  gz::common::Timer t;
  EXPECT_FALSE(t.Running());
  EXPECT_DOUBLE_EQ(0.0, t.ElapsedTime().count());

  // Call stop on a timer that hasn't been started
  t.Stop();
  EXPECT_FALSE(t.Running());

  // Start the timer
  t.Start();
  EXPECT_TRUE(t.Running());
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  EXPECT_LT(0.0, t.ElapsedTime().count());

  t.Stop();
  EXPECT_FALSE(t.Running());
  EXPECT_LT(0.0, t.ElapsedTime().count());
}

/////////////////////////////////////////////////
TEST(Timer_TEST, Elapsed)
{
  gz::common::Timer t;
  t.Start();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  t.Stop();

  // Loose margin just to make sure it is close without being flaky
  EXPECT_LT(0.0, t.ElapsedTime().count());
  gzdbg << "Actual Elapsed: " << t.ElapsedTime().count() << std::endl;
}

/////////////////////////////////////////////////
TEST(Timer_TEST, Copy)
{
  gz::common::Timer t1;
  t1.Start();
  EXPECT_TRUE(t1.Running());

  gz::common::Timer t2 = t1;
  EXPECT_TRUE(t2.Running());

  // Stop the original
  t1.Stop();
  EXPECT_FALSE(t1.Running());
  EXPECT_TRUE(t2.Running());

  // Stop the copy
  t2.Stop();
  EXPECT_FALSE(t1.Running());
  EXPECT_FALSE(t2.Running());
}
