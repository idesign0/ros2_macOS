/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#include "gz/common/testing/AutoLogFixture.hh"

#include <functional>
#include <gz/common/Event.hh>
#include <gz/common/Util.hh>
using namespace gz;

class EventTest : public common::testing::AutoLogFixture { };

int g_callback = 0;
int g_callback1 = 0;
common::EventT<void ()> g_event;
common::ConnectionPtr g_conn;
common::ConnectionPtr g_conn2;

/////////////////////////////////////////////////
void callback()
{
  g_callback++;
}

/////////////////////////////////////////////////
void callback1()
{
  g_callback1++;
}

/////////////////////////////////////////////////
// Used by the CallbackDisconnect test.
void callbackDisconnect2()
{
  // This function should still be called, even though it was disconnected
  // in the callDisconnect function. The mutex in Event.hh prevents
  // a callback from deleting active connections until the event is
  // complete.
  ASSERT_TRUE(true);
}

/////////////////////////////////////////////////
// Make sure that calling disconnect in an event callback does not produce
// a segfault.
TEST_F(EventTest, CallbackDisconnect)
{
  // Create two connections
  g_conn2 = g_event.Connect(std::bind(&callbackDisconnect2));

  // Call the event. See the callback functions for more info.
  g_event();
}

/////////////////////////////////////////////////
TEST_F(EventTest, SignalOnce)
{
  g_callback = 0;

  common::EventT<void ()> evt;
  common::ConnectionPtr conn = evt.Connect(std::bind(&callback));
  evt();

  EXPECT_EQ(g_callback, 1);
}

/////////////////////////////////////////////////
TEST_F(EventTest, SignalTwice)
{
  g_callback = 0;

  common::EventT<void ()> evt;
  common::ConnectionPtr conn = evt.Connect(std::bind(&callback));
  evt();
  evt();

  EXPECT_EQ(g_callback, 2);
}

/////////////////////////////////////////////////
TEST_F(EventTest, SignalN)
{
  g_callback = 0;

  common::EventT<void ()> evt;
  common::ConnectionPtr conn = evt.Connect(std::bind(&callback));

  for (unsigned int i = 0; i < 100; ++i)
    evt();

  EXPECT_EQ(g_callback, 100);
}

/////////////////////////////////////////////////
TEST_F(EventTest, Disconnect)
{
  g_callback = 0;

  common::EventT<void ()> evt;
  common::ConnectionPtr conn = evt.Connect(std::bind(&callback));

  conn.reset();

  evt();

  EXPECT_EQ(g_callback, 0);
}

/////////////////////////////////////////////////
TEST_F(EventTest, MultiCallback)
{
  g_callback = 0;
  g_callback1 = 0;

  common::EventT<void ()> evt;
  common::ConnectionPtr conn = evt.Connect(std::bind(&callback));
  common::ConnectionPtr conn1 = evt.Connect(std::bind(&callback1));

  evt();

  EXPECT_EQ(g_callback, 1);
  EXPECT_EQ(g_callback1, 1);
}

/////////////////////////////////////////////////
TEST_F(EventTest, MultiCallbackDisconnect)
{
  g_callback = 0;
  g_callback1 = 0;

  common::EventT<void ()> evt;
  common::ConnectionPtr conn = evt.Connect(std::bind(&callback));
  common::ConnectionPtr conn1 = evt.Connect(std::bind(&callback1));
  conn.reset();

  evt();

  EXPECT_EQ(g_callback, 0);
  EXPECT_EQ(g_callback1, 1);
}

/////////////////////////////////////////////////
TEST_F(EventTest, MultiCallbackReconnect)
{
  g_callback = 0;
  g_callback1 = 0;

  common::EventT<void ()> evt;
  common::ConnectionPtr conn = evt.Connect(std::bind(&callback));
  common::ConnectionPtr conn1 = evt.Connect(std::bind(&callback1));
  conn.reset();
  conn = evt.Connect(std::bind(&callback));

  evt();

  EXPECT_EQ(g_callback, 1);
  EXPECT_EQ(g_callback1, 1);
}

/////////////////////////////////////////////////
TEST_F(EventTest, ManyChanges)
{
  g_callback = 0;
  g_callback1 = 0;

  common::EventT<void ()> evt;
  common::ConnectionPtr conn = evt.Connect(std::bind(&callback));
  common::ConnectionPtr conn1 = evt.Connect(std::bind(&callback1));
  conn.reset();
  conn1.reset();

  evt();

  EXPECT_EQ(g_callback, 0);
  EXPECT_EQ(g_callback1, 0);

  conn = evt.Connect(std::bind(&callback));
  conn1 = evt.Connect(std::bind(&callback1));

  evt();

  EXPECT_EQ(g_callback, 1);
  EXPECT_EQ(g_callback1, 1);

  evt();

  EXPECT_EQ(g_callback, 2);
  EXPECT_EQ(g_callback1, 2);

  conn1.reset();

  evt();

  EXPECT_EQ(g_callback, 3);
  EXPECT_EQ(g_callback1, 2);
}

TEST_F(EventTest, EventWithOneParam)
{
  int count = 0;

  auto fcn = [&count](int increment1){
    count += increment1;
  };

  common::EventT<void(int)> evt;
  common::ConnectionPtr conn = evt.Connect(fcn);

  evt(10);
  EXPECT_EQ(count, 10);
}

TEST_F(EventTest, EventWithTwoParams)
{
  int count = 0;
  std::string test;

  auto fcn = [&count, &test](std::string str, int increment){
    test = str;
    count += increment;
  };

  common::EventT<void(std::string, int)> evt;
  common::ConnectionPtr conn = evt.Connect(fcn);

  evt("test", 10);
  EXPECT_EQ(count, 10);
  EXPECT_EQ(test, "test");
}

TEST_F(EventTest, EventWithTenParams)
{
  int count = 0;

  auto fcn = [&count](int i1, int i2, int i3, int i4, int i5,
                      int i6, int i7, int i8, int i9, int i10){
    count += i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
  };

  common::EventT<void(int, int, int, int, int,
                      int, int, int, int, int)> evt;
  common::ConnectionPtr conn = evt.Connect(fcn);

  evt(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  EXPECT_EQ(count, 55);
}

TEST_F(EventTest, typeid_test)
{
  using Event1 = common::EventT<void(int)>;
  using Event2 = common::EventT<void(int)>;

  // Type aliases don't change the `typeid` value.
  EXPECT_EQ(typeid(Event1).hash_code(), typeid(Event2).hash_code());
  EXPECT_EQ(typeid(Event1).name(), typeid(Event2).name());

  // To have multiple events of the same type, use the second arg
  using Event3 = common::EventT<void(int), struct Event3Tag>;
  using Event4 = common::EventT<void(int), struct Event4Tag>;

  EXPECT_NE(typeid(Event3).hash_code(), typeid(Event4).hash_code());
  EXPECT_NE(typeid(Event3).name(), typeid(Event4).name());

  // These should also not be the same as the previous.
  EXPECT_NE(typeid(Event3).hash_code(), typeid(Event1).hash_code());
  EXPECT_NE(typeid(Event3).hash_code(), typeid(Event2).hash_code());
  EXPECT_NE(typeid(Event3).name(), typeid(Event1).name());
  EXPECT_NE(typeid(Event3).name(), typeid(Event2).name());
}

/////////////////////////////////////////////////
TEST_F(EventTest, DestructionOrder)
{
  auto evt = std::make_unique<common::EventT<void()>>();
  common::ConnectionPtr conn = evt->Connect(callback);
  evt->Signal();
  evt.reset();
  // Sleep to avoid warning about deleting a connection right after creation.
  GZ_SLEEP_MS(1);

  // Check that this doesn't segfault.
  conn.reset();
  SUCCEED();
}
