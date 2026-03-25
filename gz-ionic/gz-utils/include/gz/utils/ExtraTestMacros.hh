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

#ifndef GZ_UTILS_EXTRATESTMACROS_HH
#define GZ_UTILS_EXTRATESTMACROS_HH

#include <gz/utils/detail/ExtraTestMacros.hh>

/// \brief Restrict the execution of the test to just the Windows platform
/// Other platforms will get the test compiled but it won't be run
/// as part of the test suite execution.
/// The macro uses the Disabled_ prefix provided by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
/// TODO(gz-utils4): remove the windows alias
#define GZ_UTILS_TEST_ENABLED_ONLY_ON_WINDOWS(TestName) \
  DETAIL_GZ_UTILS_TEST_ENABLED_ONLY_ON_WIN32(TestName)

/// \brief Restrict the execution of the test to just the Windows platform
/// Other platforms will get the test compiled but it won't be run
/// as part of the test suite execution.
/// The macro uses the Disabled_ prefix provided by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_ENABLED_ONLY_ON_WIN32(TestName) \
  DETAIL_GZ_UTILS_TEST_ENABLED_ONLY_ON_WIN32(TestName)

/// \brief Restrict the execution of the test for the Windows platform.
/// The test will be compiled on Windows too but will never be run as
/// part of the test suite. The macro uses the Disabled_ prefix provided
/// by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_DISABLED_ON_WIN32(TestName) \
  DETAIL_GZ_UTILS_TEST_DISABLED_ON_WIN32(TestName)

/// \brief Restrict the execution of the test to just the MacOS platform
/// Other platforms will get the test compiled but it won't be run
/// as part of the test suite execution.
/// The macro uses the Disabled_ prefix provided by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_ENABLED_ONLY_ON_MAC(TestName) \
  DETAIL_GZ_UTILS_TEST_ENABLED_ONLY_ON_MAC(TestName)

/// \brief Restrict the execution of the test for the Mac platform.
/// The test will be compiled on Mac too but will never be run as
/// part of the test suite. The macro uses the Disabled_ prefix provided
/// by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_DISABLED_ON_MAC(TestName) \
  DETAIL_GZ_UTILS_TEST_DISABLED_ON_MAC(TestName)

/// \brief Restrict the execution of the test to just the Linux platform
/// Other platforms will get the test compiled but it won't be run
/// as part of the test suite execution.
/// The macro uses the Disabled_ prefix provided by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(TestName) \
  DETAIL_GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(TestName)

/// \brief Restrict the execution of the test for the Linux platform
/// The test will be compiled on Linux too but will never be run as
/// part of the test suite. The macro uses the Disabled_ prefix provided
/// by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_DISABLED_ON_LINUX(TestName) \
  DETAIL_GZ_UTILS_TEST_DISABLED_ON_LINUX(TestName)

/// \brief Restrict the execution of the test to just the ARM32 architecture
/// Other platforms will get the test compiled but it won't be run
/// as part of the test suite execution.
/// The macro uses the Disabled_ prefix provided by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_ENABLED_ONLY_ON_ARM32(TestName) \
  DETAIL_GZ_UTILS_TEST_ENABLED_ONLY_ON_ARM32(TestName)

/// \brief Restrict the execution of the test for the ARM32 architecture
/// The test will be compiled on ARM32 too but will never be run as
/// part of the test suite. The macro uses the Disabled_ prefix provided
/// by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_DISABLED_ON_ARM32(TestName) \
  DETAIL_GZ_UTILS_TEST_DISABLED_ON_ARM32(TestName)

/// \brief Restrict the execution of the test to just the ARM64 architecture
/// Other platforms will get the test compiled but it won't be run
/// as part of the test suite execution.
/// The macro uses the Disabled_ prefix provided by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_ENABLED_ONLY_ON_ARM64(TestName) \
  DETAIL_GZ_UTILS_TEST_ENABLED_ONLY_ON_ARM64(TestName)

/// \brief Restrict the execution of the test for the ARM64 architecture
/// The test will be compiled on Linux too but will never be run as
/// part of the test suite. The macro uses the Disabled_ prefix provided
/// by googletest. See
/// https://chromium.googlesource.com/external/github.com/google/googletest/+/HEAD/googletest/docs/advanced.md
#define GZ_UTILS_TEST_DISABLED_ON_ARM64(TestName) \
  DETAIL_GZ_UTILS_TEST_DISABLED_ON_ARM64(TestName)

#endif  // GZ_UTILS_EXTRATESTMACROS_HH
