// -*-c++-*---------------------------------------------------------------------------------------
// Copyright 2024 Bernd Pfrommer <bernd.pfrommer@gmail.com>
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

#include <apriltag_detector/detector_component.hpp>
#include <apriltag_draw/apriltag_draw.hpp>
#include <rosbag2_transport/player.hpp>
#include <rosbag2_transport/recorder.hpp>

static rclcpp::Logger get_logger()
{
  return (rclcpp::get_logger("detect_from_bag"));
}

// need inheritance because get_publisher() is protected
class MyPlayer : public rosbag2_transport::Player
{
public:
  MyPlayer(const std::string & name, const rclcpp::NodeOptions & opt)
  : rosbag2_transport::Player(name, opt)
  {
  }

  std::set<std::string> getTopics()
  {
    std::set<std::string> topics;
#ifdef HAS_GET_PUBLISHERS
    const auto pubs = this->get_publishers();
#else
    const auto pubs = this->publishers_;
#endif
    for (const auto & kv : pubs) {
      topics.insert(kv.first);
    }
    return (topics);
  }
};

using Parameter = rclcpp::Parameter;

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  // using dummy node to get to the parameters
  auto dummy_node = std::make_shared<rclcpp::Node>("dummy_node");
  const std::string image_topic =
    dummy_node->declare_parameter<std::string>("image_topic", "");
  const std::string image_tag_topic =
    dummy_node->declare_parameter<std::string>("image_tag_topic", "image_tags");
  const std::string tag_topic =
    dummy_node->declare_parameter<std::string>("tag_topic", "tags");
  if (image_topic.empty()) {
    RCLCPP_ERROR(get_logger(), "must specify image_topic parameter!");
    rclcpp::shutdown();
    throw std::runtime_error("must specify image_topic parameter!");
  }
  dummy_node.reset();  // delete dummy node object

  rclcpp::executors::SingleThreadedExecutor exec;

  const std::vector<std::string> detector_remap = {
    "--ros-args", "--remap", "image:=" + image_topic, "--remap",
    "tags:=" + image_topic + "/tags"};

  rclcpp::NodeOptions detector_options;
  detector_options.arguments(detector_remap);
  detector_options.parameter_overrides({Parameter("use_sim_time", true)});
  detector_options.use_intra_process_comms(true);
  auto detector_node =
    std::make_shared<apriltag_detector::DetectorComponent>(detector_options);

  exec.add_node(detector_node);
  const std::vector<std::string> draw_remap = {
    "--ros-args",
    "--remap",
    "image:=" + image_topic,
    "--remap",
    "tags:=" + image_topic + "/" + tag_topic,
    "--remap",
    "image_tags:=" + image_topic + "/" + image_tag_topic};

  rclcpp::NodeOptions draw_options;
  draw_options.arguments(draw_remap);
  draw_options.parameter_overrides({Parameter("use_sim_time", true)});
  draw_options.use_intra_process_comms(true);
  auto draw_node = std::make_shared<apriltag_draw::ApriltagDraw>(draw_options);

  exec.add_node(draw_node);

  const std::string in_uri =
    detector_node->get_parameter_or<std::string>("in_bag", "");
  if (in_uri.empty()) {
    RCLCPP_ERROR_STREAM(get_logger(), "must provide valid in_bag parameter!");
    return (-1);
  }
  RCLCPP_INFO_STREAM(get_logger(), "using input bag: " << in_uri);

  rclcpp::NodeOptions player_options;
  player_options.parameter_overrides(
    {Parameter("storage.uri", in_uri),
     Parameter("play.clock_publish_on_topic_publish", true),
     Parameter("play.start_paused", true), Parameter("play.rate", 0.9),
     Parameter("play.progress_bar_update_rate", 0),
     Parameter("play.disable_keyboard_controls", true)});
  player_options.use_intra_process_comms(true);
  auto player_node =
    std::make_shared<MyPlayer>("rosbag_player", player_options);

  const auto topic_set = player_node->getTopics();
  std::vector<std::string> topics;
  std::copy(topic_set.begin(), topic_set.end(), std::back_inserter(topics));
  topics.push_back(image_topic + "/" + tag_topic);
  topics.push_back(image_topic + "/" + image_tag_topic);

  const std::string out_uri =
    detector_node->get_parameter_or("out_bag", std::string());
  if (out_uri.empty()) {
    RCLCPP_ERROR_STREAM(get_logger(), "must provide valid out_bag parameter!");
    return (-1);
  }
  std::shared_ptr<rosbag2_transport::Recorder> recorder_node;
  RCLCPP_INFO_STREAM(get_logger(), "writing detected tags to bag: " << out_uri);
  rclcpp::NodeOptions recorder_options;
  recorder_options.parameter_overrides(
    {Parameter("use_sim_time", true), Parameter("storage.uri", out_uri),
     Parameter("record.disable_keyboard_controls", true),
     Parameter("record.topics", topics)});
  recorder_options.use_intra_process_comms(true);

  recorder_node = std::make_shared<rosbag2_transport::Recorder>(
    "rosbag_recorder", recorder_options);

  exec.add_node(recorder_node);
  if (player_node->play_next() && rclcpp::ok()) {
    while (rclcpp::ok() &&
           !(draw_node->isSubscribed() && detector_node->isSubscribed() &&
             recorder_node->subscriptions().size() == topics.size())) {
      exec.spin_some();
    }
  }
  player_node->seek(0);
  // main loop where the playback + processing happens
  while (player_node->play_next() && rclcpp::ok()) {
    exec.spin_some();
  }
  exec.spin_some();  // for processing the last message
  exec.spin_some();  // for recording the last message

  RCLCPP_INFO_STREAM(
    get_logger(), "num messages found: " << detector_node->getNumMessages());
  RCLCPP_INFO_STREAM(
    get_logger(), "num tags detected: " << detector_node->getNumTagsDetected());
  RCLCPP_INFO(get_logger(), "detect_from_bag finished!");
  rclcpp::shutdown();
  return 0;
}
