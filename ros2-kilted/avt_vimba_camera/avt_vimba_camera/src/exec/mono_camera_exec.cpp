#include "avt_vimba_camera/mono_camera_node.hpp"

#include <memory>

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<avt_vimba_camera::MonoCameraNode>();
  node->start();
  rclcpp::spin(node);

  return 0;
}
