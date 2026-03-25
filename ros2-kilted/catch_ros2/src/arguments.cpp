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

#include <sstream>
#include "catch_ros2/arguments.hpp"
#include "rclcpp/rclcpp.hpp"

namespace catch_ros2
{
SimulateArgs::SimulateArgs(const std::string & args, bool omit_executable_path)
: SimulateArgs(tokenize(args), omit_executable_path)
{
}

SimulateArgs::SimulateArgs(const std::vector<std::string> args, bool omit_executable_path)
: args_{args}
{
  if (!omit_executable_path) {
    args_.insert(args_.begin(), "/path/to/executable");
  }

  generate_argv_vec_();
}

int SimulateArgs::argc() const {return argv_vec_.size() - 1;}

const char * const * SimulateArgs::argv() const
{
  if (!(argc() == 0)) {
    return argv_vec_.data();
  } else {
    return nullptr;
  }
}

void SimulateArgs::generate_argv_vec_()
{
  // Adapted from
  // https://stackoverflow.com/questions/39883433/create-argc-argv-in-the-code

  // Create char* vector to hold pointers to each element of the argument vector
  for (const auto & arg : args_) {
    argv_vec_.push_back(const_cast<char *>(arg.data()));
  }
  argv_vec_.push_back(nullptr);
}


SplitROSArgs::SplitROSArgs(const int argc, const char * const * argv)
: argc_{argc},
  argv_{argv},
  args_without_ros_{SimulateArgs{rclcpp::remove_ros_arguments(argc_, argv_), true}}
{}

int SplitROSArgs::argc() const {return argc_;}

int SplitROSArgs::argc_without_ros() const {return args_without_ros_.argc();}

const char * const * SplitROSArgs::argv() const {return argv_;}

const char * const * SplitROSArgs::argv_without_ros() const {return args_without_ros_.argv();}


std::vector<std::string> tokenize(const std::string & input_string)
{
  // Adapted from
  // https://www.physicsforums.com/threads/c-function-to-split-a-string-by-whitespace-ignoring-any-whitespace-in-quotes.778920/

  std::vector<std::string> result;
  bool in_quotes = false;
  std::string curr_word;

  // Iterate over string and collect words
  for (const auto & c : input_string) {
    // Toggle whether we're inside quotes or not
    if (c == '"') {
      in_quotes = !in_quotes;

      // Non-separator character: add it to the current word
    } else if (in_quotes || (c != ' ')) {
      curr_word.push_back(c);

      // First unquoted space after a word: add word to result and reset word.
    } else if (!curr_word.empty()) {
      result.push_back(curr_word);
      curr_word.clear();
    }
    // No else: this represents consecutive unquotes spaces
  }
  // Add the last word to the result
  if (!curr_word.empty()) {
    result.push_back(curr_word);
  }

  return result;
}

}  // namespace catch_ros2
