/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <zmq.hpp>

#include <vector>

#include "gz/transport/Discovery.hh"

// Compatibility macro for ZMQ_FD_T
#if (ZMQ_VERSION >= 40303)
  #define ZMQ_FD_T zmq_fd_t
#else
// Logic from newer zmq.h
  #if defined _WIN32
  // Windows uses a pointer-sized unsigned integer to store the socket fd.
    #if defined _WIN64
      #define ZMQ_FD_T unsigned __int64
    #else
      #define ZMQ_FD_T unsigned int
    #endif
  #else
    #define ZMQ_FD_T int
  #endif
#endif

namespace gz::transport
{
inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  /////////////////////////////////////////////////
  bool pollSockets(const std::vector<int> &_sockets, const int _timeout)
  {
    zmq::pollitem_t items[] =
    {
      {0, static_cast<ZMQ_FD_T>(_sockets.at(0)), ZMQ_POLLIN, 0},
    };

    try
    {
      zmq::poll(&items[0], sizeof(items) / sizeof(items[0]),
          std::chrono::milliseconds(_timeout));
    }
    catch(...)
    {
      return false;
    }

    // Return if we got a reply.
    return items[0].revents & ZMQ_POLLIN;
  }
}  // namespace GZ_TRANSPORT_VERSION_NAMESPACE
}  // namespace gz::transport
