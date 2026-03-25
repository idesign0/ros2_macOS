/*
 * Copyright 2017 Open Source Robotics Foundation
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

#ifndef GZ_COMMON_PRINTWINDOWSSYSTEMWARNING_HH_
#define GZ_COMMON_PRINTWINDOWSSYSTEMWARNING_HH_

#ifdef _WIN32
#include <windows.h>
#include <winnt.h>
#include <string>
#include <cstdint>
#include <gz/common/Console.hh>

namespace gz
{
  namespace common
  {
    /// \brief This is a Windows-specific function that prints warnings from the
    /// system in a pretty and convenient way. They get logged using the
    /// gz::common::Console.
    inline void PrintWindowsSystemWarning(const std::string &_flavorText)
    {
      // Based on example code by Microsoft: "Retrieving the Last-Error Code"
      LPVOID lpMsgBuf;
      DWORD dw = GetLastError();

      FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf, 0, NULL);

      gzwarn << _flavorText << ": " << static_cast<LPCTSTR>(lpMsgBuf) << "\n";
    }
  }
}
#endif

#endif
