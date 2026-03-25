// Copyright 2023 Nick Morales.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// @file
/// @brief Utilities for handling command line arguments

#ifndef CATCH_ROS2__ARGUMENTS_HPP_
#define CATCH_ROS2__ARGUMENTS_HPP_

#include <vector>
#include <string>

namespace catch_ros2
{
/// @brief A class for generating simulated argc/argv values from an input string
class SimulateArgs
{
public:
  /// @brief Parse string into data structures that can be used to generate simulated argc/argvs
  /// @param args - command string to parse into arguments
  /// @param omit_executable_path - since the first argument is always a path to the
  /// executable, by default this class prepends a default (fake) path to the provided
  /// argument list. To omit this default path (for example, if you'd like to provide)
  /// your own in the input args), set this boolean argument to true.
  explicit SimulateArgs(const std::string & args, bool omit_executable_path = false);

  /// @brief Store vector of arguments for use simulating arguments
  /// @param args - vector of arguments
  /// @param omit_executable_path - since the first argument is always a path to the
  /// executable, by default this class prepends a default (fake) path to the provided
  /// argument list. To omit this default path (for example, if you'd like to provide)
  /// your own in the input args), set this boolean argument to true.
  explicit SimulateArgs(const std::vector<std::string> args, bool omit_executable_path = false);

  /// @brief generate argc
  /// @return argc, argument count
  int argc() const;

  /// @brief generate argv
  /// @return pointer to first argument data
  const char * const * argv() const;

private:
  /// @brief string argument data
  std::vector<std::string> args_ {};

  /// @brief pointers to string argument data
  std::vector<char *> argv_vec_ {};

  /// @brief generate argv_vec_. For use in constructors.
  void generate_argv_vec_();
};


/// @brief a class for splitting ROS arguments out of input arguments
class SplitROSArgs
{
public:
  /// @brief find where ROS Args start
  /// @param argc - argument count
  /// @param argv - pointer to first argument data
  SplitROSArgs(const int argc, const char * const * argv);

  /// @brief return original argc
  /// @return original argc
  int argc() const;

  /// @brief return argc if ROS args are taken out
  /// @return argc if ROS args are taken out
  int argc_without_ros() const;

  /// @brief return original argv
  /// @return original argv
  const char * const * argv() const;

  /// @brief return argv if ROS args are taken out
  /// @return argv if ROS args are taken out
  const char * const * argv_without_ros() const;

private:
  /// @brief original argc
  const int argc_;

  /// @brief original argv
  const char * const * argv_;

  /// @brief SimulateArgs object that contains the arguments without ROS
  SimulateArgs args_without_ros_;
};


/// @brief split a string by whitespace, ignoring whitespace in quotes
/// @param input_string - string to split
/// @return vector of split strings
std::vector<std::string> tokenize(const std::string & input_string);
}  // namespace catch_ros2

#endif  // CATCH_ROS2__ARGUMENTS_HPP_
