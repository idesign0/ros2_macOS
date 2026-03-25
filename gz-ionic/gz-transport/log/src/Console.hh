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

#ifndef GZ_TRANSPORT_LOG_SRC_CONSOLE_HH_
#define GZ_TRANSPORT_LOG_SRC_CONSOLE_HH_

#include <iostream>

#include <gz/transport/config.hh>

namespace gz
{
  namespace transport
  {
    namespace log
    {
      // Inline bracket to help doxygen filtering.
      inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
        //
        extern int __verbosity;
      }
    }
  }
}

#define LDBG(statements) do { \
  if (gz::transport::log::__verbosity >= 4) {std::cout << statements;} \
} while (false)

#define LMSG(statements) do { \
  if (gz::transport::log::__verbosity >= 3) {std::cout << statements;} \
} while (false)

#define LWRN(statements) do { \
  if (gz::transport::log::__verbosity >= 2) {std::cout << statements;} \
} while (false)

#define LERR(statements) do { \
  if (gz::transport::log::__verbosity >= 1) {std::cerr << statements;} \
} while (false)

#define LFATAL(statements) do { \
  if (gz::transport::log::__verbosity >= 0) {std::cerr << statements;} \
} while (false)

#endif
