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

#include <cstdint>

#include "gz/transport/Helpers.hh"
#include "gz/transport/SubscribeOptions.hh"

#include "SubscribeOptionsPrivate.hh"

namespace gz::transport
{
//////////////////////////////////////////////////
SubscribeOptions::SubscribeOptions()
  : dataPtr(new SubscribeOptionsPrivate())
{
}

//////////////////////////////////////////////////
SubscribeOptions::SubscribeOptions(const SubscribeOptions &_otherSubscribeOpts)
  : dataPtr(new SubscribeOptionsPrivate(*_otherSubscribeOpts.dataPtr))
{
}

//////////////////////////////////////////////////
SubscribeOptions::~SubscribeOptions()
{
}

//////////////////////////////////////////////////
bool SubscribeOptions::Throttled() const
{
  return this->MsgsPerSec() != kUnthrottled;
}

//////////////////////////////////////////////////
uint64_t SubscribeOptions::MsgsPerSec() const
{
  return this->dataPtr->msgsPerSec;
}

//////////////////////////////////////////////////
void SubscribeOptions::SetMsgsPerSec(const uint64_t _newMsgsPerSec)
{
  this->dataPtr->msgsPerSec = _newMsgsPerSec;
}

//////////////////////////////////////////////////
bool SubscribeOptions::IgnoreLocalMessages() const
{
  return this->dataPtr->ignoreLocalMessages;
}

//////////////////////////////////////////////////
void SubscribeOptions::SetIgnoreLocalMessages(bool _ignore)
{
  this->dataPtr->ignoreLocalMessages = _ignore;
}
}  // namespace gz::transport
