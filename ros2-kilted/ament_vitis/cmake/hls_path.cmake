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
# defines the path to the Vitis HLS directory

if(DEFINED ENV{XILINX_HLS})
  set(XILINX_HLS $ENV{XILINX_HLS})
else()
  message("WARNING: XILINX_HLS is not defined, hardware acceleration disabled.")
  message(STATUS "Defaulting to 'xilinx/hls' directory.")
  set(XILINX_HLS ${CMAKE_INSTALL_PREFIX}/../xilinx/hls)  # <ws>/xilinx/hls
endif()
