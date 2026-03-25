/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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
#include <gz/msgs/int32.pb.h>
#include <gz/msgs/stringmsg.pb.h>
#include <gz/msgs/vector3d.pb.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <gz/utils/Environment.hh>
#include <gz/utils/ExtraTestMacros.hh>
#include <gz/utils/Subprocess.hh>

#include "gtest/gtest.h"
#include "gz/transport/Node.hh"

#include "test_config.hh"
#include "test_utils.hh"

using namespace gz;

static std::string g_partition; // NOLINT(*)
static std::string g_topicCBStr; // NOLINT(*)

//////////////////////////////////////////////////
/// \brief Provide a service.
bool srvEcho(const msgs::Int32 &_req, msgs::Int32 &_rep)
{
  _rep.set_data(_req.data());
  return true;
}

//////////////////////////////////////////////////
/// \brief Provide a one-way service.
void srvOneway(const msgs::StringMsg &_msg)
{
  g_topicCBStr = _msg.data();
}

//////////////////////////////////////////////////
/// \brief Provide a service without input.
bool srvNoInput(msgs::StringMsg &_msg)
{
  _msg.set_data("good_value");
  return true;
}

//////////////////////////////////////////////////
/// \brief Topic callback
void topicCB(const msgs::StringMsg &_msg)
{
  g_topicCBStr = _msg.data();
}

//////////////////////////////////////////////////
/// \brief A generic callback.
void genericCb(const transport::ProtoMsg &/*_msg*/)
{
}

//////////////////////////////////////////////////
/// \brief A raw callback.
void cbRaw(const char * /*_msgData*/, const size_t /*_size*/,
           const transport::MessageInfo &/*_info*/)
{
}

//////////////////////////////////////////////////
struct ProcessOutput
{
  int code {-1};
  std::string cout;
  std::string cerr;
};

//////////////////////////////////////////////////
ProcessOutput custom_exec_str(const std::vector<std::string> &_args)
{
  auto fullArgs = std::vector<std::string>{test_executables::kGzExe};
  std::copy(std::begin(_args), std::end(_args), std::back_inserter(fullArgs));
  fullArgs.emplace_back("--force-version");
  fullArgs.emplace_back(kGzVersion);
  auto proc = gz::utils::Subprocess(fullArgs);
  auto return_code = proc.Join();
  return {return_code, proc.Stdout(), proc.Stderr()};
}

//////////////////////////////////////////////////
std::optional<ProcessOutput>
exec_with_retry(const std::vector<std::string> &_args,
                const std::function<bool(ProcessOutput)> &_condition)
{
  bool success = false;
  int retries = 0;

  while (!success && retries++ < 10)
  {
    auto output = custom_exec_str(_args);
    success = _condition(output);
    if (success)
      return output;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
  }
  return {};
}

//////////////////////////////////////////////////
/// \brief Check 'gz topic -l' running the advertiser on a different process.
TEST(gzTest, GZ_UTILS_TEST_DISABLED_ON_MAC(TopicList))
{
  auto proc = gz::utils::Subprocess({
    test_executables::kTwoProcsPublisher, g_partition});

  auto output = exec_with_retry({"topic", "-l"},
    [](auto procOut){
      return procOut.cout == "/foo\n";
    });

  EXPECT_TRUE(output);
}

//////////////////////////////////////////////////
/// \brief Check 'gz topic -l' running a subscriber on a different process.
TEST(gzTest, TopicListSub)
{
  transport::Node node;
  node.Subscribe("/foo", topicCB);
  node.Subscribe("/bar", genericCb);
  node.SubscribeRaw("/baz", cbRaw,
      std::string(msgs::StringMsg().GetTypeName()));
  node.Subscribe("/no", topicCB);
  node.Unsubscribe("/no");

  auto output = exec_with_retry({"topic", "-l"},
    [](auto procOut){
      return procOut.cout.find("/foo\n") != std::string::npos &&
             procOut.cout.find("/bar\n") != std::string::npos &&
             procOut.cout.find("/baz\n") != std::string::npos &&
             procOut.cout.find("/no\n") == std::string::npos;
    });

  EXPECT_TRUE(output);
}

//////////////////////////////////////////////////
/// \brief Check 'gz topic -i' running the advertiser on a different process.
TEST(gzTest, TopicInfo)
{
  // Launch a new publisher process that advertises a topic.
  auto proc = gz::utils::Subprocess({
    test_executables::kTwoProcsPublisher, g_partition});

  auto output = exec_with_retry({"topic", "-t", "/foo", "-i"},
    [](auto procOut){
      return procOut.cout.size() > 50u;
    });


  ASSERT_TRUE(output) << "OUTPUT["
    << output->cout << "] Size[" << output->cout.size()
    << "]. Expected Size=50" << std::endl;
  EXPECT_TRUE(output->cout.find("gz.msgs.Vector3d") != std::string::npos);
}

//////////////////////////////////////////////////
/// \brief Check 'gz service -l' running the advertiser on a different
/// process.
TEST(gzTest, ServiceList)
{
  // Launch a new responser process that advertises a service.
  auto proc = gz::utils::Subprocess({
    test_executables::kTwoProcsSrvCallReplier, g_partition});

  auto output = exec_with_retry({"service", "-l"},
    [](auto procOut){
      return procOut.cout == "/foo\n";
    });

  EXPECT_TRUE(output);
}

//////////////////////////////////////////////////
/// \brief Check 'gz service -i' running the advertiser on a different process.
TEST(gzTest, ServiceInfo)
{
  // Launch a new responser process that advertises a service.
  auto proc = gz::utils::Subprocess(
    {test_executables::kTwoProcsSrvCallReplier, g_partition});

  auto output = exec_with_retry({"service", "-s", "/foo", "-i"},
    [](auto procOut){
      return procOut.cout.size() > 50u;
    });

  ASSERT_TRUE(output);
  EXPECT_TRUE(output->cout.find("gz.msgs.Int32") != std::string::npos);
}

//////////////////////////////////////////////////
/// \brief Check 'gz topic -l' running the advertiser on the same process.
TEST(gzTest, TopicListSameProc)
{
  transport::Node node;

  msgs::Vector3d msg;
  msg.set_x(1.0);
  msg.set_y(2.0);
  msg.set_z(3.0);

  auto pub = node.Advertise<msgs::Vector3d>("/foo");
  EXPECT_TRUE(pub);
  EXPECT_TRUE(pub.Publish(msg));

  auto output = exec_with_retry({"topic", "-l"},
    [](auto procOut){
      return procOut.cout == "/foo\n";
    });

  EXPECT_TRUE(output);
}

//////////////////////////////////////////////////
/// \brief Check 'gz topic -i' running the advertiser on the same process.
TEST(gzTest, TopicInfoSameProc)
{
  transport::Node node;

  msgs::Vector3d msg;
  msg.set_x(1.0);
  msg.set_y(2.0);
  msg.set_z(3.0);

  auto pub = node.Advertise<msgs::Vector3d>("/foo");
  EXPECT_TRUE(pub);
  EXPECT_TRUE(pub.Publish(msg));

  auto output = exec_with_retry({"topic", "-t", "/foo", "-i"},
    [](auto procOut){
      return procOut.cout.size() > 50u;
    });

  ASSERT_TRUE(output);
  EXPECT_TRUE(output->cout.find("gz.msgs.Vector3d") != std::string::npos);
}

//////////////////////////////////////////////////
/// \brief Check 'gz service -l' running the advertiser on the same process.
TEST(gzTest, ServiceListSameProc)
{
  transport::Node node;
  EXPECT_TRUE(node.Advertise("/foo", srvEcho));

  auto output = exec_with_retry({"service", "-l"},
    [](auto procOut){
      return procOut.cout == "/foo\n";
    });

  EXPECT_TRUE(output);
}

//////////////////////////////////////////////////
/// \brief Check 'gz service -i' running the advertiser on the same process.
TEST(gzTest, ServiceInfoSameProc)
{
  transport::Node node;
  EXPECT_TRUE(node.Advertise("/foo", srvEcho));

  auto output = exec_with_retry({"service", "-s", "/foo", "-i"},
    [](auto procOut){
      return procOut.cout.size() > 50u;
    });

  ASSERT_TRUE(output);
  EXPECT_TRUE(output->cout.find("gz.msgs.Int32") != std::string::npos);
}

//////////////////////////////////////////////////
/// \brief Check 'gz topic -p' to send a message.
TEST(gzTest, TopicPublish)
{
  transport::Node node;
  g_topicCBStr = "bad_value";
  EXPECT_TRUE(node.Subscribe("/bar", topicCB));

  unsigned int retries = 0;
  while (retries++ < 100u)
  {
    auto output = custom_exec_str({"topic",
        "-t", "/bar",
        "-m", "gz.msgs.StringMsg",
        "-p", "data: \"good_value\""});

    EXPECT_TRUE(output.cout.empty()) << output.cout;
    EXPECT_TRUE(output.cerr.empty()) << output.cerr;
    if (g_topicCBStr == "good_value")
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
  }
  EXPECT_EQ(g_topicCBStr, "good_value");

  // Try to publish a message not included in Gazebo Messages.
  std::string error = "Unable to create message of type";
  auto output = custom_exec_str({"topic",
    "-t", "/bar",
    "-m", "gz.msgs.__bad_msg_type",
    "-p", R"(data: "good_value")"});
  EXPECT_EQ(output.cerr.compare(0, error.size(), error), 0)
    << "error {" << error << "}, output.cerr {" << output.cerr << "}";

  // Try to publish using an incorrect topic name.
  error = "Topic [/] is not valid";
  output = custom_exec_str({"topic",
      "-t", "/",
      "-m", "gz.msgs.StringMsg",
      "-p", R"(data: "good_value")"});
  EXPECT_EQ(output.cerr.compare(0, error.size(), error), 0)
    << "error {" << error << "}, output.cerr {" << output.cerr << "}";

  // Try to publish using an incorrect number of arguments.
  error = "The following argument was not expected: wrong_topic";
  output = custom_exec_str({"topic",
      "-t", "/", "wrong_topic",
      "-m", "gz.msgs.StringMsg",
      "-p", R"(data: "good_value")"});
  EXPECT_EQ(output.cerr.compare(0, error.size(), error), 0)
    << "error {" << error << "}, output.cerr {" << output.cerr << "}";
}

//////////////////////////////////////////////////
/// \brief Check 'gz service -r' to request a two-way service.
TEST(gzTest, ServiceRequest)
{
  transport::Node node;

  // Advertise a service.
  std::string service = "/echo";
  std::string value = "10";
  EXPECT_TRUE(node.Advertise(service, srvEcho));

  msgs::Int32 msg;
  msg.set_data(10);

  // Check the 'gz service -r' command.
  auto output = custom_exec_str({"service",
    "-s", service,
    "--reqtype", "gz_msgs.Int32",
    "--reptype", "gz_msgs.Int32",
    "--timeout",  "1000",
    "--req", "data: " + value});
  ASSERT_EQ(output.cout, "data: " + value + "\n\n");
}

//////////////////////////////////////////////////
/// \brief Check 'gz service -r' to request a two-way service without timeout.
TEST(gzTest, ServiceRequestNoTimeout)
{
  transport::Node node;

  // Advertise a service.
  std::string service = "/echo";
  std::string value = "10";
  EXPECT_TRUE(node.Advertise(service, srvEcho));

  msgs::Int32 msg;
  msg.set_data(10);

  // Check the 'gz service -r' command.
  auto output = custom_exec_str({"service",
    "-s", service,
    "--reqtype", "gz_msgs.Int32",
    "--reptype", "gz_msgs.Int32",
    "--req", "data: " + value});
  ASSERT_EQ(output.cout, "data: " + value + "\n\n");
}

//////////////////////////////////////////////////
/// \brief Check 'gz service -r' to request a one-way service.
TEST(gzTest, ServiceOnewayRequest)
{
  g_topicCBStr = "bad_value";
  transport::Node node;

  // Advertise a service.
  std::string service = "/oneway";
  EXPECT_TRUE(node.Advertise(service, srvOneway));

  msgs::StringMsg msg;
  msg.set_data("good_value");

  // Check the 'gz service' oneway command.
  auto output = custom_exec_str(
    {"service", "-s", service, "--reqtype", "gz.msgs.StringMsg",
     "--req", "data: \"good_value\""});

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  EXPECT_EQ("good_value", g_topicCBStr);
}

//////////////////////////////////////////////////
/// \brief Check 'gz service -r' to request a service without input args.
TEST(gzTest, ServiceRequestNoInput)
{
  // Advertise a service.
  transport::Node node;
  std::string service = "/no_input";
  EXPECT_TRUE(node.Advertise(service, srvNoInput));

  // Check the 'gz service -r' no input command.
  auto output = custom_exec_str(
    {"service", "-s", service, "--reptype", "gz.msgs.StringMsg", "--req"});

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  EXPECT_EQ("data: \"good_value\"\n\n", output.cout);
}

//////////////////////////////////////////////////
/// \brief Check 'gz topic -e' running the publisher on a separate process.
TEST(gzTest, TopicEcho)
{
  // Launch a new publisher process that advertises a topic.
  auto proc = gz::utils::Subprocess(
    {test_executables::kTwoProcsPublisher, g_partition});

  auto output = custom_exec_str(
    {"topic", "-e", "-t", "/foo", "-d", "1.5"});

  EXPECT_TRUE(output.cout.find("x: 1") != std::string::npos);
  EXPECT_TRUE(output.cout.find("y: 2") != std::string::npos);
  EXPECT_TRUE(output.cout.find("z: 3") != std::string::npos);
}

//////////////////////////////////////////////////
/// \brief Check 'gz topic -e -n 2' running the publisher on a separate
/// process.
TEST(gzTest, TopicEchoNum)
{
  // Launch a new publisher process that advertises a topic.
  auto proc = gz::utils::Subprocess(
    {test_executables::kTwoProcsPublisher, g_partition});

  auto output = custom_exec_str(
    {"topic", "-e", "-t", "/foo", "-n", "2"});

  size_t pos = output.cout.find("x: 1");
  EXPECT_TRUE(pos != std::string::npos);
  pos = output.cout.find("x: 1", pos + 4);
  EXPECT_TRUE(pos != std::string::npos);
  pos = output.cout.find("x: 1", pos + 4);
  EXPECT_TRUE(pos == std::string::npos);

  pos = output.cout.find("y: 2");
  EXPECT_TRUE(pos != std::string::npos);
  pos = output.cout.find("y: 2", pos + 4);
  EXPECT_TRUE(pos != std::string::npos);
  pos = output.cout.find("y: 2", pos + 4);
  EXPECT_TRUE(pos == std::string::npos);

  pos = output.cout.find("z: 3");
  EXPECT_TRUE(pos != std::string::npos);
  pos = output.cout.find("z: 3", pos + 4);
  EXPECT_TRUE(pos != std::string::npos);
  pos = output.cout.find("z: 3", pos + 4);
  EXPECT_TRUE(pos == std::string::npos);
}

//////////////////////////////////////////////////
/// \brief Check 'gz service --help' message and bash completion script for
/// consistent flags
TEST(gzTest, GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(ServiceHelpVsCompletionFlags))
{
  // Flags in help message
  auto helpOutput = custom_exec_str({"service",  "--help"});

  // Equivalent to:
  // sh -c "bash -c \". /path/to/transport.bash_completion.sh;
  // _gz_service_flags\""
  auto proc = gz::utils::Subprocess(
    {"/usr/bin/bash", "-c",
     "\"\". " + std::string(kTransportBashCompletion) +
      "; _gz_service_flags\"\""});
  auto return_code = proc.Join();
  ASSERT_EQ(0u, return_code) << proc.Stderr();

  // Tokenize script output
  std::istringstream iss(proc.Stdout());
  std::vector<std::string> flags(
    (std::istream_iterator<std::string>(iss)),
    std::istream_iterator<std::string>());

  EXPECT_GT(flags.size(), 0u);

  // Match each flag in script output with help message
  for (const auto &flag : flags)
  {
    EXPECT_NE(std::string::npos, helpOutput.cout.find(flag)) << helpOutput.cout;
  }
}

//////////////////////////////////////////////////
/// \brief Check 'gz topic --help' message and bash completion script for
/// consistent flags
TEST(gzTest, GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(TopicHelpVsCompletionFlags))
{
  // Flags in help message
  auto helpOutput = custom_exec_str({"topic",  "--help"});

  // Equivalent to:
  // sh -c "bash -c \". /path/to/transport.bash_completion.sh;
  // _gz_topic_flags\""
  auto proc = gz::utils::Subprocess(
    {"/usr/bin/bash", "-c",
     "\"\". " + std::string(kTransportBashCompletion) +
      "; _gz_topic_flags\"\""});
  auto return_code = proc.Join();
  ASSERT_EQ(0u, return_code) << proc.Stderr();

  // Tokenize script output
  std::istringstream iss(proc.Stdout());
  std::vector<std::string> flags(
    (std::istream_iterator<std::string>(iss)),
    std::istream_iterator<std::string>());

  EXPECT_GT(flags.size(), 0u);

  // Match each flag in script output with help message
  for (const auto &flag : flags)
  {
    EXPECT_NE(std::string::npos, helpOutput.cout.find(flag)) << helpOutput.cout;
  }
}

/// Main
int main(int argc, char **argv)
{
  // Get a random partition name.
  g_partition = testing::getRandomNumber();

  // Set the partition name for this process.
  gz::utils::setenv("GZ_PARTITION", g_partition);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
