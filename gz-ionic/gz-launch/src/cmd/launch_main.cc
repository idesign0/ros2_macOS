
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

#include <gz/common/Console.hh>
#include <gz/common/Util.hh>

#include <gz/utils/cli/CLI.hpp>

#include "gz/launch/config.hh"
#include "gz.hh"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

/////////////////////////////////////////////////
std::string erbExec(std::string _cmd)
{
  _cmd += " 2>&1";
  FILE *pipe = popen(_cmd.c_str(), "r");

  if (!pipe)
    return "ERROR";

  char buffer[128];
  std::string result = "";

  while (!feof(pipe))
  {
    if (fgets(buffer, 128, pipe) != nullptr)
      result += buffer;
  }

  pclose(pipe);
  return result;
}

//////////////////////////////////////////////////
/// \brief Enumeration of available commands
enum class LaunchCommand
{
  kNone,
};

//////////////////////////////////////////////////
/// \brief Structure to hold all available topic options
struct LaunchOptions
{
  /// \brief Command to execute
  LaunchCommand command{LaunchCommand::kNone};

  std::string launchfile;

  std::vector<std::string> more_comms;

  int verboseLevel = 1;
};

void runLaunchCommand(const LaunchOptions &_opt)
{
  cmdVerbosity(_opt.verboseLevel);

  std::string launchfile = _opt.launchfile;
  if (!launchfile.empty())
  {
    // If the launch file is not a file, then look in the paths set by the
    // GZ_LAUNCH_CONFIG_PATH environment variable.
    if (!gz::common::isFile(launchfile))
    {
      std::string configPathEnv;
      gz::common::env("GZ_LAUNCH_CONFIG_PATH", configPathEnv, true);

      if (!configPathEnv.empty())
      {
        std::vector<std::string> paths =
          gz::common::split(configPathEnv, ":");
        for (const std::string &path : paths)
        {
          std::string filePath =
            gz::common::joinPaths(path, launchfile);
          if (gz::common::isFile(filePath))
          {
            launchfile = filePath;
            break;
          }
        }
      }

      if (!gz::common::isFile(launchfile))
      {
        gzerr << "File [" + launchfile + "] does not exists"
          << std::endl;
        exit(-1);
      }
    }

    std::string cmd = "erb ";
    for (auto & arg : _opt.more_comms)
    {
      auto tokens = gz::common::split(arg, ":=");
      if (tokens.size() == 2)
      {
        cmd += " " + tokens[0] + "=" + tokens[1];
      }
      else
      {
        gzerr << "This argument [" << arg << "] is wrong for erb"
               << std::endl;
        exit(1);
      }
    }
    cmd += " " + launchfile;
    std::string config = erbExec(cmd);
    run(config.c_str());
    return;
  }

  if (_opt.command == LaunchCommand::kNone)
  {
    // In the event that there is no command, display help
    throw CLI::CallForHelp();
  }
}

void addLaunchFlags(CLI::App &_app)
{
  auto opt = std::make_shared<LaunchOptions>();

  _app.add_option("-v,--verbose",
                  opt->verboseLevel,
                  "Verbose level");
  _app.add_option("launch", opt->launchfile, "launch_file")->take_last();
  _app.allow_extras();
  _app.callback([&_app, opt](){
    opt->more_comms = _app.remaining();
    runLaunchCommand(*opt);
  });
}

//////////////////////////////////////////////////
int main(int argc, char** argv)
{
  CLI::App app{"Introspect Gazebo launch"};

  app.set_help_all_flag("--help-all", "Show all help");

  app.add_flag_callback("--version", [](){
      std::cout << gzVersion() << std::endl;
      throw CLI::Success();
  });

  addLaunchFlags(app);
  CLI11_PARSE(app, argc, argv);
}
