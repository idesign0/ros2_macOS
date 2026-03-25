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
#ifndef GZ_COMMON_UUID_HH_INCLUDED_
#define GZ_COMMON_UUID_HH_INCLUDED_

#include <iostream>
#include <string>

#include <gz/common/Export.hh>

#ifdef _WIN32
  #include <Rpc.h>
  #pragma comment(lib, "Rpcrt4.lib")
  using portable_uuid_t = UUID;
// else unix
#else
  #include <uuid/uuid.h>
  using portable_uuid_t = uuid_t;
#endif

namespace gz
{
  namespace common
  {
    /// \brief A portable class for representing a Universally Unique Identifier
    class GZ_COMMON_VISIBLE Uuid
    {
      /// \brief Constructor.
      public: Uuid();

      /// \brief Destructor.
      public: virtual ~Uuid();

      /// \brief Return the string representation of the Uuid.
      /// \return the UUID in string format.
      public: std::string String() const;

      /// \brief Stream insertion operator.
      /// \param[out] _out The output stream.
      /// \param[in] _msg AdvMsg to write to the stream.
      public: friend std::ostream &operator<<(std::ostream &_out,
                  const gz::common::Uuid &_uuid)
      {
        _out << _uuid.String();
        return _out;
      }

      /// \brief Length of a UUID in string format.
      /// A UUID is a 16-octet number. In its string representation, every octet
      /// is divided in two parts, and each part (4 bits) is represented as an
      /// hexadecimal value. A UUID is also displayed in five groups separated
      /// by hyphens, in the form 8-4-4-4-12 for a total of 36 characters.
      /// To summarize: 36 octets + \0 = 37 octets.
      private: static const int UuidStrLen = 37;

      /// \brief Internal representation.
      private: portable_uuid_t data;
    };
  }
}
#endif
