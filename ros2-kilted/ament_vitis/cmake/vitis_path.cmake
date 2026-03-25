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
# defines the path to the Vitis directory

if(DEFINED ENV{XILINX_VITIS})  
  set(XILINX_VITIS $ENV{XILINX_VITIS})
# else()
#   # # DEPRECATED: use local installation of Vitis
#   # message("WARNING: Using acceleration but XILINX_VITIS is not defined, acceleration kernels won't be generated.")
#   # message(STATUS "Defaulting to 'xilinx/vitis' directory.")
#   # set(XILINX_VITIS ${CMAKE_INSTALL_PREFIX}/../xilinx/vitis)  # <ws>/xilinx/vitis
#
#   # Fail
#   message(FATAL_ERROR "No XILINX_VITIS environmental variable set, please install Vitis and source its settings script.")
endif()
