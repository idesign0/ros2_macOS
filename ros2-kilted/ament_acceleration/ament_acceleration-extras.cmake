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
# include hardware acceleration extras

find_package(ament_cmake_core QUIET REQUIRED)

# various functions / macros
foreach(filename
  "ros_acceleration"
)
  include(${ament_acceleration_DIR}/${filename}.cmake)
endforeach()
