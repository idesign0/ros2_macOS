/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_SIGNALHANDLER_HH_
#define GZ_COMMON_SIGNALHANDLER_HH_

#include <functional>

#include <gz/common/Export.hh>

namespace gz
{
  namespace common
  {
    // Forward declarations.
    class SignalHandlerPrivate;

    /// \brief Register callbacks that get triggered on SIGINT and SIGTERM.
    ///
    /// This class is a thread-safe interface to system signals (SIGINT and
    /// SIGTERM). Use the AddCallback(std::function<void(int)>)
    /// function to register callbacks that
    /// should be called when a SIGINT or SIGTERM is triggered.
    ///
    /// Example using a lambda callback:
    ///
    /// ~~~
    /// gz::common::SignalHandler handler;
    /// handler.AddCallback([] (int _sig)
    /// {
    ///   printf("Signal[%d] received\n", _sig);
    /// });
    /// ~~~
    ///
    class GZ_COMMON_VISIBLE SignalHandler
    {
      /// \brief Constructor
      public: SignalHandler();

      /// \brief Destructor.
      public: virtual ~SignalHandler();

      /// \brief Add a callback to execute when a signal is received.
      /// \param[in] _cb Callback to execute.
      /// \return True if the callback was added. A callback may not be
      /// added if the SignalHandler was not successfully initialized.
      /// \sa bool Initialized() const
      public: bool AddCallback(std::function<void(int)> _cb);

      /// \brief Get whether the signal handlers were successfully
      /// initialized.
      /// \return True if the signal handlers were successfully created.
      public: bool Initialized() const;

      /// \brief Set whether the signal handler was successfully
      /// initialized. This function is here for testing purposes, and
      /// if for some reason a person needs to create a subclass to control
      /// the initialization value.
      /// \param[in] _init New value of the initialized member variabl.e
      protected: virtual void SetInitialized(const bool _init);

      /// \brief Private data pointer
      private: SignalHandlerPrivate *dataPtr;
    };
  }
}
#endif
