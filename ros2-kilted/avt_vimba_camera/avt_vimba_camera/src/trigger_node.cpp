#include "avt_vimba_camera/trigger_node.hpp"

namespace trigger
{
TriggerNode::TriggerNode() : Node("trigger"), vimba_system_(AVT::VmbAPI::VimbaSystem::GetInstance())
{
  clock_ = rclcpp::Clock(RCL_ROS_TIME);

  VmbErrorType return_value = vimba_system_.Startup();

  if (return_value != VmbErrorSuccess)
  {
    RCLCPP_ERROR_STREAM(this->get_logger(), "Failed to start Vimba system, vimba error code: " << return_value);
    rclcpp::shutdown();
  }

  LoadParams();
  InitializeAddress();

  if (trigger_src_ == "timer")
  {
    trigger_timer_ =
        this->create_wall_timer(std::chrono::duration<float>(timer_period_), std::bind(&TriggerNode::TimerCb, this));
  }
  else if (trigger_src_ == "subscriber")
  {
    trigger_sub_ = this->create_subscription<std_msgs::msg::Bool>(
        "trigger_input", 10, std::bind(&TriggerNode::TriggerCb, this, std::placeholders::_1));
  }
  else
  {
    RCLCPP_ERROR(this->get_logger(), "Unknown trigger_src %s", trigger_src_.c_str());
    rclcpp::shutdown();
  }
}

TriggerNode::~TriggerNode()
{
  vimba_system_.Shutdown();
}

void TriggerNode::LoadParams()
{
  std::string destination_ip = this->declare_parameter<std::string>("destination_ip", "");
  trigger_src_ = this->declare_parameter<std::string>("trigger_src", "timer");
  timer_period_ = this->declare_parameter<double>("timer_period", 0.1);
  action_device_key_ = this->declare_parameter<int64_t>("action_device_key", 1);
  action_group_key_ = this->declare_parameter<int64_t>("action_group_key", 1);
  action_group_mask_ = this->declare_parameter<int64_t>("action_group_mask", 1);

  if (inet_aton(destination_ip.c_str(), &destination_ip_) == 0)
  {
    RCLCPP_ERROR(this->get_logger(), "Unable to parse desination_ip: %s", destination_ip.c_str());
    rclcpp::shutdown();
  }

  RCLCPP_INFO(this->get_logger(), "Sending action commands to %s", destination_ip.c_str());
}

void TriggerNode::InitializeAddress()
{
  if (!SetIntFeatureValue("GevActionDestinationIPAddress", destination_ip_.s_addr))
  {
    RCLCPP_ERROR(this->get_logger(), "Could not set destination address");
  }
}

bool TriggerNode::PrepareActionCommand()
{
  return (SetIntFeatureValue("ActionDeviceKey", 1) && SetIntFeatureValue("ActionGroupKey", 1) &&
          SetIntFeatureValue("ActionGroupMask", 1));
}

// Sets an integer feature value on the vimba system
bool TriggerNode::SetIntFeatureValue(const std::string& name, int64_t value)
{
  VmbErrorType return_value = VmbErrorSuccess;

  AVT::VmbAPI::FeaturePtr feature_ptr;
  return_value = vimba_system_.GetFeatureByName(name.c_str(), feature_ptr);

  if (return_value != VmbErrorSuccess)
  {
    RCLCPP_ERROR_STREAM(this->get_logger(), "Failed to get feature, vimba error code: " << return_value);
    return false;
  }
  else
  {
    return_value = feature_ptr->SetValue((VmbInt64_t)value);
  }

  return (return_value == VmbErrorSuccess);
}

void TriggerNode::TimerCb()
{
  SendActionCommand();
}

void TriggerNode::TriggerCb(const std_msgs::msg::Bool::SharedPtr msg)
{
  (void)msg;  // unused
  SendActionCommand();
}

void TriggerNode::SendActionCommand()
{
  if (!PrepareActionCommand())
  {
    RCLCPP_ERROR_THROTTLE(this->get_logger(), clock_, 1.0, "Failed to prepare action command");
    return;
  }

  VmbErrorType return_value = VmbErrorSuccess;

  AVT::VmbAPI::FeaturePtr lFeature;
  return_value = vimba_system_.GetFeatureByName("ActionCommand", lFeature);

  if (return_value == VmbErrorSuccess)
  {
    return_value = lFeature->RunCommand();
  }

  if (return_value == VmbErrorSuccess)
  {
    RCLCPP_DEBUG(this->get_logger(), "Action command sent");
  }
  else
  {
    RCLCPP_ERROR_THROTTLE(this->get_logger(), clock_, 1.0, "Failed to send action command");
  }
}

}  // namespace trigger
