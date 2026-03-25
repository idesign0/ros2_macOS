# Copyright (C) 2023 Open Source Robotics Foundation
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#################################################
# gz_get_sources(<sources>)
#
# From the current directory, grab all the source files and place them into
# <sources>. Remove their paths to make them suitable for passing into
# gz_add_[library/tests].
function(gz_get_sources sources_var)

  # GLOB all the source files
  file(GLOB source_files "*.cc")
  list(SORT source_files)

  # Initialize this list
  set(sources)

  foreach(source_file ${source_files})

    # Remove the path from the source file and append it the list of soures
    get_filename_component(source ${source_file} NAME)
    list(APPEND sources ${source})

  endforeach()

  # Return the list that has been created
  set(${sources_var} ${sources} PARENT_SCOPE)

endfunction()
