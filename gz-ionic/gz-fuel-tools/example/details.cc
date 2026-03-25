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

#include <gz/fuel_tools.hh>

DEFINE_bool(h, false, "Show help");

DEFINE_string(c, "", "Config file");
DEFINE_string(n, "", "Resource name");
DEFINE_string(o, "", "Owner name");
DEFINE_string(s, "", "Server name");
DEFINE_string(t, "", "Resource type (model / world)");

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Simple usage.
  std::string usage("Show details of a resource.");
  usage += " Usage:\n  ./details <options>\n\n";
  usage += "  Example:\n"
    "\t ./details -t model -o OpenRobotics -n Beer"
    "\t ./details -t world -s https://localhost:4430 -o OpenRobotics -n Empty"
    "\t ./details -c /tmp/my_config.yaml -o OpenRobotics -n Beer -t model";

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
  conf.SetUserAgent("ExampleDetails");

  if (FLAGS_s != "")
  {
    // The user specified a Fuel server via command line.
    gz::fuel_tools::ServerConfig srv;
    srv.SetUrl(gz::common::URI(FLAGS_s));

    // Add the extra Fuel server.
    conf.AddServer(srv);
  }

  if (FLAGS_c != "")
  {
    if (!conf.LoadConfig(FLAGS_c))
    {
      std::cerr << "Error loading configuration file [" << FLAGS_c << "]"
                << std::endl;
      return -1;
    }
  }

  // Instantiate the FuelClient object with the configuration.
  gz::fuel_tools::FuelClient client(conf);

  // Set the properties of the resource that we want to search.
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

  // Fetch the details.
  for (const auto &server : client.Config().Servers())
  {
    if (FLAGS_t == "model")
    {
      // Set server
      auto id = modelIdentifier;
      id.SetServer(server);

      gz::fuel_tools::ModelIdentifier model;
      if (!client.ModelDetails(id, model))
        continue;

      std::cout << model.AsPrettyString() << std::endl;
    }
    else if (FLAGS_t == "world")
    {
      // Set server
      auto id = worldIdentifier;
      id.SetServer(server);

      gz::fuel_tools::WorldIdentifier world;
      if (!client.WorldDetails(id, world))
        continue;

      std::cout << world.AsPrettyString() << std::endl;
    }
  }

  return 0;
}
