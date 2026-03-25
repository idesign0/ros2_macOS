/*
* Copyright (C) 2022 Open Source Robotics Foundation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/
#ifndef GZ_COMMON_TESTING_UTILS_HH_
#define GZ_COMMON_TESTING_UTILS_HH_

#include <climits>
#include <cstdint>
#include <string>

#include "gz/common/testing/Export.hh"

namespace gz::common::testing
{

/////////////////////////////////////////////////
/// \brief Create an empty file with no content
///
/// \param[in] _filename Filename of the file to be created
/// \return true if file successfully created, false otherwise
bool
GZ_COMMON_TESTING_VISIBLE
createNewEmptyFile(const std::string &_filename);

}  // namespace gz::common::testing

#endif  // GZ_COMMON_TESTING_TESTPATHS_HH_
