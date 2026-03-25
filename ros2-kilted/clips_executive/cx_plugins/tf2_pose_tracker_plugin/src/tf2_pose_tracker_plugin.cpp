// Copyright (c) 2024-2026 Carologistics
// SPDX-License-Identifier: Apache-2.0
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

#include "cx_tf2_pose_tracker_plugin/tf2_pose_tracker_plugin.hpp"

#include <string>

#include "cx_utils/clips_env_context.hpp"
#include "cx_utils/format.hpp"
#include "cx_utils/param_utils.hpp"

// To export as plugin
#include "pluginlib/class_list_macros.hpp"

using std::placeholders::_1;
namespace cx
{
Tf2PoseTrackerPlugin::Tf2PoseTrackerPlugin() {}

Tf2PoseTrackerPlugin::~Tf2PoseTrackerPlugin() {}

void Tf2PoseTrackerPlugin::initialize()
{
  logger_ = std::make_unique<rclcpp::Logger>(rclcpp::get_logger(plugin_name_));

  auto node = parent_.lock();
  // fetch plugin parameter
  cx::cx_utils::declare_parameter_if_not_declared(
    node, plugin_name_ + ".spin_thread", rclcpp::ParameterValue(true));
  bool tf_spin_thread;
  node->get_parameter(plugin_name_ + ".spin_thread", tf_spin_thread);

  // setup transform listener
  cb_group_ = node->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
  tf_buffer_ = std::make_unique<tf2_ros::Buffer>(node->get_clock());
  tf_listener_ = std::make_unique<tf2_ros::TransformListener>(
    *tf_buffer_, node->get_node_base_interface(), node->get_node_logging_interface(),
    node->get_node_parameters_interface(), node->get_node_topics_interface(), tf_spin_thread);
}

void Tf2PoseTrackerPlugin::finalize()
{
  // release all memory and cancel all timers
  for (auto & pose_tracker : pose_trackers_) {
    pose_tracker->timer->cancel();
    clips::ReleaseFact(pose_tracker->pose_fact);
  }

  // release references
  pose_trackers_.clear();
  logger_.reset();
  tf_buffer_.reset();
  tf_listener_.reset();
  cb_group_.reset();
}

bool Tf2PoseTrackerPlugin::clips_env_init(std::shared_ptr<clips::Environment> & env)
{
  auto context = CLIPSEnvContext::get_context(env.get());
  RCLCPP_DEBUG(*logger_, "Initializing plugin for environment %s", context->env_name_.c_str());

  // define fact template
  clips::Build(
    env.get(),
    R"CLIPS(
(deftemplate tf2-tracked-pose
    (slot parent (type STRING))
    (slot child (type STRING))
    (slot stamp (type FLOAT))
    (multislot translation (type FLOAT) (cardinality 3 3))
    (multislot rotation (type FLOAT) (cardinality 4 4))
    (slot timer (type EXTERNAL-ADDRESS))
)
)CLIPS");

  // user defined functions
  clips::AddUDF(
    env.get(), "tf2-start-periodic-lookup", "b", 3, 3, ";sy;sy;d",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<Tf2PoseTrackerPlugin *>(udfc->context);
      clips::UDFValue parent, child, freq;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, LEXEME_BITS, &parent);
      clips::UDFNthArgument(udfc, 2, LEXEME_BITS, &child);
      clips::UDFNthArgument(udfc, 3, NUMBER_BITS, &freq);

      try {
        instance->start_periodic_lookup(
          env, parent.lexemeValue->contents, child.lexemeValue->contents,
          freq.floatValue->contents);
        out->lexemeValue = clips::CreateBoolean(env, true);
      } catch (std::exception & e) {
        RCLCPP_ERROR(*instance->logger_, "Failed to create pose updater: %s", e.what());
        out->lexemeValue = clips::CreateBoolean(env, false);
      }
    },
    "tf2_start_periodic_lookup", this);

  clips::AddUDF(
    env.get(), "tf2-stop-periodic-lookup", "b", 1, 1, ";e",
    [](clips::Environment * env, clips::UDFContext * udfc, clips::UDFValue * out) {
      auto * instance = static_cast<Tf2PoseTrackerPlugin *>(udfc->context);
      clips::UDFValue pose_tracker;
      using namespace clips;  // NOLINT
      clips::UDFNthArgument(udfc, 1, EXTERNAL_ADDRESS_BIT, &pose_tracker);
      PoseTracker * typed_pose_tracker =
        static_cast<PoseTracker *>(pose_tracker.externalAddressValue->contents);
      out->lexemeValue =
        clips::CreateBoolean(env, instance->stop_periodic_lookup(typed_pose_tracker));
    },
    "tf2_stop_periodic_lookup", this);

  return true;
}

bool Tf2PoseTrackerPlugin::clips_env_destroyed(std::shared_ptr<clips::Environment> & env)
{
  auto context = CLIPSEnvContext::get_context(env.get());
  RCLCPP_INFO(*logger_, "Destroying plugin for environment %s", context->env_name_.c_str());
  clips::Environment * env_ptr = env.get();
  auto rm_it = std::remove_if(
    pose_trackers_.begin(), pose_trackers_.end(),
    [this, env_ptr](const std::shared_ptr<PoseTracker> & p) { return p->env == env_ptr; });
  if (rm_it != pose_trackers_.end()) {
    for (auto it = rm_it; it < pose_trackers_.end(); it++) {
      it->get()->timer->cancel();
      clips::ReleaseFact(it->get()->pose_fact);
    }
    pose_trackers_.erase(rm_it, pose_trackers_.end());
  }

  clips::Deftemplate * curr_tmpl = clips::FindDeftemplate(env.get(), "tf2-tracked-pose");
  if (curr_tmpl) {
    clips::Undeftemplate(curr_tmpl, env.get());
  }
  clips::RemoveUDF(env.get(), "tf2-start-periodic-lookup");
  clips::RemoveUDF(env.get(), "tf2-stop-periodic-lookup");
  return true;
}

void Tf2PoseTrackerPlugin::start_periodic_lookup(
  clips::Environment * env, const std::string & parent, const std::string & child, double frequency)
{
  std::shared_ptr<PoseTracker> pose_tracker = std::make_shared<PoseTracker>();
  pose_tracker->env = env;
  auto node = parent_.lock();
  pose_tracker->timer = node->create_wall_timer(
    std::chrono::duration<double>(1.0 / frequency),
    [this, pose_tracker, env, parent, child]() {
      geometry_msgs::msg::TransformStamped tf;
      try {
        tf = tf_buffer_->lookupTransform(parent, child, tf2::TimePointZero);
        double stamp_sec = tf.header.stamp.sec + tf.header.stamp.nanosec * 1e-9;

        // safely access CLIPS environment
        auto context = CLIPSEnvContext::get_context(env);
        std::scoped_lock clips_lock{context->env_mtx_};

        // update exisiting fact or create new one
        bool fact_exists = clips::FactExistp(pose_tracker->pose_fact);
        if (!pose_tracker->pose_fact || !fact_exists) {
          if (pose_tracker->pose_fact) {
            // fact was retained before but did not survive the engine, this
            // is not supposed to happen
            RCLCPP_WARN(
              *logger_,
              "TF lookup from %s to %s: fact was retained but does "
              "not exist anymore",
              parent.c_str(), child.c_str());
            clips::ReleaseFact(pose_tracker->pose_fact);
          }

          // New fact needed, build and retain it
          clips::FactBuilder * fact_builder = clips::CreateFactBuilder(env, "tf2-tracked-pose");
          clips::FBPutSlotCLIPSExternalAddress(
            fact_builder, "timer", clips::CreateCExternalAddress(env, pose_tracker.get()));
          clips::FBPutSlotString(fact_builder, "parent", parent.c_str());
          clips::FBPutSlotString(fact_builder, "child", child.c_str());
          clips::FBPutSlotFloat(fact_builder, "stamp", stamp_sec);
          clips::FBPutSlotMultifield(
            fact_builder, "translation",
            clips::StringToMultifield(
              env, cx::format(
                     "{} {} {}", tf.transform.translation.x, tf.transform.translation.y,
                     tf.transform.translation.z)
                     .c_str()));
          clips::FBPutSlotMultifield(
            fact_builder, "rotation",
            clips::StringToMultifield(
              env, cx::format(
                     "{} {} {} {}", tf.transform.rotation.x, tf.transform.rotation.y,
                     tf.transform.rotation.z, tf.transform.rotation.w)
                     .c_str()));
          pose_tracker->pose_fact = clips::FBAssert(fact_builder);
          clips::RetainFact(pose_tracker->pose_fact);
          clips::FBDispose(fact_builder);
        } else {
          // the fact exists and can can be modified
          clips::ReleaseFact(pose_tracker->pose_fact);
          clips::FactModifier * fact_modifier =
            clips::CreateFactModifier(env, pose_tracker->pose_fact);
          clips::FMPutSlotFloat(fact_modifier, "stamp", stamp_sec);
          clips::FMPutSlotMultifield(
            fact_modifier, "translation",
            clips::StringToMultifield(
              env, cx::format(
                     "{} {} {}", tf.transform.translation.x, tf.transform.translation.y,
                     tf.transform.translation.z)
                     .c_str()));
          clips::FMPutSlotMultifield(
            fact_modifier, "rotation",
            clips::StringToMultifield(
              env, cx::format(
                     "{} {} {} {}", tf.transform.rotation.x, tf.transform.rotation.y,
                     tf.transform.rotation.z, tf.transform.rotation.w)
                     .c_str()));
          pose_tracker->pose_fact = clips::FMModify(fact_modifier);
          clips::RetainFact(pose_tracker->pose_fact);
          clips::FMDispose(fact_modifier);
        }
      } catch (const tf2::TransformException & e) {
        RCLCPP_WARN(*logger_, "TF lookup failed: %s", e.what());
      }
    },
    cb_group_);

  // store the pose tracker
  pose_trackers_.push_back(pose_tracker);
}

bool Tf2PoseTrackerPlugin::stop_periodic_lookup(PoseTracker * pose_tracker)
{
  // lookup active updater, cancel the timer and release the fact address
  auto it = std::find_if(
    pose_trackers_.begin(), pose_trackers_.end(),
    [pose_tracker](const std::shared_ptr<PoseTracker> & p) { return p.get() == pose_tracker; });

  if (it != pose_trackers_.end()) {
    clips::ReleaseFact(it->get()->pose_fact);
    it->get()->timer->cancel();
    pose_trackers_.erase(it);
    return true;
  } else {
    RCLCPP_WARN(
      *logger_,
      "tf2-stop-periodic-lookup: failed to stop periodic "
      "lookup, invalid pointer!");
    return false;
  }
}

}  // namespace cx

PLUGINLIB_EXPORT_CLASS(cx::Tf2PoseTrackerPlugin, cx::ClipsPlugin)
