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

#include <memory>

#include "gz/common/Battery.hh"
#include "gz/common/testing/AutoLogFixture.hh"

using namespace gz;

class BatteryTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(BatteryTest, Construction)
{
  // Create the battery
  std::unique_ptr<common::Battery> battery(new common::Battery());
  EXPECT_TRUE(battery != nullptr);

  EXPECT_DOUBLE_EQ(battery->Voltage(), 0.0);
  EXPECT_EQ(battery->PowerLoads().size(), 0u);
}

/////////////////////////////////////////////////
TEST_F(BatteryTest, CopyConstructor)
{
  std::unique_ptr<common::Battery> battery(new common::Battery("battery1",
    12.0));

  std::unique_ptr<common::Battery> batteryCopied(new common::Battery(*battery));
  EXPECT_EQ(batteryCopied->Name(), "battery1");
  EXPECT_DOUBLE_EQ(batteryCopied->InitVoltage(), 12.0);
}

/////////////////////////////////////////////////
TEST_F(BatteryTest, EqualToOperator)
{
  std::unique_ptr<common::Battery> battery(new common::Battery("battery1",
    12.0));
  std::unique_ptr<common::Battery> batteryCopied(new common::Battery(*battery));
  EXPECT_TRUE((*battery) == (*batteryCopied));
  EXPECT_FALSE((*battery) != (*batteryCopied));

  // Difference in name
  batteryCopied->SetName("battery2");
  EXPECT_FALSE((*battery) == (*batteryCopied));
  EXPECT_TRUE((*battery) != (*batteryCopied));

  // Difference in initial voltage
  batteryCopied->SetName(battery->Name());
  batteryCopied->SetInitVoltage(battery->InitVoltage() + 1.0);
  EXPECT_FALSE((*battery) == (*batteryCopied));
  EXPECT_TRUE((*battery) != (*batteryCopied));
}

/////////////////////////////////////////////////
TEST_F(BatteryTest, AssignmentOperator)
{
  std::unique_ptr<common::Battery> battery(new common::Battery("battery1",
    12.0));

  common::Battery batteryAssigned;
  batteryAssigned = *battery;
  EXPECT_EQ(batteryAssigned.Name(), "battery1");
  EXPECT_DOUBLE_EQ(batteryAssigned.InitVoltage(), 12.0);
}

/////////////////////////////////////////////////
TEST_F(BatteryTest, AddConsumer)
{
  // Create the battery
  std::unique_ptr<common::Battery> battery(new common::Battery());
  EXPECT_TRUE(battery != nullptr);

  // Invalid consumer ID
  EXPECT_FALSE(battery->SetPowerLoad(10, 5.0));

  uint32_t consumerId = battery->AddConsumer();
  EXPECT_EQ(consumerId, 0u);
  EXPECT_EQ(battery->PowerLoads().size(), 1u);

  EXPECT_TRUE(battery->SetPowerLoad(consumerId, 5.0));

  double powerLoad = 0;
  EXPECT_TRUE(battery->PowerLoad(consumerId, powerLoad));
  EXPECT_DOUBLE_EQ(powerLoad, 5.0);

  // Resetting the voltage has no effect on the power load
  battery->ResetVoltage();
  EXPECT_TRUE(battery->PowerLoad(consumerId, powerLoad));
  EXPECT_DOUBLE_EQ(powerLoad, 5.0);

  // Reinitializing the battery discard any power load
  battery->Init();
  EXPECT_EQ(battery->PowerLoads().size(), 0u);
  EXPECT_FALSE(battery->PowerLoad(consumerId, powerLoad));
}

/////////////////////////////////////////////////
TEST_F(BatteryTest, RemoveConsumer)
{
  // Create the battery
  std::unique_ptr<common::Battery> battery(new common::Battery());
  EXPECT_TRUE(battery != nullptr);

  uint32_t consumerId = battery->AddConsumer();
  EXPECT_EQ(consumerId, 0u);
  EXPECT_EQ(battery->PowerLoads().size(), 1u);

  double powerLoad = 1.0;
  EXPECT_TRUE(battery->SetPowerLoad(consumerId, powerLoad));
  EXPECT_TRUE(battery->PowerLoad(consumerId, powerLoad));
  EXPECT_DOUBLE_EQ(powerLoad, 1.0);

  uint32_t consumerId2 = battery->AddConsumer();

  EXPECT_TRUE(battery->RemoveConsumer(consumerId));
  EXPECT_EQ(battery->PowerLoads().size(), 1u);

  uint32_t consumerId3 = battery->AddConsumer();
  EXPECT_TRUE(battery->RemoveConsumer(consumerId3));
  uint32_t consumerId4 = battery->AddConsumer();

  EXPECT_FALSE(consumerId == consumerId2);
  EXPECT_FALSE(consumerId == consumerId3);
  EXPECT_FALSE(consumerId == consumerId4);

  EXPECT_FALSE(consumerId2 == consumerId3);
  EXPECT_FALSE(consumerId2 == consumerId4);

  EXPECT_FALSE(consumerId3 == consumerId4);

  EXPECT_FALSE(battery->RemoveConsumer(25));
}

/////////////////////////////////////////////////
TEST_F(BatteryTest, SetPowerLoad)
{
  // Create the battery
  std::unique_ptr<common::Battery> battery(new common::Battery());
  EXPECT_TRUE(battery != nullptr);

  // Add two consumers
  uint32_t consumerId1 = battery->AddConsumer();
  uint32_t consumerId2 = battery->AddConsumer();
  EXPECT_EQ(battery->PowerLoads().size(), 2u);

  // Set consumers power load
  double powerLoad1 = 1.0;
  double powerLoad2 = 2.0;
  EXPECT_TRUE(battery->SetPowerLoad(consumerId1, powerLoad1));
  EXPECT_TRUE(battery->SetPowerLoad(consumerId2, powerLoad2));

  // Check consumers power load
  EXPECT_TRUE(battery->PowerLoad(consumerId1, powerLoad1));
  EXPECT_DOUBLE_EQ(powerLoad1, 1.0);
  EXPECT_TRUE(battery->PowerLoad(consumerId2, powerLoad2));
  EXPECT_DOUBLE_EQ(powerLoad2, 2.0);

  // Copy power load in copy contructor
  std::unique_ptr<common::Battery> batteryCopied(new common::Battery(*battery));
  EXPECT_EQ(battery->PowerLoad(consumerId1, powerLoad1),
    batteryCopied->PowerLoad(consumerId1, powerLoad1));
  EXPECT_EQ(battery->PowerLoad(consumerId2, powerLoad2),
    batteryCopied->PowerLoad(consumerId2, powerLoad2));

  // Copy power load in assignment constructor
  common::Battery batteryAssigned;
  batteryAssigned = *battery;
  EXPECT_EQ(battery->PowerLoad(consumerId1, powerLoad1),
    batteryAssigned.PowerLoad(consumerId1, powerLoad1));
  EXPECT_EQ(battery->PowerLoad(consumerId2, powerLoad2),
    batteryAssigned.PowerLoad(consumerId2, powerLoad2));
}

/// \brief A fixture class to help with updating the battery voltage.
class BatteryUpdateFixture
{
  /// \brief Update voltage by incrementing it.
  public: double Update(common::Battery *_battery)
          {
            if (_battery)
              return _battery->Voltage() + this->step;
            else
              return -1.0;
          }

  /// \brief Voltage amount to increment by.
  public: double step;
};

/////////////////////////////////////////////////
TEST_F(BatteryTest, SetUpdateFunc)
{
  int N = 10;
  const double initVoltage = 12.0;

  // Create the battery
  std::unique_ptr<common::Battery> battery(new common::Battery());
  EXPECT_TRUE(battery != nullptr);

  battery->SetName("battery");
  battery->SetInitVoltage(initVoltage);

  battery->Init();
  EXPECT_DOUBLE_EQ(battery->Voltage(), initVoltage);

  BatteryUpdateFixture fixture;
  fixture.step = -0.1;
  battery->SetUpdateFunc(std::bind(&BatteryUpdateFixture::Update,
        &fixture, std::placeholders::_1));

  for (int i = 0; i < N; ++i)
    battery->Update();

  EXPECT_DOUBLE_EQ(battery->Voltage(), initVoltage + N * fixture.step);

  // Reinitialize the battery, and expect the same result
  battery->Init();
  EXPECT_DOUBLE_EQ(battery->Voltage(), initVoltage);

  for (int i = 0; i < N; ++i)
    battery->Update();

  EXPECT_DOUBLE_EQ(battery->Voltage(), initVoltage + N * fixture.step);

  // Reset the voltage to its initial value, and expect the same result
  battery->ResetVoltage();
  EXPECT_DOUBLE_EQ(battery->Voltage(), initVoltage);

  for (int i = 0; i < N; ++i)
    battery->Update();

  EXPECT_DOUBLE_EQ(battery->Voltage(), initVoltage + N * fixture.step);

  // Reset update function to default, and expect unchanged voltage
  double origVolt = battery->Voltage();
  battery->ResetUpdateFunc();

  for (int i = 0; i < N; ++i)
    battery->Update();

  EXPECT_DOUBLE_EQ(battery->Voltage(), origVolt);
}
