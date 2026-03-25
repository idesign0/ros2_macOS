// Copyright (c) 2025-2026 Carologistics
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CX_UTILS__FORMAT_HPP_
#define CX_UTILS__FORMAT_HPP_
#include <string>
#include <utility>

#if __has_include(<format>) && __cplusplus >= 202002L
#include <format>
namespace cx
{
template <typename... Args>
inline std::string format(std::format_string<Args...> fmt, Args &&... args)
{
  return std::format(fmt, std::forward<Args>(args)...);
}
}  // namespace cx
#elif __has_include(<spdlog/fmt/fmt.h>)
#include <spdlog/fmt/fmt.h>
namespace cx
{
template <typename... Args>
inline std::string format(fmt::format_string<Args...> fmt, Args &&... args)
{
  return fmt::format(fmt, std::forward<Args>(args)...);
}
}  // namespace cx
#elif __has_include(<fmt/format.h>)
#include <fmt/format.h>
namespace cx
{
template <typename... Args>
inline std::string format(fmt::format_string<Args...> fmt, Args &&... args)
{
  return fmt::format(fmt, std::forward<Args>(args)...);
}
}  // namespace cx
#else
#error "No available formatting library found: need std::format or fmt"
#endif

#endif  // CX_UTILS__FORMAT_HPP_
