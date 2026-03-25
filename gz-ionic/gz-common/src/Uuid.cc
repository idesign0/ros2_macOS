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

#include <string>
#include <vector>

#include "gz/common/Uuid.hh"

using namespace gz;
using namespace common;

#ifdef _WIN32
/* Windows implementation using libuuid library */
//////////////////////////////////////////////////
Uuid::Uuid()
{
  RPC_STATUS Result = ::UuidCreate(&this->data);
  if (Result != RPC_S_OK)
  {
    std::cerr << "Call to UuidCreate return a non success RPC call. " <<
                 "Return code: " << Result << std::endl;
  }
}

//////////////////////////////////////////////////
Uuid::~Uuid()
{
  // No method in windows to release the uuid
}

//////////////////////////////////////////////////
std::string Uuid::String() const
{
  std::string uuidStr;
  char* szUuid = NULL;
  if (::UuidToStringA(&this->data, reinterpret_cast<RPC_CSTR*>(&szUuid)) ==
    RPC_S_OK)
  {
        uuidStr = szUuid;
        ::RpcStringFreeA(reinterpret_cast<RPC_CSTR*>(&szUuid));
  }

  return uuidStr;
}
#else
/* Unix implementation using libuuid library */

//////////////////////////////////////////////////
Uuid::Uuid()
{
  uuid_generate(this->data);
}

//////////////////////////////////////////////////
Uuid::~Uuid()
{
  uuid_clear(this->data);
}

//////////////////////////////////////////////////
std::string Uuid::String() const
{
  std::vector<char> uuidStr(Uuid::UuidStrLen);

  snprintf(&uuidStr[0], Uuid::UuidStrLen,
    "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    this->data[0], this->data[1], this->data[2], this->data[3],
    this->data[4], this->data[5], this->data[6], this->data[7],
    this->data[8], this->data[9], this->data[10], this->data[11],
    this->data[12], this->data[13], this->data[14], this->data[15]);

  // Do not include the \0 in the string.
  return std::string(uuidStr.begin(), uuidStr.end() - 1);
}

#endif
