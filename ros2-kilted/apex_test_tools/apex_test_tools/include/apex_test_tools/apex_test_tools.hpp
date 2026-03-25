// Copyright 2017-2020 Apex.AI, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef APEX_TEST_TOOLS__APEX_TEST_TOOLS_HPP_
#define APEX_TEST_TOOLS__APEX_TEST_TOOLS_HPP_

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>
#include <osrf_testing_tools_cpp/memory_tools/testing_helpers.hpp>
#include <osrf_testing_tools_cpp/memory_tools/memory_tools.hpp>
#include <osrf_testing_tools_cpp/scope_exit.hpp>
#include <string>
#include <tuple>

namespace apex_test_tools
{
namespace memory_test
{

static std::tuple<const char *, const char *> whitelist_function_names[] = {
  std::tuple<const char *, const char *>{"testing::Message", "src/gtest.cc"},
};

inline bool is_whitelist_stacktrace(const osrf_testing_tools_cpp::memory_tools::StackTrace * st)
{
  for (const auto & trace : st->get_traces()) {
    for (const auto & whitelist_name : whitelist_function_names) {
      const auto & fn_name = trace.object_function();
      const auto & src_name = trace.source_location().filename();

      if ((fn_name.find(std::get<0>(whitelist_name)) != std::string::npos) &&
        (src_name.find(std::get<1>(whitelist_name)) != std::string::npos))
      {
        return true;
      }
    }
  }
  return false;
}

inline void resume(void)
{
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_begin();
  osrf_testing_tools_cpp::memory_tools::expect_no_realloc_begin();
  osrf_testing_tools_cpp::memory_tools::expect_no_calloc_begin();
  osrf_testing_tools_cpp::memory_tools::expect_no_free_begin();
}

inline void pause(void)
{
  osrf_testing_tools_cpp::memory_tools::expect_no_malloc_end();
  osrf_testing_tools_cpp::memory_tools::expect_no_realloc_end();
  osrf_testing_tools_cpp::memory_tools::expect_no_calloc_end();
  osrf_testing_tools_cpp::memory_tools::expect_no_free_end();
}

inline void start(void)
{
  osrf_testing_tools_cpp::memory_tools::initialize();
  // create a callback for "unexpected" mallocs that does a non-fatal gtest
  // failure and then register it with memory tools
  auto on_unexpected_malloc =
    [](osrf_testing_tools_cpp::memory_tools::MemoryToolsService & service) {
      // First, walk the stack trace to make sure we're not blowing up because of something
      // in googletest.  See gitlab issue #2489 for some history
      if (is_whitelist_stacktrace(service.get_stack_trace())) {
        service.ignore();
        return;
      }

      ADD_FAILURE() << "apex_test_tools::memory_test: Unexpected malloc";
      // this will cause a bracktrace to be printed for each unexpected malloc
      service.print_backtrace();
    };
  osrf_testing_tools_cpp::memory_tools::on_unexpected_malloc(on_unexpected_malloc);

  auto on_unexpected_realloc =
    [](osrf_testing_tools_cpp::memory_tools::MemoryToolsService & service) {
      ADD_FAILURE() << "apex_test_tools::memory_test: Unexpected realloc";
      // this will cause a bracktrace to be printed for each unexpected realloc
      service.print_backtrace();
    };
  osrf_testing_tools_cpp::memory_tools::on_unexpected_realloc(on_unexpected_realloc);

  auto on_unexpected_calloc =
    [](osrf_testing_tools_cpp::memory_tools::MemoryToolsService & service) {
      ADD_FAILURE() << "apex_test_tools::memory_test: Unexpected calloc";
      // this will cause a bracktrace to be printed for each unexpected calloc
      service.print_backtrace();
    };
  osrf_testing_tools_cpp::memory_tools::on_unexpected_calloc(on_unexpected_calloc);

  auto on_unexpected_free =
    [](osrf_testing_tools_cpp::memory_tools::MemoryToolsService & service) {
      if (is_whitelist_stacktrace(service.get_stack_trace())) {
        service.ignore();
        return;
      }

      ADD_FAILURE() << "apex_test_tools::memory_test: Unexpected free";
      // this will cause a bracktrace to be printed for each unexpected free
      service.print_backtrace();
    };
  osrf_testing_tools_cpp::memory_tools::on_unexpected_free(on_unexpected_free);

  osrf_testing_tools_cpp::memory_tools::enable_monitoring();
  apex_test_tools::memory_test::resume();
}

inline void start_paused(void)
{
  apex_test_tools::memory_test::start();
  apex_test_tools::memory_test::pause();
}

inline void stop(void)
{
  apex_test_tools::memory_test::pause();
  (void)osrf_testing_tools_cpp::memory_tools::disable_monitoring_in_all_threads();
  (void)osrf_testing_tools_cpp::memory_tools::uninitialize();
}

}  // namespace memory_test
}  // namespace apex_test_tools

#endif  // APEX_TEST_TOOLS__APEX_TEST_TOOLS_HPP_
