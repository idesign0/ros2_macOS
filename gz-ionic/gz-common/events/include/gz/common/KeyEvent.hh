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
#ifndef GZ_COMMON_KEYEVENT_HH_
#define GZ_COMMON_KEYEVENT_HH_

#include <memory>
#include <string>

#include <gz/common/events/Export.hh>
#include <gz/utils/SuppressWarning.hh>

namespace gz
{
  namespace common
  {
    // Forward declare private data class
    class KeyEventPrivate;

    /// \class KeyEvent KeyEvent.hh gz/common/KeyEvent.hh
    /// \brief Generic description of a keyboard event.
    class GZ_COMMON_EVENTS_VISIBLE KeyEvent
    {
      /// \brief Key event types enumeration.
      public: enum EventType {NO_EVENT, PRESS, RELEASE};

      /// \brief Constructor.
      public: KeyEvent();

      /// \brief Copy constructor.
      /// \param[in] _other Other key event
      public: KeyEvent(const KeyEvent &_other);

      // \brief  Move constructor.
      /// \param[in] _other Other key event
      public: KeyEvent(KeyEvent &&_other);

      /// \brief Destructor
      public: ~KeyEvent();

      /// \brief Get the event type
      /// \return The key event type
      public: EventType Type() const;

      /// \brief Set the event type
      /// \param[in] _type Event type
      public: void SetType(EventType _type);

      /// \brief Get the key number
      /// \return The key number
      public: int Key() const;

      /// \brief Set the key
      /// \param[in] _key The key number
      public: void SetKey(int _key);

      /// \brief Get the formatted string of the key pressed (could be
      /// uppercase).
      /// \return The formatted string of the key pressed.
      public: std::string Text() const;

      /// \brief Set the formatted string of the key pressed.
      /// \param[in] _text Formatted string of the key pressed.
      public: void SetText(const std::string &_text);

      /// \brief Get whether the control key was held during this key event
      /// \return True if the control key was pressed
      public: bool Control() const;

      /// \brief Set whether the control key was held during this key event
      /// \param[in] _control Status of the control key
      public: void SetControl(bool _control);

      /// \brief Get whether the shift key was held during this key event
      /// \return True if the shift key was pressed
      public: bool Shift() const;

      /// \brief Set whether the shift key was held during this key event
      /// \param[in] _shift Status of the shift key
      public: void SetShift(bool _shift);

      /// \brief Get whether the alt key was held during this key event
      /// \return True if the alt key was pressed
      public: bool Alt() const;

      /// \brief Set whether the alt key was held during this key event
      /// \param[in] _alt Status of the alt key
      public: void SetAlt(bool _alt);

      /// \brief Assignment operator
      /// \param[in] _other Other key event
      /// \return this
      public: KeyEvent &operator=(const KeyEvent &_other);

      /// \brief Move assignment operator.
      /// \param[in] _other Other key event
      /// \return this
      public: KeyEvent& operator=(KeyEvent&& other);

      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief Private data pointer
      private: std::unique_ptr<KeyEventPrivate> dataPtr;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    };
  }
}
#endif
