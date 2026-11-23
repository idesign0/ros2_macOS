#pragma once
// Compat shim: newer Abseil removed the legacy unprefixed thread-safety
// annotation macros. Cartographer (ros2 branch) still uses these three.
// Force-included for the cartographer build via cmake/toolchain.cmake.
#include <absl/base/thread_annotations.h>
#ifndef GUARDED_BY
#define GUARDED_BY(x) ABSL_GUARDED_BY(x)
#endif
#ifndef EXCLUSIVE_LOCKS_REQUIRED
#define EXCLUSIVE_LOCKS_REQUIRED(...) ABSL_EXCLUSIVE_LOCKS_REQUIRED(__VA_ARGS__)
#endif
#ifndef LOCKS_EXCLUDED
#define LOCKS_EXCLUDED(...) ABSL_LOCKS_EXCLUDED(__VA_ARGS__)
#endif
