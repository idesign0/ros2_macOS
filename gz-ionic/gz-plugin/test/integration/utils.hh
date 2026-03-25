/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#ifndef GZ_TEST_INTEGRATION_UTILS_HH_
#define GZ_TEST_INTEGRATION_UTILS_HH_

#include <dlfcn.h>

/////////////////////////////////////////////////
// The macro RTLD_NOLOAD is not part of the POSIX standard, and is a custom
// addition to glibc-2.2, so the unloading test can only work when we are using
// glibc-2.2 or higher. The unloading tests fundamentally require the use of the
// RTLD_NOLOAD feature, because without it, there is no way to observe that a
// library is not loaded.
#ifdef RTLD_NOLOAD

/////////////////////////////////////////////////
// Note (MXG): According to some online discussions, there is no guarantee
// that a correct number of calls to dlclose(void*) will actually unload the
// shared library. In fact, there is no guarantee that a dynamically loaded
// library from dlopen will ever be unloaded until the program is terminated.
// This may cause dlopen(~, RTLD_NOLOAD) to return a non-null handle even if
// we are managing the handles correctly. If the test for
// EXPECT_EQ(nullptr, dlHandle) is found to fail occasionally, we should
// consider removing it because it may be unreliable. At the very least, if
// it fails very infrequently, then we can safely consider the failures to be
// false negatives and may want to consider relaxing this test.
#define CHECK_FOR_LIBRARY(_path, _isLoaded) \
{ \
  void *dlHandle = dlopen(_path.c_str(), \
                          RTLD_NOLOAD | RTLD_LAZY | RTLD_GLOBAL); \
  \
  if (_isLoaded) \
    EXPECT_NE(nullptr, dlHandle); \
  else /* NOLINT */ \
    EXPECT_EQ(nullptr, dlHandle); \
  \
  if (dlHandle) \
    dlclose(dlHandle); \
}

#else

#define CHECK_FOR_LIBRARY(_path, _isLoaded)

#endif

#endif
