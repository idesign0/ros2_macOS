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

#include "implptr_test_classes.hh"

#include <gz/utils/ImplPtr.hh>

#include <string>
#include <thread>

using namespace gz;
using namespace gz::implptr_test_classes;

class gz::implptr_test_classes::ObjectPrivate
{
  public: void TestFunc()
  {
    this->ivalue += 1;
  }

  public: int ivalue;
  public: std::string svalue;
};

//////////////////////////////////////////////////
CopyableObject::CopyableObject(const int _ivalue,
                               const std::string &_svalue)
  : dataPtr(utils::MakeImpl<ObjectPrivate>(_ivalue, _svalue))
{
  // Do nothing
}

//////////////////////////////////////////////////
int CopyableObject::GetInt() const
{
  return dataPtr->ivalue;
}

//////////////////////////////////////////////////
void CopyableObject::SetInt(const int _value)
{
  dataPtr->ivalue = _value;
}

//////////////////////////////////////////////////
const std::string &CopyableObject::GetString() const
{
  return (*dataPtr).svalue;
}

//////////////////////////////////////////////////
void CopyableObject::SetString(const std::string &_value)
{
  (*dataPtr).svalue = _value;
}

//////////////////////////////////////////////////
int CopyableObject::ThreadIncrementInt()
{
  this->SetInt(0);
  auto thread = std::thread(&ObjectPrivate::TestFunc, this->dataPtr.Get());
  thread.join();
  return this->GetInt();
}

//////////////////////////////////////////////////
MovableObject::MovableObject(const int _ivalue, const std::string &_svalue)
  : dataPtr(utils::MakeUniqueImpl<ObjectPrivate>(_ivalue, _svalue))
{
  // Do nothing
}

//////////////////////////////////////////////////
int MovableObject::GetInt() const
{
  return dataPtr->ivalue;
}

//////////////////////////////////////////////////
void MovableObject::SetInt(const int _value)
{
  dataPtr->ivalue = _value;
}

//////////////////////////////////////////////////
const std::string &MovableObject::GetString() const
{
  return (*dataPtr).svalue;
}

//////////////////////////////////////////////////
void MovableObject::SetString(const std::string &_value)
{
  (*dataPtr).svalue = _value;
}

//////////////////////////////////////////////////
int MovableObject::ThreadIncrementInt()
{
  this->SetInt(0);
  auto thread = std::thread(&ObjectPrivate::TestFunc, this->dataPtr.get());
  thread.join();
  return this->GetInt();
}

//////////////////////////////////////////////////
class CopyableObjectAlt::Implementation
{
  public: int ivalue;
  public: std::string svalue;
};

//////////////////////////////////////////////////
CopyableObjectAlt::CopyableObjectAlt(const int _ivalue,
                               const std::string &_svalue)
  : dataPtr(utils::MakeImpl<Implementation>(_ivalue, _svalue))
{
  // Do nothing
}

//////////////////////////////////////////////////
int CopyableObjectAlt::GetInt() const
{
  return dataPtr->ivalue;
}

//////////////////////////////////////////////////
void CopyableObjectAlt::SetInt(const int _value)
{
  dataPtr->ivalue = _value;
}

//////////////////////////////////////////////////
const std::string &CopyableObjectAlt::GetString() const
{
  return (*dataPtr).svalue;
}

//////////////////////////////////////////////////
void CopyableObjectAlt::SetString(const std::string &_value)
{
  (*dataPtr).svalue = _value;
}
