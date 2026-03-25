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

#include <cmath>

#include <gz/common/Console.hh>
#include <gz/common/Battery.hh>

using namespace gz;
using namespace common;

/// \brief Private data for the Battery class
class common::Battery::Implementation
{
  /// \brief Initial voltage in volts.
  public: double initVoltage = 0.0;

  /// \brief Real voltage in volts.
  public: double realVoltage = 0.0;

  /// \brief Map of unique consumer ID to power loads in watts.
  public: std::map<uint32_t, double> powerLoads;

  /// \brief Counter used to produce unique consumer (powerload) ids.
  public: uint32_t powerLoadCounter = 0;

  /// \brief The function used to to update the real voltage.
  /// It takes as inputs current voltage and list of power loads.
  public: std::function<double (Battery *)> updateFunc;

  /// \brief Name of the battery.
  public: std::string name = "";

  /// \brief Mutex that protects the powerLoads map
  public: std::mutex powerLoadsMutex;
};

/////////////////////////////////////////////////
Battery::Battery()
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
  this->SetUpdateFunc(std::bind(&Battery::UpdateDefault, this,
        std::placeholders::_1));
}

/////////////////////////////////////////////////
Battery::Battery(const std::string &_name, const double _voltage)
  : Battery()
{
  this->dataPtr->name = _name;
  this->dataPtr->initVoltage = _voltage;
}

/////////////////////////////////////////////////
Battery::Battery(const Battery &_battery)
  : Battery()
{
  this->dataPtr->initVoltage = _battery.dataPtr->initVoltage;
  this->dataPtr->realVoltage = _battery.dataPtr->realVoltage;

  this->dataPtr->powerLoads.clear();
  for (auto& load : _battery.dataPtr->powerLoads)
  {
    this->dataPtr->powerLoads.insert(std::pair<uint32_t, double>(
      load.first, load.second));
  }

  this->dataPtr->powerLoadCounter = _battery.dataPtr->powerLoadCounter;
  this->dataPtr->updateFunc = _battery.dataPtr->updateFunc;

  this->dataPtr->name = _battery.dataPtr->name;
  // Mutex neither copyable nor movable.
}

/////////////////////////////////////////////////
Battery::Battery(Battery &&_battery)
  : dataPtr(std::exchange(_battery.dataPtr, nullptr))
{
}

/////////////////////////////////////////////////
Battery &Battery::operator=(const Battery &_battery)
{
  this->dataPtr->initVoltage = _battery.dataPtr->initVoltage;
  this->dataPtr->realVoltage = _battery.dataPtr->realVoltage;

  this->dataPtr->powerLoads.clear();
  for (auto& load : _battery.dataPtr->powerLoads)
  {
    this->dataPtr->powerLoads.insert(std::pair<uint32_t, double>(
      load.first, load.second));
  }

  this->dataPtr->powerLoadCounter = _battery.dataPtr->powerLoadCounter;
  this->dataPtr->updateFunc = _battery.dataPtr->updateFunc;

  this->dataPtr->name = _battery.dataPtr->name;
  // Mutex neither copyable nor movable.
  return *this;
}

/////////////////////////////////////////////////
Battery &Battery::operator=(Battery &&_battery)
{
  std::swap(this->dataPtr, _battery.dataPtr);
  return *this;
}

/////////////////////////////////////////////////
bool Battery::operator==(const Battery &_battery) const
{
  if (_battery.Name() == this->Name() &&
    std::abs(_battery.InitVoltage() - this->InitVoltage()) < 1e-6)
    return true;
  else
    return false;
}

/////////////////////////////////////////////////
bool Battery::operator!=(const Battery &_battery) const
{
  return !(*this == _battery);
}

/////////////////////////////////////////////////
void Battery::Init()
{
  this->ResetVoltage();
  this->InitConsumers();
}

/////////////////////////////////////////////////
void Battery::ResetVoltage()
{
  this->dataPtr->realVoltage = std::max(0.0, this->dataPtr->initVoltage);
}

//////////////////////////////////////////////////
double Battery::InitVoltage() const
{
  return this->dataPtr->initVoltage;
}

//////////////////////////////////////////////////
void Battery::SetInitVoltage(const double _voltage)
{
  this->dataPtr->initVoltage = _voltage;
}

//////////////////////////////////////////////////
std::string Battery::Name() const
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////
void Battery::SetName(const std::string &_name) const
{
  this->dataPtr->name = _name;
}

//////////////////////////////////////////////////
void Battery::InitConsumers()
{
  std::lock_guard<std::mutex> lock(this->dataPtr->powerLoadsMutex);
  this->dataPtr->powerLoads.clear();
}

/////////////////////////////////////////////////
uint32_t Battery::AddConsumer()
{
  std::lock_guard<std::mutex> lock(this->dataPtr->powerLoadsMutex);
  uint32_t newId = this->dataPtr->powerLoadCounter++;
  this->dataPtr->powerLoads[newId] = 0.0;
  return newId;
}

/////////////////////////////////////////////////
bool Battery::RemoveConsumer(uint32_t _consumerId)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->powerLoadsMutex);
  if (this->dataPtr->powerLoads.erase(_consumerId))
  {
    return true;
  }
  else
  {
    gzerr << "Invalid battery consumer id[" << _consumerId << "]\n";
    return false;
  }
}

/////////////////////////////////////////////////
bool Battery::SetPowerLoad(const uint32_t _consumerId, const double _powerLoad)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->powerLoadsMutex);
  auto iter = this->dataPtr->powerLoads.find(_consumerId);
  if (iter == this->dataPtr->powerLoads.end())
  {
    gzerr << "Invalid param value[_consumerId] : " << _consumerId << "\n";
    return false;
  }

  iter->second = _powerLoad;
  return true;
}

/////////////////////////////////////////////////
bool Battery::PowerLoad(const uint32_t _consumerId, double &_powerLoad) const
{
  std::lock_guard<std::mutex> lock(this->dataPtr->powerLoadsMutex);
  auto iter = this->dataPtr->powerLoads.find(_consumerId);
  if (iter == this->dataPtr->powerLoads.end())
  {
    gzerr << "Invalid param value[_consumerId] : " << _consumerId << "\n";
    return false;
  }

  _powerLoad = iter->second;
  return true;
}

/////////////////////////////////////////////////
const Battery::PowerLoad_M &Battery::PowerLoads() const
{
  std::lock_guard<std::mutex> lock(this->dataPtr->powerLoadsMutex);
  return this->dataPtr->powerLoads;
}

/////////////////////////////////////////////////
double Battery::Voltage() const
{
  return this->dataPtr->realVoltage;
}

/////////////////////////////////////////////////
void Battery::Update()
{
  this->dataPtr->realVoltage = std::max(0.0, this->dataPtr->updateFunc(this));
}

/////////////////////////////////////////////////
double Battery::UpdateDefault(Battery *_battery)
{
  // Ideal battery
  if (_battery)
    return _battery->Voltage();
  else
    return 0.0;
}

/////////////////////////////////////////////////
void Battery::SetUpdateFunc(
    std::function<double (Battery *)> _updateFunc)
{
  this->dataPtr->updateFunc = _updateFunc;
}

/////////////////////////////////////////////////
void Battery::ResetUpdateFunc()
{
  this->SetUpdateFunc(std::bind(&Battery::UpdateDefault, this,
        std::placeholders::_1));
}
