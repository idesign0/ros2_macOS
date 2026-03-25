/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#include <cstdint>
#include <cstring>
#include <iostream>

#include "gz/transport/AdvertiseOptions.hh"
#include "gz/transport/Helpers.hh"

using namespace gz;
using namespace transport;

namespace gz::transport
{
inline namespace GZ_TRANSPORT_VERSION_NAMESPACE
{
/// \internal
/// \brief Private data for AdvertiseOptions class.
class AdvertiseOptionsPrivate
{
  /// \brief Constructor.
  public: AdvertiseOptionsPrivate() = default;

  /// \brief Destructor.
  public: virtual ~AdvertiseOptionsPrivate() = default;

  /// \brief Default scope value.
  public: Scope_t scope = Scope_t::ALL;
};

/// \internal
/// \brief Private data for AdvertiseMessageOptions class.
class AdvertiseMessageOptionsPrivate
{
  /// \brief Constructor.
  public: AdvertiseMessageOptionsPrivate() = default;

  /// \brief Destructor.
  public: virtual ~AdvertiseMessageOptionsPrivate() = default;

  /// \brief Default message publication rate.
  public: uint64_t msgsPerSec = kUnthrottled;
};

/// \internal
/// \brief Private data for AdvertiseServiceOptions class.
class AdvertiseServiceOptionsPrivate
{
  /// \brief Constructor.
  public: AdvertiseServiceOptionsPrivate() = default;

  /// \brief Destructor.
  public: virtual ~AdvertiseServiceOptionsPrivate() = default;
};

//////////////////////////////////////////////////
AdvertiseOptions::AdvertiseOptions()
  : dataPtr(new AdvertiseOptionsPrivate())
{
}

//////////////////////////////////////////////////
AdvertiseOptions::AdvertiseOptions(const AdvertiseOptions &_other)
  : AdvertiseOptions()
{
  (*this) = _other;
}

//////////////////////////////////////////////////
AdvertiseOptions::~AdvertiseOptions()
{
}

//////////////////////////////////////////////////
AdvertiseOptions &AdvertiseOptions::operator=(const AdvertiseOptions &_other)
{
  this->SetScope(_other.Scope());
  return *this;
}

//////////////////////////////////////////////////
bool AdvertiseOptions::operator==(const AdvertiseOptions &_other) const
{
  return this->Scope() == _other.Scope();
}

//////////////////////////////////////////////////
bool AdvertiseOptions::operator!=(const AdvertiseOptions &_other) const
{
  return !(*this == _other);
}

//////////////////////////////////////////////////
const Scope_t &AdvertiseOptions::Scope() const
{
  return this->dataPtr->scope;
}

//////////////////////////////////////////////////
void AdvertiseOptions::SetScope(const Scope_t &_scope)
{
  this->dataPtr->scope = _scope;
}

//////////////////////////////////////////////////
AdvertiseMessageOptions::AdvertiseMessageOptions()
  : AdvertiseOptions(),
    dataPtr(new AdvertiseMessageOptionsPrivate())
{
}

//////////////////////////////////////////////////
AdvertiseMessageOptions::AdvertiseMessageOptions(
  const AdvertiseMessageOptions &_other)
  : AdvertiseMessageOptions()
{
  (*this) = _other;
}

//////////////////////////////////////////////////
AdvertiseMessageOptions::~AdvertiseMessageOptions()
{
}

//////////////////////////////////////////////////
AdvertiseMessageOptions &AdvertiseMessageOptions::operator=(
  const AdvertiseMessageOptions &_other)
{
  AdvertiseOptions::operator=(_other);
  this->SetMsgsPerSec(_other.MsgsPerSec());
  return *this;
}

//////////////////////////////////////////////////
bool AdvertiseMessageOptions::operator==(
  const AdvertiseMessageOptions &_other) const
{
  return AdvertiseOptions::operator==(_other) &&
         this->MsgsPerSec() == _other.MsgsPerSec();
}

//////////////////////////////////////////////////
bool AdvertiseMessageOptions::operator!=(
  const AdvertiseMessageOptions &_other) const
{
  return !(*this == _other);
}

//////////////////////////////////////////////////
bool AdvertiseMessageOptions::Throttled() const
{
  return this->MsgsPerSec() != kUnthrottled;
}

//////////////////////////////////////////////////
uint64_t AdvertiseMessageOptions::MsgsPerSec() const
{
  return this->dataPtr->msgsPerSec;
}

//////////////////////////////////////////////////
void AdvertiseMessageOptions::SetMsgsPerSec(const uint64_t _newMsgsPerSec)
{
  this->dataPtr->msgsPerSec = _newMsgsPerSec;
}

//////////////////////////////////////////////////
AdvertiseServiceOptions::AdvertiseServiceOptions()
  : AdvertiseOptions(),
    dataPtr(new AdvertiseServiceOptionsPrivate())
{
}

//////////////////////////////////////////////////
AdvertiseServiceOptions::AdvertiseServiceOptions(
  const AdvertiseServiceOptions &_other)
  : AdvertiseServiceOptions()
{
  (*this) = _other;
}

//////////////////////////////////////////////////
AdvertiseServiceOptions::~AdvertiseServiceOptions()
{
}

//////////////////////////////////////////////////
AdvertiseServiceOptions &AdvertiseServiceOptions::operator=(
  const AdvertiseServiceOptions &_other)
{
  AdvertiseOptions::operator=(_other);
  return *this;
}

//////////////////////////////////////////////////
bool AdvertiseServiceOptions::operator==(
  const AdvertiseServiceOptions &_other) const
{
  return AdvertiseOptions::operator==(_other);
}

//////////////////////////////////////////////////
bool AdvertiseServiceOptions::operator!=(
  const AdvertiseServiceOptions &_other) const
{
  return !(*this == _other);
}
}  // namespace GZ_TRANSPORT_VERSION_NAMESPACE
}  // namespace gz::transport
