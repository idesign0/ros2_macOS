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
# defines the path to the PLATFORM_REPO_PATHS env. variable
#
# NOTE: "target" in the path below is expected to by a symlink to one of the available
#  firmware options


if(DEFINED ENV{PLATFORM_REPO_PATHS})
  set(PLATFORM_REPO_PATHS $ENV{PLATFORM_REPO_PATHS})
else()
  set(PLATFORM_REPO_PATHS ${CMAKE_INSTALL_PREFIX}/../acceleration/firmware/select/platform)
      # <ws>/acceleration/firmware/select/platform
endif()
