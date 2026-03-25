/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <gz/common/URI.hh>
#include <gz/fuel_tools.hh>

DEFINE_bool(h, false, "Show help");

DEFINE_string(c, "", "Config file");
DEFINE_string(t, "", "Resource type (model / world)");
DEFINE_string(n, "", "Resource name");
DEFINE_string(o, "", "Owner name");
DEFINE_string(s, "", "Server URL");

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Simple usage.
  std::string usage("Download a resource.");
  usage += " Usage:\n  ./download <options>\n\n";
  usage += "  Examples:\n"
    "\t ./download -t model -o OpenRobotics -n Beer\n"
    "\t ./download -s https://fuel.gazebosim.org -t world "
    "-o OpenRobotics"
    " -n Empty\n"
    "\t ./download -c /tmp/my_config.yaml -t model -o caguero -n Beer\n";

  gflags::SetUsageMessage(usage);

  // Parse command line arguments.
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);

  // Show help, if specified.
  if (FLAGS_h || FLAGS_n == "" || (FLAGS_t != "model" && FLAGS_t != "world"))
  {
    gflags::SetCommandLineOptionWithMode("help", "false",
        gflags::SET_FLAGS_DEFAULT);
    gflags::SetCommandLineOptionWithMode("helpshort", "true",
        gflags::SET_FLAGS_DEFAULT);
  }
  gflags::HandleCommandLineHelpFlags();

  // Setup ClientConfig.
  gz::fuel_tools::ClientConfig conf;
  conf.SetUserAgent("ExampleDownload");

  if (FLAGS_c != "")
  {
    if (!conf.LoadConfig(FLAGS_c))
    {
      std::cerr << "Error loading configuration file [" << FLAGS_c << "]"
                << std::endl;
      return -1;
    }
  }

  if (FLAGS_s != "")
  {
    // The user specified a Fuel server via command line.
    gz::fuel_tools::ServerConfig srv;
    srv.SetUrl(gz::common::URI(FLAGS_s));

    // Add the extra Fuel server.
    conf.AddServer(srv);
  }

  // Instantiate the FuelClient object with the configuration.
  gz::fuel_tools::FuelClient client(conf);

  // Set the properties of the resource that we want to download.
  gz::fuel_tools::ModelIdentifier modelIdentifier;
  gz::fuel_tools::WorldIdentifier worldIdentifier;

  if (FLAGS_t == "model")
  {
    modelIdentifier.SetOwner(FLAGS_o);
    modelIdentifier.SetName(FLAGS_n);
  }
  else if (FLAGS_t == "world")
  {
    worldIdentifier.SetOwner(FLAGS_o);
    worldIdentifier.SetName(FLAGS_n);
  }

  // Fetch the resource.
  for (const auto &server : client.Config().Servers())
  {
    if (FLAGS_t == "model")
    {
      // Set server
      auto id = modelIdentifier;
      id.SetServer(server);

      // Download and return 0 only if success
      if (client.DownloadModel(id).Type() == gz::fuel_tools::ResultType::FETCH)
      {
        return 0;
      }
    }
    else if (FLAGS_t == "world")
    {
      // Set server
      auto id = worldIdentifier;
      id.SetServer(server);

      // Download and return 0 only if success
      if (client.DownloadWorld(id).Type() == gz::fuel_tools::ResultType::FETCH)
      {
        return 0;
      }
    }
  }

  std::cerr << "Unable to download resource" << std::endl;
  return -1;
}
