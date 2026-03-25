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
#include "gz/common/KeyEvent.hh"

using namespace gz;
using namespace common;

/////////////////////////////////////////////////
TEST(KeyEventTEST, Constructor)
{
  // copy constructor
  {
    KeyEvent evt;
    EXPECT_EQ(evt.Type(), KeyEvent::NO_EVENT);
    EXPECT_EQ(evt.Key(), 0);
    EXPECT_TRUE(evt.Text().empty());
    EXPECT_FALSE(evt.Control());
    EXPECT_FALSE(evt.Shift());
    EXPECT_FALSE(evt.Alt());

    evt.SetType(KeyEvent::PRESS);
    evt.SetKey(123);
    evt.SetText("hello");
    evt.SetControl(true);
    evt.SetShift(true);
    evt.SetAlt(true);

    KeyEvent otherEvent(evt);
    EXPECT_EQ(otherEvent.Type(), KeyEvent::PRESS);
    EXPECT_EQ(otherEvent.Key(), 123);
    EXPECT_EQ(otherEvent.Text(), "hello");
    EXPECT_TRUE(otherEvent.Control());
    EXPECT_TRUE(otherEvent.Shift());
    EXPECT_TRUE(otherEvent.Alt());
  }

  // assignment
  {
    KeyEvent evt;
    EXPECT_EQ(evt.Type(), KeyEvent::NO_EVENT);
    EXPECT_EQ(evt.Key(), 0);
    EXPECT_TRUE(evt.Text().empty());
    EXPECT_FALSE(evt.Control());
    EXPECT_FALSE(evt.Shift());
    EXPECT_FALSE(evt.Alt());

    evt.SetType(KeyEvent::PRESS);
    evt.SetKey(123);
    evt.SetText("hello");
    evt.SetControl(true);
    evt.SetShift(true);
    evt.SetAlt(true);

    KeyEvent otherEvent;
    otherEvent = evt;
    EXPECT_EQ(otherEvent.Type(), KeyEvent::PRESS);
    EXPECT_EQ(otherEvent.Key(), 123);
    EXPECT_EQ(otherEvent.Text(), "hello");
    EXPECT_TRUE(otherEvent.Control());
    EXPECT_TRUE(otherEvent.Shift());
    EXPECT_TRUE(otherEvent.Alt());
  }

  // move assignment
  {
    KeyEvent evt;
    EXPECT_EQ(evt.Type(), KeyEvent::NO_EVENT);
    EXPECT_EQ(evt.Key(), 0);
    EXPECT_TRUE(evt.Text().empty());
    EXPECT_FALSE(evt.Control());
    EXPECT_FALSE(evt.Shift());
    EXPECT_FALSE(evt.Alt());

    evt.SetType(KeyEvent::PRESS);
    evt.SetKey(123);
    evt.SetText("hello");
    evt.SetControl(true);
    evt.SetShift(true);
    evt.SetAlt(true);

    KeyEvent otherEvent;
    otherEvent = std::move(evt);
    EXPECT_EQ(otherEvent.Type(), KeyEvent::PRESS);
    EXPECT_EQ(otherEvent.Key(), 123);
    EXPECT_EQ(otherEvent.Text(), "hello");
    EXPECT_TRUE(otherEvent.Control());
    EXPECT_TRUE(otherEvent.Shift());
    EXPECT_TRUE(otherEvent.Alt());
  }

  // move
  {
    KeyEvent evt;
    EXPECT_EQ(evt.Type(), KeyEvent::NO_EVENT);
    EXPECT_EQ(evt.Key(), 0);
    EXPECT_TRUE(evt.Text().empty());
    EXPECT_FALSE(evt.Control());
    EXPECT_FALSE(evt.Shift());
    EXPECT_FALSE(evt.Alt());

    evt.SetType(KeyEvent::PRESS);
    evt.SetKey(123);
    evt.SetText("hello");
    evt.SetControl(true);
    evt.SetShift(true);
    evt.SetAlt(true);

    KeyEvent otherEvent(std::move(evt));
    EXPECT_EQ(otherEvent.Type(), KeyEvent::PRESS);
    EXPECT_EQ(otherEvent.Key(), 123);
    EXPECT_EQ(otherEvent.Text(), "hello");
    EXPECT_TRUE(otherEvent.Control());
    EXPECT_TRUE(otherEvent.Shift());
    EXPECT_TRUE(otherEvent.Alt());
  }
}
