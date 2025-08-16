// Copyright (c) 2019, Open Source Robotics Foundation, Inc.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef INTERACTIVE_MARKERS__TIMED_EXPECT_HPP_
#define INTERACTIVE_MARKERS__TIMED_EXPECT_HPP_
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "rclcpp/rclcpp.hpp"

#define TIMED_EXPECT_EQ_6_ARGS(lhs, rhs, timeout, period, executor, func, ...) \
  do { \
    auto start_time = std::chrono::steady_clock::now(); \
    while (lhs != rhs && \
      (std::chrono::steady_clock::now() - start_time < timeout)) \
    { \
      executor.spin_once(std::chrono::nanoseconds(0)); \
      func(); \
      std::this_thread::sleep_for(period); \
    } \
    EXPECT_EQ(lhs, rhs); \
  } while (0)

#define TIMED_EXPECT_EQ_5_ARGS(lhs, rhs, timeout, period, executor, ...) \
  TIMED_EXPECT_EQ_6_ARGS(lhs, rhs, timeout, period, executor, [] {}, UNUSED)

#define TIMED_EXPECT_EQ_CHOOSER(lhs, rhs, timeout, period, executor, func, args, ...) args

// Workaround for MSVC's preprocessor
// See: https://stackoverflow.com/a/5134656
#define EXPAND(x) x

/// Assert equality with a timeout.
/**
 * This macro blocks until lhs == rhs or a timeout occurs.
 * The provided executor spins for the duration of this macro.
 * The optional function is also repeatedly called.
 *
 * The == and != operators must be defined for lhs and rhs types.
 *
 * \param lhs The left hand operand of the equality test.
 * \param rhs The right hand operand of the equality test.
 * \param timeout The duration after which the test will fail.
 * \param period The period at which the executor spins and the provided function is called.
 * \param executor The executor to spin.
 * \param func The optional function to call.
 */
#define TIMED_EXPECT_EQ(...) \
  EXPAND( \
    TIMED_EXPECT_EQ_CHOOSER( \
      __VA_ARGS__, \
      TIMED_EXPECT_EQ_6_ARGS(__VA_ARGS__, UNUSED), \
      TIMED_EXPECT_EQ_5_ARGS(__VA_ARGS__, UNUSED), \
      UNUSED))

#endif  // INTERACTIVE_MARKERS__TIMED_EXPECT_HPP_
