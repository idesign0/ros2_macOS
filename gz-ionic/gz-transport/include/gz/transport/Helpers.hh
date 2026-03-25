/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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

#ifndef GZ_TRANSPORT_HELPERS_HH_
#define GZ_TRANSPORT_HELPERS_HH_

#include <zmq.hpp>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

#include "gz/transport/config.hh"
#include "gz/transport/Export.hh"

// Avoid using deprecated message send/receive function when possible.
#if ZMQ_VERSION > ZMQ_MAKE_VERSION(4, 3, 1)
  #define GZ_ZMQ_POST_4_3_1
#endif

// Avoid using deprecated set function when possible
#if CPPZMQ_VERSION >= ZMQ_MAKE_VERSION(4, 7, 0)
  // Ubuntu Focal (20.04) packages a different "4.7.0"
  #ifndef UBUNTU_FOCAL
    #define GZ_CPPZMQ_POST_4_7_0
  #endif
#endif

namespace gz::transport
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  /// \brief Constant used when not interested in throttling.
  static const uint64_t kUnthrottled = std::numeric_limits<uint64_t>::max();

  /// \brief Find the environment variable '_name' and return its value.
  /// \param[in] _name Name of the environment variable.
  /// \param[out] _value Value if the variable was found.
  /// \return True if the variable was found or false otherwise.
  bool GZ_TRANSPORT_VISIBLE env(const std::string &_name,
                                      std::string &_value);

  /// \brief split at a one character delimiter to get a vector of something
  /// \param[in] _orig The string to split
  /// \param[in] _delim a character to split the string at
  /// \returns vector of split pieces of the string excluding the delimiter
  std::vector<std::string> GZ_TRANSPORT_VISIBLE split(
      const std::string &_orig,
      char _delim);

  /// \brief Portable function to get the id of the current process.
  /// \returns id of current process
  unsigned int GZ_TRANSPORT_VISIBLE getProcessId();

  // Use safer functions on Windows
  #ifdef _MSC_VER
    #define gz_strcat strcat_s
    #define gz_strcpy strcpy_s
    #define gz_sprintf sprintf_s
    #define gz_strdup _strdup
  #else
    #define gz_strcat std::strcat
    #define gz_strcpy std::strcpy
    #define gz_sprintf std::sprintf
    #define gz_strdup strdup
  #endif
  }
}

// GZ_TRANSPORT_HELPERS_HH_
#endif
