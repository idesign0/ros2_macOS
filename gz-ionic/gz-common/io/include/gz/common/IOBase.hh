/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_IO_HH_
#define GZ_COMMON_IO_HH_

#include <sstream>
#include <string>

namespace gz
{
  namespace common
  {
    /// \brief Traits for type-specific object I/O.
    ///
    /// To be fully specialized as needed.
    template <typename T>
    struct IO
    {
      /// \brief Read object from stream.
      ///
      /// This default implementation relies on stream operator overloads.
      ///
      /// \param[in] _istream Stream to read object from.
      /// \return object instance.
      static T ReadFrom(std::istream &_istream)
      {
        T value;
        _istream >> value;
        return value;
      }

      /// \brief Read object from string.
      ///
      /// This default implementation relies on stream operator overloads.
      ///
      /// \param[in] _string String to read object from.
      /// \return object instance.
      static T ReadFrom(const std::string &_string)
      {
        std::istringstream stream{_string};
        return ReadFrom(stream);
      }
    };

    /// \brief Traits for string I/O.
    template<>
    struct IO<std::string>
    {
      /// \brief Read object from stream.
      ///
      /// This default implementation relies on stream operator overloads.
      ///
      /// \param[in] _istream Stream to read object from.
      /// \return object instance.
      static std::string ReadFrom(std::istream &_istream)
      {
        std::string value;
        _istream >> value;
        return value;
      }

      /// \brief Read string from string (copy as-is).
      static std::string ReadFrom(std::string _string)
      {
        return _string;
      }
    };
  }
}

#endif
