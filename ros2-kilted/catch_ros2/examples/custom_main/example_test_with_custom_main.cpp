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

/// @file - Example test with a custom main. See main function at bottom.

#include <vector>
#include <string>
#include "catch_ros2/catch_ros2.hpp"

using catch_ros2::tokenize;
using catch_ros2::SimulateArgs;
using Catch::Matchers::Equals;

TEST_CASE("tokenize", "[tokenize]")
{
  SECTION("no quotes")
  {
    const auto result = tokenize("tokenize me please!");
    CHECK_THAT(result.at(0), Equals("tokenize"));
    CHECK_THAT(result.at(1), Equals("me"));
    CHECK_THAT(result.at(2), Equals("please!"));
  }
  SECTION("quotes")
  {
    const auto result = tokenize("here is a string \"with some\" quotations \"in it\"");
    CHECK_THAT(result.at(0), Equals("here"));
    CHECK_THAT(result.at(1), Equals("is"));
    CHECK_THAT(result.at(2), Equals("a"));
    CHECK_THAT(result.at(3), Equals("string"));
    CHECK_THAT(result.at(4), Equals("with some"));
    CHECK_THAT(result.at(5), Equals("quotations"));
    CHECK_THAT(result.at(6), Equals("in it"));
  }
}

TEST_CASE("SimulateArgs", "[SimulateArgs]")
{
  // Examples of different constructors of SimulateArgs
  SECTION("string, default executable path")
  {
    SimulateArgs args {"-c some -o args.o"};
    const auto arg_count = args.argc() == 5;  // temporary bool to pass style checks
    CHECK(arg_count);
    CHECK_THAT(*(args.argv() + 0), Equals("/path/to/executable"));
    CHECK_THAT(*(args.argv() + 1), Equals("-c"));
    CHECK_THAT(*(args.argv() + 2), Equals("some"));
    CHECK_THAT(*(args.argv() + 3), Equals("-o"));
    CHECK_THAT(*(args.argv() + 4), Equals("args.o"));
  }
  SECTION("vector, default executable path")
  {
    const std::vector<std::string> arg_vec {"argument with space", "Klaatu barada nikto"};
    SimulateArgs args {arg_vec};
    const auto arg_count = args.argc() == 3;  // temporary bool to pass style checks
    CHECK(arg_count);
    CHECK_THAT(*(args.argv() + 0), Equals("/path/to/executable"));
    CHECK_THAT(*(args.argv() + 1), Equals("argument with space"));
    CHECK_THAT(*(args.argv() + 2), Equals("Klaatu barada nikto"));
  }
  SECTION("string, omit executable path")
  {
    // Second boolean argument omits the default executable path
    // from the simulated arguments
    SimulateArgs args {"/millenium/falcon --kessel-run-parsecs 12", true};
    const auto arg_count = args.argc() == 3;  // temporary bool to pass style checks
    CHECK(arg_count);
    CHECK_THAT(*(args.argv() + 0), Equals("/millenium/falcon"));
    CHECK_THAT(*(args.argv() + 1), Equals("--kessel-run-parsecs"));
    CHECK_THAT(*(args.argv() + 2), Equals("12"));
  }
  SECTION("vector, omit executable path")
  {
    // Second boolean argument omits the default executable path
    // from the simulated arguments
    SimulateArgs args {
      std::vector<std::string>({"/bin/reach", "--current-objective", "survive"}),
      true
    };
    const auto arg_count = args.argc() == 3;  // temporary bool to pass style checks
    CHECK(arg_count);
    CHECK_THAT(*(args.argv() + 0), Equals("/bin/reach"));
    CHECK_THAT(*(args.argv() + 1), Equals("--current-objective"));
    CHECK_THAT(*(args.argv() + 2), Equals("survive"));
  }

  // An empty argument list should have argc == 0 and argv is NULL
  SECTION("string, empty arg list")
  {
    SimulateArgs args {"", true};
    const auto arg_count = args.argc() == 0;  // temporary bool to pass style checks
    CHECK(arg_count);
    CHECK_FALSE(args.argv());
  }
  SECTION("vector, empty arg list")
  {
    SimulateArgs args {std::vector<std::string>(), true};
    const auto arg_count = args.argc() == 0;  // temporary bool to pass style checks
    CHECK(arg_count);
    CHECK_FALSE(args.argv());
  }
}

// Example main can be specified like so. For more information see
// https://github.com/catchorg/Catch2/blob/devel/docs/own-main.md
int main(int argc, char * argv[])
{
  // your setup ...

  int result = Catch::Session().run(argc, argv);

  // your clean-up...

  return result;
}
