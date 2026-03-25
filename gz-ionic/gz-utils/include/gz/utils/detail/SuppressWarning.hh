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

#ifndef GZ_UTILS__DETAIL__SUPPRESSWARNING_HH_
#define GZ_UTILS__DETAIL__SUPPRESSWARNING_HH_

#include <gz/utils/SuppressWarning.hh>

#ifndef DETAIL_GZ_UTILS_STRINGIFY
  #define DETAIL_GZ_UTILS_STRINGIFY(x) #x
#endif

/* cppcheck-suppress */

// BEGIN / FINISH Macros

#if defined __clang__

  #ifndef DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH
    #define DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH \
      _Pragma("clang diagnostic push")
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_SUP_HELPER_2
    #define DETAIL_GZ_UTILS_WARN_SUP_HELPER_2(w) \
      DETAIL_GZ_UTILS_STRINGIFY(clang diagnostic ignored w)
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_SUP_HELPER
    #define DETAIL_GZ_UTILS_WARN_SUP_HELPER(w) \
      _Pragma(DETAIL_GZ_UTILS_WARN_SUP_HELPER_2(w))
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_RESUME
    #define DETAIL_GZ_UTILS_WARN_RESUME \
      _Pragma("clang diagnostic pop")
  #endif


#elif defined __GNUC__

  // NOTE: clang will define both __clang__ and __GNUC__, and it seems that
  // clang will gladly accept GCC pragmas. Even so, if we want the pragmas to
  // target the "correct" compiler, we should check if __clang__ is defined
  // before checking whether __GNUC__ is defined.

  #ifndef DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH
    #define DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH \
      _Pragma("GCC diagnostic push")
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_SUP_HELPER_2
    #define DETAIL_GZ_UTILS_WARN_SUP_HELPER_2(w) \
      DETAIL_GZ_UTILS_STRINGIFY(GCC diagnostic ignored w)
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_SUP_HELPER
    #define DETAIL_GZ_UTILS_WARN_SUP_HELPER(w) \
      _Pragma(DETAIL_GZ_UTILS_WARN_SUP_HELPER_2(w))
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_RESUME
    #define DETAIL_GZ_UTILS_WARN_RESUME \
      _Pragma("GCC diagnostic pop")
  #endif


#elif defined _MSC_VER


  #ifndef DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH
    #define DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH \
      __pragma(warning(push))
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_SUP_HELPER
    #define DETAIL_GZ_UTILS_WARN_SUP_HELPER(w) \
      __pragma(warning(disable: w))
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_RESUME
    #define DETAIL_GZ_UTILS_WARN_RESUME \
      __pragma(warning(pop))
  #endif


#else

  // Make these into no-ops if we don't know the type of compiler

  #ifndef DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH
    #define DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_SUP_HELPER
    #define DETAIL_GZ_UTILS_WARN_SUP_HELPER(w)
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_RESUME
    #define DETAIL_GZ_UTILS_WARN_RESUME
  #endif


#endif


#ifndef DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION
  #define DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION(warning_token) \
    DETAIL_GZ_UTILS_BEGIN_WARN_SUP_PUSH \
    DETAIL_GZ_UTILS_WARN_SUP_HELPER(warning_token)
#endif



// Warning Tokens
#if defined __GNUC__ || defined __clang__

  #ifndef DETAIL_GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR
    #define DETAIL_GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR \
      DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION("-Wdelete-non-virtual-dtor")
  #endif

  #ifndef DETAIL_GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR
    #define DETAIL_GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR \
      DETAIL_GZ_UTILS_WARN_RESUME
  #endif


  // There is no analogous warning for this in GCC or Clang so we just make
  // blank macros for this warning type.
  #ifndef DETAIL_GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    #define DETAIL_GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
  #endif
  #ifndef DETAIL_GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    #define DETAIL_GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
    #define DETAIL_GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION \
      DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION("-Wdeprecated-declarations")
  #endif

  #ifndef DETAIL_GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
    #define DETAIL_GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION \
      DETAIL_GZ_UTILS_WARN_RESUME
  #endif


#elif defined _MSC_VER

  #ifndef DETAIL_GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR
    #define DETAIL_GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR \
      DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION(4265) \
      DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION(5205)
  #endif

  #ifndef DETAIL_GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR
    #define DETAIL_GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR \
      DETAIL_GZ_UTILS_WARN_RESUME
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    #define DETAIL_GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING \
      DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION(4251)
  #endif

  #ifndef DETAIL_GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    #define DETAIL_GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING \
      DETAIL_GZ_UTILS_WARN_RESUME
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
    #define DETAIL_GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION \
      DETAIL_GZ_UTILS_BEGIN_WARNING_SUPPRESSION(4996)
  #endif

  #ifndef DETAIL_GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
    #define DETAIL_GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION \
      DETAIL_GZ_UTILS_WARN_RESUME
  #endif


#else

  // If the compiler is unknown, we simply leave these macros blank to avoid
  // compilation errors.

  #ifndef DETAIL_GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR
    #define DETAIL_GZ_UTILS_WARN_IGNORE__NON_VIRTUAL_DESTRUCTOR
  #endif
  #ifndef DETAIL_GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR
    #define DETAIL_GZ_UTILS_WARN_RESUME__NON_VIRTUAL_DESTRUCTOR
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
    #define DETAIL_GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
  #endif
  #ifndef DETAIL_GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    #define DETAIL_GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
  #endif


  #ifndef DETAIL_GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
    #define DETAIL_GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  #endif
  #ifndef DETAIL_GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
    #define DETAIL_GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
  #endif


#endif

#endif  // GZ_UTILS__DETAIL__SUPPRESSWARNING_HH_
