/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#ifndef GZ_UTILS__SUBPROCESS_HH_
#define GZ_UTILS__SUBPROCESS_HH_

#include "detail/subprocess.h"
#include "gz/utils/Environment.hh"

#include <iostream>
#include <string>
#include <vector>
#include <utility>

#include <gz/utils/detail/subprocess.h>

// This header contains a wrapper to the cross-platform subprocess library.
// It can be used to spawn processes to interact with, primarily for testing.

namespace gz
{
namespace utils
{

/// \brief Create a RAII-type object that encapsulates a subprocess.
class Subprocess
{
  /// \brief Constructor
  ///
  /// This variant will spawn a subprocess that inherits the environment
  /// from the calling process.
  ///
  /// \param[in] _commandLine set of arguments starting with an executable
  ///   used to spawn the subprocess
  public: explicit Subprocess(const std::vector<std::string> &_commandLine):
    commandLine(_commandLine),
    environment({}),
    inheritEnvironment(true)
  {
    this->Create();
  }

  /// \brief Constructor
  ///
  /// This variant will spawn a subprocess that uses the user-specified
  /// environment
  ///
  /// \param[in] _commandLine set of arguments starting with an executable
  ///   used to spawn the subprocess
  /// \param[in] _environment environment variables to set in the spawned
  ///   subprocess
  public: Subprocess(const std::vector<std::string> &_commandLine,
                     gz::utils::EnvironmentMap _environment):
    commandLine(_commandLine),
    environment(std::move(_environment)),
    inheritEnvironment(false)
  {
    this->Create();
  }

  /// \brief Constructor
  ///
  /// This variant will spawn a subprocess that uses the user-specified
  /// environment
  ///
  /// \param[in] _commandLine set of arguments starting with an executable
  ///   used to spawn the subprocess
  /// \param[in] _environment environment variables to set in the spawned
  ///   subprocess
  public: Subprocess(const std::vector<std::string> &_commandLine,
                     const gz::utils::EnvironmentStrings &_environment):
    Subprocess(_commandLine, gz::utils::envStringsToMap(_environment))
  {
  }


  private: void Create()
  {
    if (this->process != nullptr)
        return;

    this->process = new subprocess_s;

    auto environmentStr = gz::utils::envMapToStrings(this->environment);
    std::vector<const char*> environmentCstr;
    std::vector<const char*> commandLineCstr;

    for (const auto &val : this->commandLine)
    {
      commandLineCstr.push_back(val.c_str());
    }
    commandLineCstr.push_back(nullptr);

    if (!this->inheritEnvironment)
    {
      for (const auto &val : environmentStr)
      {
        environmentCstr.push_back(val.c_str());
      }
      environmentCstr.push_back(nullptr);
    }

    int ret = -1;
    if (!this->inheritEnvironment)
    {
      ret = subprocess_create_ex(commandLineCstr.data(),
                                 0, environmentCstr.data(), this->process);
    }
    else
    {
      ret = subprocess_create(commandLineCstr.data(),
                              subprocess_option_inherit_environment,
                              this->process);
    }

    if (0 != ret)
    {
      std::cerr << "failed to create subprocess" << std::endl;
      delete this->process;
      this->process = nullptr;
    }
  }

  public: ~Subprocess()
  {
    if (this->process != nullptr)
      subprocess_destroy(this->process);
    delete this->process;
  }

  public: std::string Stdout()
  {
    std::string result;
    if (this->process != nullptr)
    {
      auto *p_stdout = subprocess_stdout(this->process);
      char buffer[128];
      while (!feof(p_stdout))
      {
        if (fgets(buffer, 128, p_stdout) != nullptr)
          result += buffer;
      }
    }
    return result;
  }

  public: std::string Stderr()
  {
    std::string result;
    if (this->process != nullptr)
    {
      auto *p_stdout = subprocess_stderr(this->process);
      char buffer[128];
      while (!feof(p_stdout))
      {
        if (fgets(buffer, 128, p_stdout) != nullptr)
          result += buffer;
      }
    }
    return result;

  }

  public: bool Alive()
  {
    if (this->process != nullptr)
      return subprocess_alive(this->process);
    else
      return false;
  }

  public: bool Terminate()
  {
    if (this->process != nullptr)
      return subprocess_terminate(this->process) != 0;
    else
      return false;
  }

  public: int Join()
  {
    int return_code = -1;
    if (this->process != nullptr)
    {
      auto ret = subprocess_join(this->process, &return_code);
      if (ret != 0)
      {
        std::cerr << "Failed to join subprocess" << std::endl;
      }
    }

    return return_code;
  }

  protected: std::vector<std::string> commandLine;

  protected: EnvironmentMap environment;

  protected: bool inheritEnvironment;

  protected: subprocess_s * process {nullptr};
};
}  // namespace utils
}  // namespace gz

#endif  // GZ_UTILS__SUBPROCESS_HH_
