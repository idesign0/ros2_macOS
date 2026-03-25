// Copyright (c) 2024-2026 Carologistics
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

#ifndef CX_UTILS__STRING_UTILS_HPP_
#define CX_UTILS__STRING_UTILS_HPP_

#include <algorithm>
#include <iostream>
#include <random>
#include <string>

using std::string;

namespace cx
{

inline constexpr std::string_view chars =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-";

std::random_device rd;
std::mt19937 generator(rd());

std::string generate_rnd_string(int length)
{
  std::string output(chars);
  std::shuffle(output.begin(), output.end(), generator);
  return output.substr(0, length);
}

}  // namespace cx
#endif  // CX_UTILS__STRING_UTILS_HPP_
