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
# set the ROS_XRT variable
#  used to signal the ROS 2 build system that the Xilinx RunTime (XRT) is available for constructing binaries and/or libraries
#
# NOTE: refer to the ROS_ACCELERATION or ROS_VITIS CMake variables for
# general hardware acceleration expressions and Vitis-specific Nodes, 
# respectively.

if(NOT DEFINED ROS_XRT)
    # TODO: check the dev. rootfs and enable if XRT installed appropriately
    #   alternative, consider a second variable that allows user to define
    #   where XRT has been installed.
    set(ROS_XRT FALSE)
endif()
