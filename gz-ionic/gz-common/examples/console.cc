/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include <filesystem>
#include <gz/common/Console.hh>

int main(int argc, char **argv)
{
  // Default verbosity is level 1 (err).
  gztrace << "This is a trace message" << std::endl;
  gzdbg   << "This is a debug message" << std::endl;
  gzmsg   << "This is an informational message" << std::endl;
  gzwarn  << "This is a warning" << std::endl;
  gzerr   << "This is an error" << std::endl;
  gzcrit  << "This is a critical error" << std::endl;

  // Change verbosity to level 5 (trace), all messages show.
  gz::common::Console::SetVerbosity(5);
  gz::common::Console::SetPrefix("My prefix. ");
  gztrace << "This is a trace message" << std::endl;
  gzdbg   << "This is a debug message" << std::endl;
  gzmsg   << "This is an informational message" << std::endl;
  gzwarn  << "This is a warning" << std::endl;
  gzerr   << "This is an error" << std::endl;
  gzcrit  << "This is a critical error" << std::endl;

  std::filesystem::path logDir = std::filesystem::temp_directory_path();
  std::filesystem::path logFile = "my_log.txt";

  gz::common::Console c("gz_tmp");
  c.SetLogDestination(logDir / "tmp2" / logFile);
  auto logger = c.RawLogger();
  logger.log(spdlog::level::err, "Hello\n");

  gz::common::Console::Init(logDir / "tmp3", logFile);
  gzerr << "Error 1" << std::endl;
  gzerr << "Error 2" << std::endl;
  gzerr << "Directory: " << gz::common::Console::Directory() << std::endl;
  gz::common::Console::Close();
  // We shouldn't see this line in the log.
  gzerr << "Error 3" << std::endl;

  return 0;
}
