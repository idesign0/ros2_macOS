/*
 * Copyright (C) 2016 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#include <gz/math/Vector2.hh>
#include "gz/common/MouseEvent.hh"

using namespace gz;
using namespace common;

/// \brief Mouse event private data
class common::MouseEventPrivate
{
  /// \brief Constructor.
  public: MouseEventPrivate()
          : pos(0, 0), prevPos(0, 0), pressPos(0, 0), scroll(0, 0),
            moveScale(0.01f), dragging(false), type(MouseEvent::NO_EVENT),
            button(MouseEvent::NO_BUTTON), buttons(MouseEvent::NO_BUTTON),
            shift(false), alt(false), control(false)
          {}

  /// \brief Mouse pointer position on the screen.
  public: math::Vector2i pos;

  /// \brief Previous position.
  public: math::Vector2i prevPos;

  /// \brief Position of button press.
  public: math::Vector2i pressPos;

  /// \brief Scroll position.
  public: math::Vector2i scroll;

  /// \brief Scaling factor.
  public: float moveScale;

  /// \brief Flag for mouse drag motion
  public: bool dragging;

  /// \brief Event type.
  public: MouseEvent::EventType type;

  /// \brief The button which caused the event.
  public: MouseEvent::MouseButton button;

  /// \brief State of the buttons when the event was generated.
  public: unsigned int buttons;

  /// \brief Shift key press flag.
  public: bool shift;

  /// \brief Alt key press flag.
  public: bool alt;

  /// \brief Control key press flag.
  public: bool control;
};

/////////////////////////////////////////////////
MouseEvent::MouseEvent()
  : dataPtr(new MouseEventPrivate)
{
}

/////////////////////////////////////////////////
MouseEvent::MouseEvent(const MouseEvent &_other)
  : dataPtr(new MouseEventPrivate)
{
  *dataPtr = *_other.dataPtr;
}

/////////////////////////////////////////////////
MouseEvent::~MouseEvent()
{
}

/////////////////////////////////////////////////
math::Vector2i MouseEvent::Pos() const
{
  return this->dataPtr->pos;
}

/////////////////////////////////////////////////
void MouseEvent::SetPos(const math::Vector2i &_pos)
{
  this->dataPtr->pos = _pos;
}

/////////////////////////////////////////////////
void MouseEvent::SetPos(int _x, int _y)
{
  this->dataPtr->pos.Set(_x, _y);
}

/////////////////////////////////////////////////
math::Vector2i MouseEvent::PrevPos() const
{
  return this->dataPtr->prevPos;
}

/////////////////////////////////////////////////
void MouseEvent::SetPrevPos(const math::Vector2i &_pos)
{
  this->dataPtr->prevPos = _pos;
}

/////////////////////////////////////////////////
void MouseEvent::SetPrevPos(int _x, int _y)
{
  this->dataPtr->prevPos.Set(_x, _y);
}

/////////////////////////////////////////////////
math::Vector2i MouseEvent::PressPos() const
{
  return this->dataPtr->pressPos;
}

/////////////////////////////////////////////////
void MouseEvent::SetPressPos(const math::Vector2i &_pos)
{
  this->dataPtr->pressPos = _pos;
}

/////////////////////////////////////////////////
void MouseEvent::SetPressPos(int _x, int _y)
{
  this->dataPtr->pressPos.Set(_x, _y);
}

/////////////////////////////////////////////////
math::Vector2i MouseEvent::Scroll() const
{
  return this->dataPtr->scroll;
}

/////////////////////////////////////////////////
void MouseEvent::SetScroll(const math::Vector2i &_scroll)
{
  this->dataPtr->scroll = _scroll;
}

/////////////////////////////////////////////////
void MouseEvent::SetScroll(int _x, int _y)
{
  this->dataPtr->scroll.Set(_x, _y);
}

/////////////////////////////////////////////////
float MouseEvent::MoveScale() const
{
  return this->dataPtr->moveScale;
}

/////////////////////////////////////////////////
void MouseEvent::SetMoveScale(float _scale)
{
  this->dataPtr->moveScale = _scale;
}

/////////////////////////////////////////////////
bool MouseEvent::Dragging() const
{
  return this->dataPtr->dragging;
}

/////////////////////////////////////////////////
void MouseEvent::SetDragging(bool _dragging)
{
  this->dataPtr->dragging = _dragging;
}

/////////////////////////////////////////////////
MouseEvent::EventType MouseEvent::Type() const
{
  return this->dataPtr->type;
}

/////////////////////////////////////////////////
void MouseEvent::SetType(MouseEvent::EventType _type)
{
  this->dataPtr->type = _type;
}

/////////////////////////////////////////////////
MouseEvent::MouseButton MouseEvent::Button() const
{
  return this->dataPtr->button;
}

/////////////////////////////////////////////////
void MouseEvent::SetButton(MouseEvent::MouseButton _button)
{
  this->dataPtr->button = _button;
}

/////////////////////////////////////////////////
unsigned int MouseEvent::Buttons() const
{
  return this->dataPtr->buttons;
}

/////////////////////////////////////////////////
void MouseEvent::SetButtons(unsigned int _buttons)
{
  this->dataPtr->buttons = _buttons;
}

/////////////////////////////////////////////////
bool MouseEvent::Shift() const
{
  return this->dataPtr->shift;
}

/////////////////////////////////////////////////
void MouseEvent::SetShift(bool _shift)
{
  this->dataPtr->shift = _shift;
}

/////////////////////////////////////////////////
bool MouseEvent::Alt() const
{
  return this->dataPtr->alt;
}

/////////////////////////////////////////////////
void MouseEvent::SetAlt(bool _alt)
{
  this->dataPtr->alt = _alt;
}

/////////////////////////////////////////////////
bool MouseEvent::Control() const
{
  return this->dataPtr->control;
}

/////////////////////////////////////////////////
void MouseEvent::SetControl(bool _control)
{
  this->dataPtr->control = _control;
}

/////////////////////////////////////////////////
MouseEvent &MouseEvent::operator=(const MouseEvent &_other)
{
  if (this == &_other)
      return *this;

  *this->dataPtr = *_other.dataPtr;

  return *this;
}
