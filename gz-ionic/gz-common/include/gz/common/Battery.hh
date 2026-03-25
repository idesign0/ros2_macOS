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
#ifndef GZ_COMMON_BATTERY_HH_
#define GZ_COMMON_BATTERY_HH_

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include <gz/common/Export.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \brief A battery abstraction
    ///
    /// The default battery model is ideal: It just takes the initial voltage
    /// value as its constant voltage value. This behavior can be changed by
    /// specifying a custom update function.
    ///
    /// The battery handles a list of consumers. It updates itself after each
    /// simulation iteration. The update function takes the power loads for each
    /// consumer and current voltage value as inputs and returns a new voltage
    /// value.
    class GZ_COMMON_VISIBLE Battery
    {
      /// \brief Typedef the powerload map.
      /// \sa SetUpdateFunc
      public: typedef std::map<uint32_t, double> PowerLoad_M;

      /// \brief Constructor
      public: explicit Battery();

      /// \brief Constructor that sets the name and initial voltage.
      /// \param[in] _name Name of the battery
      /// \param[in] _voltage Initial voltage of the battery
      public: Battery(const std::string &_name, const double _voltage);

      /// \brief Copy constructor
      /// \param[in] _battery Battery to copy.
      public: Battery(const Battery &_battery);

      /// \brief Copy assignment operator
      /// \param[in] _battery The new battery
      /// \return a reference to this instance
      public: Battery &operator=(const Battery &_battery);

      /// \brief Move constructor
      /// \param[in] _battery Battery to move.
      public: Battery(Battery &&_battery);

      /// \brief Move assignment operator
      /// \param[in] _battery The new battery
      /// \return a reference to this instance
      public: Battery &operator=(Battery &&_battery);

      /// \brief Destructor.
      public: virtual ~Battery() = default;

      /// \brief Equal to operator
      /// \param[in] _battery the battery to compare to
      /// \return true if names and initial voltages are the same, false
      /// otherwise
      public: bool operator==(const Battery &_battery) const;

      /// \brief Inequality operator
      /// \param[in] _battery the battery to compare to
      /// \return true if names or initial voltages are not the same, false
      /// otherwise
      public: bool operator!=(const Battery &_battery) const;

      /// \brief Initialize.
      public: virtual void Init();

      /// \brief Reset the battery voltage to the initial value.
      /// The initial value might have been loaded from an sdf element.
      /// \sa Load
      public: virtual void ResetVoltage();

      /// \brief Return the initial voltage.
      /// \return The initial voltage.
      public: double InitVoltage() const;

      /// \brief Set the initial voltage
      /// \param[in] _voltage Initial voltage.
      public: virtual void SetInitVoltage(const double _voltage);

      /// \brief Return the name of the battery.
      /// \return The name of the battery.
      public: std::string Name() const;

      /// \brief Set the name of the battery.
      /// \param[in] _name Name of the battery.
      public: void SetName(const std::string &_name) const;

      /// \brief Create a unique consumer.
      /// \return Unique consumer identifier.
      public: uint32_t AddConsumer();

      /// \brief Remove a consumer.
      /// \param[in] _consumerId Unique consumer identifier.
      /// \return True if the consumer was removed. False if the consumer id
      /// was not found.
      public: bool RemoveConsumer(const uint32_t _consumerId);

      /// \brief Set consumer power load in watts.
      /// \param[in] _consumerId Unique consumer identifier.
      /// \param[in] _powerLoad Power load in watts.
      /// \return True if setting the power load consumption was successful.
      public: bool SetPowerLoad(const uint32_t _consumerId,
                                const double _powerLoad);

      /// \brief Get consumer power load in watts.
      /// \param[in] _consumerId Unique consumer identifier.
      /// \param[out] _powerLoad Power load consumption in watts.
      /// \return True if getting the power load consumption was successful.
      public: bool PowerLoad(const uint32_t _consumerId,
                             double &_powerLoad) const;

      /// \brief Get list of power loads in watts.
      /// \return List of power loads in watts.
      public: const PowerLoad_M &PowerLoads() const;

      /// \brief Get the real voltage in volts.
      /// \return Voltage.
      public: double Voltage() const;

      /// \brief Setup function to update voltage.
      /// \param[in] _updateFunc The update function callback that is used
      /// to modify the battery's voltage. The parameter to the update
      /// function callback is a reference to an instance of
      /// Battery::UpdateData. The update function must return the new
      /// battery voltage as a double.
      /// \sa UpdateData
      public: void SetUpdateFunc(
                  std::function<double (Battery *)> _updateFunc);

      /// \brief Reset function to update voltage, upon destruction of current
      /// callback function.
      /// \sa UpdateDefault
      public: void ResetUpdateFunc();

      /// \brief Update the battery. This will in turn trigger the function
      /// set using the SetUpdateFunc function.
      /// \sa SetUpdateFunc.
      public: void Update();

      /// \brief Initialize the list of consumers.
      protected: void InitConsumers();

      /// \brief Update voltage using an ideal battery model.
      /// \param[in] _battery Pointer to the battery.
      /// \return New battery voltage.
      private: double UpdateDefault(Battery *_battery);

      /// \brief Pointer to private data.
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };

    /// \def BatteryPtr
    /// \brief Shared pointer to a battery
    typedef std::shared_ptr<Battery> BatteryPtr;
  }
}
#endif
