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
#ifndef GZ_COMMON_TIMER_HH_
#define GZ_COMMON_TIMER_HH_

#include <gz/common/Export.hh>
#include <gz/utils/ImplPtr.hh>

#include <chrono>
#include <iostream>

namespace gz
{
  namespace common
  {
    /// \class Timer Timer.hh common/common.hh
    /// \brief A timer class, used to time things in real world walltime
    class GZ_COMMON_VISIBLE Timer
    {
      /// \brief Constructor
      public: Timer();

      /// \brief Start the timer
      public: virtual void Start();

      /// \brief Stop the timer
      public: virtual void Stop();

      /// \brief Returns true if the timer is running.
      /// \return Tue if the timer has been started and not stopped.
      public: bool Running() const;

      /// \brief Get the elapsed time
      /// \return The elapsed time
      public: std::chrono::duration<double> ElapsedTime() const;

      /// \brief Stream operator friendly
      public: friend std::ostream &operator<<(std::ostream &out,
                                              const common::Timer &t)
              {
                out << t.ElapsedTime().count();
                return out;
              }

      /// \brief Private data pointer
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
