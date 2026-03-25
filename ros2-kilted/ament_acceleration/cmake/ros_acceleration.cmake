#    ____  ____
#   /   /\/   /
#  /___/  \  /   Copyright (c) 2021, Xilinx®.
#  \   \   \/    Author: Víctor Mayoral Vilches <victorma@xilinx.com>
#   \   \
#   /   /
#  /___/   /\
#  \   \  /  \
#   \___\/\___\
#
# set the ROS_ACCELERATION variable
#  this CMake variable helps enable capabilities and define 
#  conditions for hardware-accelerated packages at build-time.

if(NOT DEFINED ROS_ACCELERATION)
  set(ROS_ACCELERATION TRUE)
endif()
