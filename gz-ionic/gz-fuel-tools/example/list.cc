/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <gflags/gflags.h>

#include <iostream>

#include <gz/fuel_tools.hh>

DEFINE_bool(h, false, "Show help");

DEFINE_string(c, "", "Config file");
DEFINE_string(s, "", "Server URL");
DEFINE_string(t, "", "Resource type (model / world)");

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Simple usage.
  std::string usage("List all models.");
  usage += " Usage:\n  ./list <options>\n\n";
  usage += "  Example:\n"
           "\t ./list -t world\n"
           "\t ./list -s https://localhost:4430 -t model"
           "\t ./list -c /tmp/my_config.yaml -t world\n";

  gflags::SetUsageMessage(usage);

  // Parse command line arguments.
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);

  // Show help, if specified.
  if (FLAGS_h || (FLAGS_t != "model" && FLAGS_t != "world"))
  {
    gflags::SetCommandLineOptionWithMode("help", "false",
        gflags::SET_FLAGS_DEFAULT);
    gflags::SetCommandLineOptionWithMode("helpshort", "true",
        gflags::SET_FLAGS_DEFAULT);
  }
  gflags::HandleCommandLineHelpFlags();

  if (FLAGS_c != "" && FLAGS_s != "")
  {
    std::cerr << "Please, do not use -c <config_file> and -s <server> together."
              << std::endl;
    return -1;
  }

  // Setup ClientConfig.
  gz::fuel_tools::ClientConfig conf;
  conf.SetUserAgent("ExampleList");

  if (FLAGS_s != "")
  {
    // The user specified a Fuel server via command line.
    gz::fuel_tools::ServerConfig srv;
    srv.SetUrl(gz::common::URI(FLAGS_s));

    // Add the extra Fuel server.
    conf.AddServer(srv);
  }
  else
  {
    if (FLAGS_c != "")
    {
      if (!conf.LoadConfig(FLAGS_c))
      {
        std::cerr << "Error loading configuration file [" << FLAGS_c << "]"
                  << std::endl;
        return -1;
      }
    }
  }

  // Instantiate the FuelClient object with the configuration.
  gz::fuel_tools::FuelClient client(conf);

  for (const auto &server : client.Config().Servers())
  {
    std::cout << "[" << server.Url().Str() << "]\n\n";
    if (FLAGS_t == "model")
    {
      for (auto iter = client.Models(server); iter; ++iter)
        std::cout << "  " << iter->Identification().Name() << "\n";
    }
    else if (FLAGS_t == "world")
    {
      for (auto iter = client.Worlds(server); iter; ++iter)
        std::cout << "  " << iter->Name() << "\n";
    }
    std::cout << std::endl;
  }
}

