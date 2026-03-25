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

#include <iostream>
#include <string>
#include <vector>

#include <gz/utils/cli/CLI.hpp>
#include <gz/utils/cli/GzFormatter.hpp>

#include <gz/msgs/config.hh>
#include <gz/msgs/Factory.hh>
#include <gz/msgs/MessageTypes.hh>

//////////////////////////////////////////////////
/// \brief Enumeration of available commands
enum class MsgCommand
{
  kNone,
  kMsgInfo,
  kMsgList,
};

//////////////////////////////////////////////////
/// \brief Structure to hold all available message options
struct MsgOptions
{
  /// \brief Command to execute
  MsgCommand command {MsgCommand::kNone};

  /// \brief List of messages for message info command
  std::vector<std::string> msgNames;
};

//////////////////////////////////////////////////
void runMsgInfo(const MsgOptions &_opt)
{
  for (const auto &msgName: _opt.msgNames)
  {
    auto msg = gz::msgs::Factory::New(msgName);
    if (msg)
    {
      auto descriptor = msg->GetDescriptor();
      auto fileDescriptor = descriptor->file();
      std::cout << "Name: " << descriptor->full_name() << std::endl;
      std::cout << "File: " << fileDescriptor->name() << std::endl << std::endl;
      std::cout << descriptor->DebugString() << std::endl;
    }
    else
    {
      std::cerr << "Unable to create message of type[" << msgName << "]\n";
    }
  }
}

//////////////////////////////////////////////////
void runMsgList(const MsgOptions &/*_opt*/)
{
  std::vector<std::string> types;
  gz::msgs::Factory::Types(types);

  for (auto const &type : types)
    std::cout << type << std::endl;
}

//////////////////////////////////////////////////
void runMsgCommand(const MsgOptions &_opt)
{
  switch(_opt.command)
  {
    case MsgCommand::kMsgInfo:
      runMsgInfo(_opt);
      break;
    case MsgCommand::kMsgList:
      runMsgList(_opt);
      break;
    case MsgCommand::kNone:
    default:
      // In the event that there is no command, display help
      throw CLI::CallForHelp();
      break;
  }
}

//////////////////////////////////////////////////
void addMsgFlags(CLI::App &_app)
{
  auto opt = std::make_shared<MsgOptions>();

  auto listOpt = _app.add_flag_callback("-l,--list",
     [opt](){
       opt->command = MsgCommand::kMsgList;
     }, "List all message types.");

  auto infoOpt = _app.add_option("-i,--info",
    opt->msgNames, "Get info about the specified message type.")
    ->excludes(listOpt);

  _app.callback([opt, infoOpt](){
    if(infoOpt->count() > 0) {
      opt->command = MsgCommand::kMsgInfo;
    }
    runMsgCommand(*opt);
  });
}

//////////////////////////////////////////////////
int main(int argc, char** argv)
{
  CLI::App app{"Print information about Gazebo messages"};

  app.add_flag_callback("-v,--version", [](){
      std::cout << GZ_MSGS_VERSION_FULL << std::endl;
      throw CLI::Success();
  });

  addMsgFlags(app);
  app.formatter(std::make_shared<GzFormatter>(&app));
  CLI11_PARSE(app, argc, argv);
}
