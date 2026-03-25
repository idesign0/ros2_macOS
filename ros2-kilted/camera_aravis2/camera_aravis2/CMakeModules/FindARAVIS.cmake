# Copyright (c) 2024 Fraunhofer IOSB and contributors
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


set(ARAVIS_VERSION 0.8)

find_path(ARAVIS_INCLUDE_DIRS arv.h
  PATHS
  "$ENV{ARAVIS_INCLUDE_PATH}"
  /usr/local/include/aravis-${ARAVIS_VERSION}
  /usr/include/aravis-${ARAVIS_VERSION}
)

find_library(ARAVIS_LIBRARIES aravis-${ARAVIS_VERSION}
  PATHS
  "$ENV{ARAVIS_LIBRARY}"
  /usr/local/lib
  /usr/lib
  /usr/lib/x86_64-linux-gnu
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( ARAVIS DEFAULT_MSG
  ARAVIS_INCLUDE_DIRS
  ARAVIS_LIBRARIES
)
