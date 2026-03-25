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
# captures the firmware's System-on-Chip (SoC) value fetched from file
# <ws>/acceleration/firmware/select/SOC into the FIRMWARE_SOC variable
#
# This value is then used to make the vitis_hls_generate_tcl macro SOC-agnostic and
# purely dependant on the firmware deployed in the ROS 2 workspace.
#
# NOTE: "target" in the path below is expected to by a symlink to one of the available
#  firmware options
#
# NOTE 2: the FIRMWARE_SOC is not set the first time this package (ament_vitis) is
#  built, which leads to an error. In addition, the fact that this variable is over-
#  written every time select changes requires that changes in the cmake generated
#  file ocurr.
#  An alternative implementation could derive this variable within the 
#  vitis_hls_generate_tcl macro, at build-time, when being invoked.

set(FIRMWARE_SOC_PATH ${CMAKE_INSTALL_PREFIX}/../acceleration/firmware/select/SOC)
      # e.g. <ws>/acceleration/firmware/select/SOC

file (STRINGS ${FIRMWARE_SOC_PATH} FIRMWARE_SOC)
      # e.g. xcvu9p-flga2104-2-i