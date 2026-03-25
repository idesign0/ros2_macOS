/*
 * Copyright 2024 CogniPilot Foundation
 * Copyright 2024 Open Source Robotics Foundation
 * Copyright 2024 Rudis Laboratories
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
#include <gz/utils/cli/GzFormatter.hpp>

#include "gz.hh"

#include <gz/transport/config.hh>

//////////////////////////////////////////////////
/// \brief Enumeration of available commands
enum class TopicCommand
{
  kNone,
  kTopicList,
  kTopicInfo,
  kTopicPub,
  kTopicEcho,
  kTopicFrequency
};

//////////////////////////////////////////////////
/// \brief Structure to hold all available topic options
struct TopicOptions
{
  /// \brief Command to execute
  TopicCommand command{TopicCommand::kNone};

  /// \brief Name of the topic
  std::string topic;

  /// \brief Message type to use when publishing
  std::string msgType;

  /// \brief Message data to use when publishing
  std::string msgData;

  /// \brief Amount of time to echo (in seconds)
  double duration{-1};

  /// \brief Number of messages to echo
  int count{-1};

  /// \brief Message output format
  MsgOutputFormat msgOutputFormat {MsgOutputFormat::kDefault};
};

//////////////////////////////////////////////////
/// \brief Callback fired when options are successfully parsed
void runTopicCommand(const TopicOptions &_opt)
{
  switch(_opt.command)
  {
    case TopicCommand::kTopicList:
      cmdTopicList();
      break;
    case TopicCommand::kTopicInfo:
      cmdTopicInfo(_opt.topic.c_str());
      break;
    case TopicCommand::kTopicPub:
      cmdTopicPub(_opt.topic.c_str(),
                  _opt.msgType.c_str(),
                  _opt.msgData.c_str());
      break;
    case TopicCommand::kTopicEcho:
      cmdTopicEcho(_opt.topic.c_str(), _opt.duration, _opt.count,
                   _opt.msgOutputFormat);
      break;
    case TopicCommand::kTopicFrequency:
      cmdTopicFrequency(_opt.topic.c_str());
      break;
    case TopicCommand::kNone:
    default:
      // In the event that there is no command, display help
      throw CLI::CallForHelp();
  }
}

//////////////////////////////////////////////////
void addTopicFlags(CLI::App &_app)
{
  auto opt = std::make_shared<TopicOptions>();

  auto topicOpt = _app.add_option("-t,--topic",
                                  opt->topic, "Name of a topic.");
  auto msgTypeOpt = _app.add_option("-m,--msgtype",
                                   opt->msgType, "Type of message to publish.");
  auto durationOpt = _app.add_option("-d,--duration",
                                     opt->duration,
                                     "Duration (seconds) to run.");
  auto countOpt = _app.add_option("-n,--num",
                                  opt->count,
                                  "Number of messages to echo and then exit.");

  durationOpt->excludes(countOpt);
  countOpt->excludes(durationOpt);

  auto command = _app.add_option_group("command", "Command to be executed.");

  command->add_flag_callback("-l,--list",
    [opt](){
      opt->command = TopicCommand::kTopicList;
    },
    "List all topics.");

  command->add_flag_callback("-i,--info",
    [opt](){
      opt->command = TopicCommand::kTopicInfo;
    },
R"(Get info about a topic. E.g.:
  gz topic -i -t /foo)")
    ->needs(topicOpt);

  command->add_flag_callback("-e,--echo",
    [opt](){
      opt->command = TopicCommand::kTopicEcho;
    },
R"(Output data to screen. E.g.:
  gz topic -e -t /foo)")
    ->needs(topicOpt);

  command->add_flag_callback("-f,--frequency",
    [opt](){
      opt->command = TopicCommand::kTopicFrequency;
    },
R"(Calculate the frequency of a topic:
  gz topic -f -t /foo)")
    ->needs(topicOpt);

  command->add_flag_callback("--json-output",
      [opt]() { opt->msgOutputFormat = MsgOutputFormat::kJSON; },
      "Output messages in JSON format.");

  command->add_option_function<std::string>("-p,--pub",
      [opt](const std::string &_msgData){
        opt->command = TopicCommand::kTopicPub;
        opt->msgData = _msgData;
      },
R"(Publish a message.
TEXT is the message data. The format expected is
the same used by google::protobuf::TextFormat::PrintToString(). E.g.:
  gz topic -t /foo -m gz.msgs.StringMsg \
    -p 'data:"Custom data"')")
    ->needs(topicOpt)
    ->needs(msgTypeOpt);

  _app.callback([opt](){runTopicCommand(*opt); });
}

//////////////////////////////////////////////////
int main(int argc, char** argv)
{
  CLI::App app{"Introspect Gazebo topics"};

  app.add_flag_callback("-v,--version", [](){
      std::cout << GZ_TRANSPORT_VERSION_FULL << std::endl;
      throw CLI::Success();
  });

  addTopicFlags(app);
  app.formatter(std::make_shared<GzFormatter>(&app));
  CLI11_PARSE(app, argc, argv);
}
