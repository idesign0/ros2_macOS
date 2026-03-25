/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <gz/utils/cli/CLI.hpp>
#include <gz/utils/config.hh>

/*
 * At this point, all functionality of CLI11 will be available.
 * Consult https://github.com/CLIUtils/CLI11 for usage
 */

//////////////////////////////////////////////////
int main(int argc, char** argv)
{
  CLI::App app{"Using gz-utils CLI wrapper"};

  app.add_flag_callback("-v,--version", [](){
      std::cout << GZ_UTILS_VERSION_FULL << std::endl;
      throw CLI::Success();
  });

  CLI11_PARSE(app, argc, argv);
}
