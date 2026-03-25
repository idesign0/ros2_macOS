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
# defines the path to the Vivado directory

if(DEFINED ENV{XILINX_VIVADO})
  set(XILINX_VIVADO $ENV{XILINX_VIVADO})
else()
  message("WARNING: XILINX_VIVADO is not defined, hardware acceleration disabled.")
  message(STATUS "Defaulting to 'xilinx/vivado' directory.")
  set(XILINX_VIVADO ${CMAKE_INSTALL_PREFIX}/../xilinx/vivado)  # <ws>/xilinx/vivado
endif()
