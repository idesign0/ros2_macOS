/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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

#ifndef GZ_UTILITIES_DETAIL_EXTRATESTMACROS_HH
#define GZ_UTILITIES_DETAIL_EXTRATESTMACROS_HH

#include <gz/utilities/SuppressWarning.hh>

#define DETAIL_GZ_UTILS_ADD_DISABLED_PREFIX(x) DISABLED_##x

#if defined _WIN32

  #define DETAIL_GZ_UTILS_TEST_DISABLED_ON_WIN32(TestName) \
      DETAIL_GZ_UTILS_ADD_DISABLED_PREFIX(TestName)

#else

  #define DETAIL_GZ_UTILS_TEST_DISABLED_ON_WIN32(TestName) \
      TestName

#endif

#if defined __APPLE__

  #define DETAIL_GZ_UTILS_TEST_DISABLED_ON_MAC(TestName) \
      DETAIL_GZ_UTILS_ADD_DISABLED_PREFIX(TestName)

#else

  #define DETAIL_GZ_UTILS_TEST_DISABLED_ON_MAC(TestName) \
      TestName

#endif

#if defined __linux__

  #define DETAIL_GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(TestName) \
      TestName

#else

  #define DETAIL_GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(TestName) \
      DETAIL_GZ_UTILS_ADD_DISABLED_PREFIX(TestName)

#endif


#endif
