/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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


#ifndef GZ_UTILITIES_DETAIL_SUPPRESSWARNING_HH_
#define GZ_UTILITIES_DETAIL_SUPPRESSWARNING_HH_

#include <gz/utilities/SuppressWarning.hh>

#define DETAIL_GZ_UTILS_STRINGIFY(x) #x

/* cppcheck-suppress */

// BEGIN / FINISH Macros

#if defined __clang__

  #define DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH \
    _Pragma("clang diagnostic push")


  #define DETAIL_GZ_UTILS_WARN_SUP_HELPER_2(w) \
    DETAIL_GZ_UTILS_STRINGIFY(clang diagnostic ignored w)


  #define DETAIL_GZ_UTILS_WARN_SUP_HELPER(w) \
    _Pragma(DETAIL_GZ_UTILS_WARN_SUP_HELPER_2(w))


  #define DETAIL_GZ_UTILS_WARN_RESUME \
    _Pragma("clang diagnostic pop")


#elif defined __GNUC__

  // NOTE: clang will define both __clang__ and __GNUC__, and it seems that
  // clang will gladly accept GCC pragmas. Even so, if we want the pragmas to
  // target the "correct" compiler, we should check if __clang__ is defined
  // before checking whether __GNUC__ is defined.

  #define DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH \
    _Pragma("GCC diagnostic push")


  #define DETAIL_GZ_UTILS_WARN_SUP_HELPER_2(w) \
    DETAIL_GZ_UTILS_STRINGIFY(GCC diagnostic ignored w)


  #define DETAIL_GZ_UTILS_WARN_SUP_HELPER(w) \
    _Pragma(DETAIL_GZ_UTILS_WARN_SUP_HELPER_2(w))


  #define DETAIL_GZ_UTILS_WARN_RESUME \
    _Pragma("GCC diagnostic pop")


#elif defined _MSC_VER


  #define DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH \
    __pragma(warning(push))


  #define DETAIL_GZ_UTILS_WARN_SUP_HELPER(w) \
    __pragma(warning(disable: w))


  #define DETAIL_GZ_UTILS_WARN_RESUME \
    __pragma(warning(pop))


#else

  // Make these into no-ops if we don't know the type of compiler

  #define DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH


  #define DETAIL_GZ_UTILS_WARN_SUP_HELPER(w)


  #define DETAIL_GZ_UTILS_WARN_RESUME


#endif


#define DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION(warning_token) \
  DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH \
  DETAIL_GZ_UTILS_WARN_SUP_HELPER(warning_token)



// Warning Tokens
#if defined __GNUC__ || defined __clang__

  #define DETAIL_GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR \
    DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION("-Wdelete-non-virtual-dtor")

  #define DETAIL_GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR \
    DETAIL_GZ_UTILS_WARN_RESUME


  // There is no analogous warning for this in GCC or Clang so we just make
  // blank macros for this warning type.
  #define DETAIL_GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
  #define DETAIL_GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING


#elif defined _MSC_VER

  #define DETAIL_GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR \
    DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION(4265) \
    DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION(5205)

  #define DETAIL_GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR \
    DETAIL_GZ_UTILS_WARN_RESUME


  #define DETAIL_GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING \
    DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION(4251)

  #define DETAIL_GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING \
    DETAIL_GZ_UTILS_WARN_RESUME


#else

  // If the compiler is unknown, we simply leave these macros blank to avoid
  // compilation errors.

  #define DETAIL_GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR
  #define DETAIL_GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR


  #define DETAIL_GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
  #define DETAIL_GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING


#endif


#endif
