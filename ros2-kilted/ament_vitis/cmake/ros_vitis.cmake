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
# set the ROS_VITIS variable
#  used to signal the ROS 2 build system that Vitis is available while constructing binaries and/or libraries
#
# NOTE: refer to the ROS_ACCELERATION CMake variable for
# general hardware acceleration expressions.

# NOTE 2: the ROS_VITIS_TARGET variable is defined by the ament ROS 2 build system
# extensions. Particularly, from the corresponding mixin that should be used 
# while cross-compiling things for the selected target hardware.

if(NOT DEFINED ROS_VITIS)
  if(ROS_VITIS_TARGET)
    set(ROS_VITIS TRUE)    
  endif()
endif()
