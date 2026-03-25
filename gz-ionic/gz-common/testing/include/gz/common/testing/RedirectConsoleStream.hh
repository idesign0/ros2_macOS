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
#ifndef GZ_COMMON_TESTING_REDIRECTCONSOLE_STREAM_HH
#define GZ_COMMON_TESTING_REDIRECTCONSOLE_STREAM_HH

#include <string>

#include <gz/utils/ImplPtr.hh>
#include <gz/common/testing/TestPaths.hh>

#include "gz/common/testing/Export.hh"

namespace gz::common::testing
{

enum class StreamSource
{
  STDOUT,
  STDERR,
};

class GZ_COMMON_TESTING_VISIBLE RedirectConsoleStream
{
  /// \brief Constructor
  ///
  /// Redirect a choosen stream to a temporary file that can be read back.
  /// Upon destruction, any redirection will be removed.
  /// Upon destruction, the temporary file will be removed.
  /// Access contents before destruction using GetString
  ///
  /// \param[in] _source Console source to redirect (eg stdout or stderr)
  /// \param[in] _destination Destination filename
  public: RedirectConsoleStream(const StreamSource &_source,
                                const std::string &_destination);

  /// \brief Get the contents of the redirected console output
  /// This will additionally remove any redirection
  public: std::string GetString();

  /// \brief Get if the redirection is valid and enabled
  /// \return bool True if the redirection is valid
  public: bool Active() const;

  /// \brief Implementation pointer
  GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
};

/// \brief Redirect standard out to a test-specific temporary path
inline RedirectConsoleStream RedirectStdout()
{
  auto path = gz::common::testing::TempPath("stdout.out");
  return RedirectConsoleStream(StreamSource::STDOUT, path);
}

/// \brief Redirect standard error to a test-specific temporary path
inline RedirectConsoleStream RedirectStderr()
{
  auto path = gz::common::testing::TempPath("stderr.out");
  return RedirectConsoleStream(StreamSource::STDERR, path);
}
}  // namespace gz::common::testing
#endif  // GZ_COMMON_TESTING_REDIRECTCONSOLE_STREAM_HH
