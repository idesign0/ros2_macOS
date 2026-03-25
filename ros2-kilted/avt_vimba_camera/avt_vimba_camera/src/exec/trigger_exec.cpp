#include "avt_vimba_camera/trigger_node.hpp"

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<trigger::TriggerNode>();
  rclcpp::spin(node);

  return 0;
}
