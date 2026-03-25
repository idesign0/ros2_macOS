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
#include "gz/common/KeyEvent.hh"

using namespace gz;
using namespace common;

class common::KeyEventPrivate
{
  /// \brief Event type.
  public: KeyEvent::EventType type = KeyEvent::NO_EVENT;

  /// \brief The raw value of the key pressed.
  public: int key = 0;

  /// \brief Formatted string of the key pressed (could be uppercase).
  public: std::string text = "";

  /// \brief Was control held during this key event?
  public: bool control = false;

  /// \brief Was shift held during this key event?
  public: bool shift = false;

  /// \brief Was alt held during this key event?
  public: bool alt = false;
};

/////////////////////////////////////////////////
KeyEvent::KeyEvent()
: dataPtr(new KeyEventPrivate)
{
}

/////////////////////////////////////////////////
KeyEvent::KeyEvent(const KeyEvent &_other)
  : dataPtr(new KeyEventPrivate)
{
  *dataPtr = *_other.dataPtr;
}

/////////////////////////////////////////////////
KeyEvent::KeyEvent(KeyEvent &&_other)
{
  this->dataPtr = std::move(_other.dataPtr);
  _other.dataPtr = nullptr;
}

/////////////////////////////////////////////////
KeyEvent::~KeyEvent()
{
}

/////////////////////////////////////////////////
KeyEvent::EventType KeyEvent::Type() const
{
  return this->dataPtr->type;
}

/////////////////////////////////////////////////
void KeyEvent::SetType(KeyEvent::EventType _type)
{
  this->dataPtr->type = _type;
}

/////////////////////////////////////////////////
int KeyEvent::Key() const
{
  return this->dataPtr->key;
}

/////////////////////////////////////////////////
void KeyEvent::SetKey(int _key)
{
  this->dataPtr->key = _key;
}

/////////////////////////////////////////////////
std::string KeyEvent::Text() const
{
  return this->dataPtr->text;
}

/////////////////////////////////////////////////
void KeyEvent::SetText(const std::string &_text)
{
  this->dataPtr->text = _text;
}

/////////////////////////////////////////////////
bool KeyEvent::Control() const
{
  return this->dataPtr->control;
}

/////////////////////////////////////////////////
void KeyEvent::SetControl(bool _control)
{
  this->dataPtr->control = _control;
}

/////////////////////////////////////////////////
bool KeyEvent::Shift() const
{
  return this->dataPtr->shift;
}

/////////////////////////////////////////////////
void KeyEvent::SetShift(bool _shift)
{
  this->dataPtr->shift = _shift;
}

/////////////////////////////////////////////////
bool KeyEvent::Alt() const
{
  return this->dataPtr->alt;
}

/////////////////////////////////////////////////
void KeyEvent::SetAlt(bool _alt)
{
  this->dataPtr->alt = _alt;
}

/////////////////////////////////////////////////
KeyEvent &KeyEvent::operator=(const KeyEvent &_other)
{
  if (this == &_other)
      return *this;

  *this->dataPtr = *_other.dataPtr;

  return *this;
}

/////////////////////////////////////////////////
KeyEvent &KeyEvent::operator=(KeyEvent&& other)
{
  if (this->dataPtr)
  {
    this->dataPtr.reset();
  }
  this->dataPtr = std::move(other.dataPtr);
  return *this;
}
