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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

#include <gz/common/testing/RedirectConsoleStream.hh>
#include <gz/common/Console.hh>
#include <gz/common/testing/TestPaths.hh>
#include <gz/utils/SuppressWarning.hh>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

namespace gz::common::testing
{

class RedirectConsoleStream::Implementation
{
  /// \brief Console source being redirected
  public: StreamSource source;

  /// \brief Filename to write console output to
  public: std::string sink {""};

  /// \brief Holds the redirected file descriptor
  public: int fd {-1};

  /// \brief Holds the original source file descriptor
  public: int originalFd {-1};

  /// \brief Remove any console redirection, restoring original sink
  public: void RemoveRedirection();

  /// \brief Destructor
  public: ~Implementation();
};

//////////////////////////////////////////////////
void RedirectConsoleStream::Implementation::RemoveRedirection()
{
  // MSVC treats dup and dup2 as deprecated, preferring _dup and _dup2
  // We can safely ignore that here.
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION;
  /// Restore the orignal source file descriptor
  if (this->originalFd != -1)
  {
    fflush(nullptr);
    dup2(this->originalFd, this->fd);
    close(this->originalFd);
    this->originalFd = -1;
  }
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION;
}

//////////////////////////////////////////////////
RedirectConsoleStream::Implementation::~Implementation()
{
  this->RemoveRedirection();

  if (!this->sink.empty() && common::exists(this->sink))
  {
    common::removeFile(this->sink);
  }
}

//////////////////////////////////////////////////
int GetSourceFd(const StreamSource &_source)
{
  switch(_source)
  {
    case StreamSource::STDOUT:
      return fileno(stdout);
      break;
    case StreamSource::STDERR:
      return fileno(stderr);
      break;
    default:
      return -1;
  }
}

//////////////////////////////////////////////////
RedirectConsoleStream::RedirectConsoleStream(const StreamSource &_source,
    const std::string &_destination):
  dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
  // MSVC treats dup and dup2 as deprecated, preferring _dup and _dup2
  // We can safely ignore that here.
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION;
  this->dataPtr->sink = _destination;

  if (this->dataPtr->sink.empty())
  {
    gzerr << "Failed to open sink file: console redirection disabled"
      << "(empty filename)" << std::endl;
    return;
  }

  if (common::exists(this->dataPtr->sink))
  {
    gzerr << "Failed to open sink file: console redirection disabled"
      << "(destination exists)" << std::endl;
    return;
  }

  this->dataPtr->fd = GetSourceFd(_source);

  // Store the fd so that it can be restored upon destruction
  this->dataPtr->originalFd = dup(this->dataPtr->fd);

  int sinkFd;
  // Create a file with read/write permissions and exclusive access
  if ((sinkFd = open(this->dataPtr->sink.c_str(),
          O_EXCL | O_RDWR | O_CREAT,
          S_IREAD | S_IWRITE)) < 0)
  {
    gzerr << "Failed to open sink file, console redirection disabled"
      << "(" << strerror(sinkFd) << ")" << std::endl;
    return;
  }

  fflush(nullptr);
  // Duplicate the sink file descriptor onto the source
  dup2(sinkFd, this->dataPtr->fd);
  // Close the file handle;
  close(sinkFd);
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION;
}

//////////////////////////////////////////////////
std::string RedirectConsoleStream::GetString()
{
  this->dataPtr->RemoveRedirection();

  // Read the file contents and return to the user
  std::ifstream input(this->dataPtr->sink);
  std::stringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

//////////////////////////////////////////////////
bool RedirectConsoleStream::Active() const
{
  return !this->dataPtr->sink.empty() && this->dataPtr->originalFd != -1;
}

}  // namespace gz::common::testing
