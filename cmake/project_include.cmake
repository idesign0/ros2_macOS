# Injected into EVERY project() via CMAKE_PROJECT_INCLUDE (set in toolchain.cmake).
# Runs after the compiler is detected, so it can adjust per-language settings that
# the compiler-detection step would otherwise overwrite.

# --- Keep the C SDK's usr/include off the compile line -------------------------------
# Some exported ROS configs drag the macOS SDK's C include dir into their interface
# include dirs -- the classic offender is FindCURL, which on macOS resolves
# CURL_INCLUDE_DIRS to ${CMAKE_OSX_SYSROOT}/usr/include and gets re-exported through
# resource_retriever -> rviz_common -> every rviz plugin. CMake then emits it as
# `-isystem ${SDK}/usr/include`, which is searched BEFORE libc++'s own headers, so
# <cstddef>/<cstdlib>/<cmath> pick up the C SDK copies and libc++ stops working:
#   "<cstddef> tried including <stddef.h> but didn't find libc++'s <stddef.h> ...
#    The header search paths should contain the C++ Standard Library headers before
#    any C Standard Library" (cartographer_rviz's Qt AUTOMOC TU).
# Force-including <cstdlib>/<cmath> cannot fix it -- the forced includes hit the same
# poisoned search path. Listing the directory as an IMPLICIT include dir makes CMake
# OMIT it from the generated compile lines (the same mechanism that keeps /usr/include
# off them), while the compiler still finds those headers through its own defaults.
if(APPLE)
  foreach(_ci_lang C CXX OBJC OBJCXX)
    list(APPEND CMAKE_${_ci_lang}_IMPLICIT_INCLUDE_DIRECTORIES
      "${CMAKE_OSX_SYSROOT}/usr/include" "/usr/include")
    list(REMOVE_DUPLICATES CMAKE_${_ci_lang}_IMPLICIT_INCLUDE_DIRECTORIES)
  endforeach()
  unset(_ci_lang)
endif()
