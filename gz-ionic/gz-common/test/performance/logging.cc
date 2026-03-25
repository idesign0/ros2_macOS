/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <map>
#include <thread>

#include <gz/common/Console.hh>
#include <gz/common/testing/TestPaths.hh>
#include <spdlog/sinks/basic_file_sink.h>

using namespace gz;

namespace {
// Lower value than spdlog to keep CI from flaking
const uint64_t g_iterations{10000};

std::atomic<size_t> g_counter = {0};

void WriteToFile(std::string result_filename, std::string content)
{
  std::ofstream out;
  std::ios_base::openmode mode = std::ios_base::out | std::ios_base::app;
  out.open(result_filename.c_str(), mode);
  if (!out.is_open())
  {
    std::cerr << "Error writing to " << result_filename << std::endl;
  }
  out << content << std::flush;
}

void MeasurePeakDuringLogWrites(const size_t id, std::vector<uint64_t> &result)
{
  while (true)
  {
    const size_t value_now = ++g_counter;
    if (value_now > g_iterations)
    {
      return;
    }
    std::stringstream ss;
    ss << "Some text to log for thread: " << id << "\n";
    auto start_time = std::chrono::high_resolution_clock::now();
    gzmsg << ss.str();
    auto stop_time = std::chrono::high_resolution_clock::now();
    uint64_t time_us = std::chrono::duration_cast<std::chrono::microseconds>(
                           stop_time - start_time)
                           .count();
    result.push_back(time_us);
  }
}

void PrintStats(const std::string &filename,
                const std::map<size_t, std::vector<uint64_t>> &threads_result,
                const uint64_t total_time_in_us)
{
  size_t idx = 0;
  std::ostringstream oss;
  for (auto t_result : threads_result)
  {
    if (t_result.second.empty())
    {
      oss << idx++ << " no logs were recorded for this thread" << std::endl;
      continue;
    }
    uint64_t worstUs =
        (*std::max_element(t_result.second.begin(), t_result.second.end()));
    oss << idx++ << " the worst thread latency was:" << worstUs / uint64_t(1000)
        << " ms  (" << worstUs << " us)] " << std::endl;
  }

  oss << "Total time :" << total_time_in_us / uint64_t(1000) << " ms ("
      << total_time_in_us << " us)" << std::endl;
  oss << "Average time: " << double(total_time_in_us) / double(g_iterations)
      << " us" << std::endl;
  WriteToFile(filename, oss.str());
}

void SaveResultToBucketFile(
    std::string result_filename,
    const std::map<size_t, std::vector<uint64_t>> &threads_result)
{
  // now split the result in buckets of 1ms each so that it's obvious how the
  // peaks go
  std::vector<uint64_t> all_measurements;
  all_measurements.reserve(g_iterations);
  for (auto &t_result : threads_result)
  {
    all_measurements.insert(all_measurements.end(), t_result.second.begin(),
                            t_result.second.end());
  }

  std::map<uint64_t, uint64_t> bucketsWithEmpty;
  std::map<uint64_t, uint64_t> buckets;

  if (all_measurements.empty())
  {
    return;
  }

  // force empty buckets to appear
  auto maxValueIterator =
      std::max_element(all_measurements.begin(), all_measurements.end());
  const auto kMaxValue = *maxValueIterator;

  for (uint64_t idx = 0; idx <= kMaxValue; ++idx)
  {
    bucketsWithEmpty[idx] = 0;
  }

  for (auto value : all_measurements)
  {
    ++bucketsWithEmpty[value];
    ++buckets[value];
  }

  /*
  std::cout << "\n\n Microsecond bucket measurement" << std::endl;
  for (const auto ms_bucket : buckets) {
     std::cout << ms_bucket.first << "\t, " << ms_bucket.second << std::endl;
  }
  std::cout << "\n\n";
  */

  std::ostringstream oss;
  // Save to xcel and google doc parsable format. with empty buckets
  oss << "\n\n Microsecond bucket measurement with zero buckets till max"
      << std::endl;
  for (const auto ms_bucket : bucketsWithEmpty)
  {
    oss << ms_bucket.first << "\t, " << ms_bucket.second << std::endl;
  }
  WriteToFile(result_filename, oss.str());
  std::cout << "Worst Case Latency, max value: " << kMaxValue << std::endl;
  std::cout << "microsecond bucket result is in file: " << result_filename
            << std::endl;
}
}  // namespace

void run(size_t number_of_threads)
{
  g_counter = 0;
  common::Console::SetVerbosity(4);
  std::vector<std::thread> threads(number_of_threads);
  std::map<size_t, std::vector<uint64_t>> threads_result;

  for (size_t idx = 0; idx < number_of_threads; ++idx)
  {
    // reserve to 1 million for all the result
    // it's a test so  let's not care about the wasted space
    threads_result[idx].reserve(g_iterations);
  }

  // int queue_size = 1048576; // 2 ^ 20
  // int queue_size = 524288;  // 2 ^ 19
  // spdlog::set_async_mode(queue_size); // default size is 1048576
  auto filename_result = std::to_string(number_of_threads) + ".result.csv";
  std::ostringstream oss;
  oss << "Using " << number_of_threads;
  oss << " to log in total " << g_iterations << " log entries to "
      << filename_result << std::endl;
  WriteToFile(filename_result, oss.str());

  // We use a direct spdlog file sink instead of RedirectStdout.
  // RedirectStdout relies on dup2, which is unstable on Windows when
  // combined with high-concurrency writes, leading to SEH exceptions.
  // Using a file sink preserves I/O testing while ensuring stability.
  auto &logger = gz::common::Console::Root().RawLogger();
  auto original_sinks = logger.sinks();
  logger.sinks().clear();
  std::string logPath = gz::common::testing::TempPath("perf_test.log");
  logger.sinks().push_back(
      std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath));

  auto start_time_application_total =
  std::chrono::high_resolution_clock::now();
  for (uint64_t idx = 0; idx < number_of_threads; ++idx)
  {
    threads[idx] = std::thread(MeasurePeakDuringLogWrites, idx,
                               std::ref(threads_result[idx]));
  }
  for (size_t idx = 0; idx < number_of_threads; ++idx)
  {
    threads[idx].join();
  }
  auto stop_time_application_total = std::chrono::high_resolution_clock::now();

  // Restore original sinks
  logger.sinks() = original_sinks;

  uint64_t total_time_in_us =
      std::chrono::duration_cast<std::chrono::microseconds>(
          stop_time_application_total - start_time_application_total)
          .count();

  PrintStats(filename_result, threads_result, total_time_in_us);
  SaveResultToBucketFile(filename_result, threads_result);
}

class LoggingTest:
      public ::testing::TestWithParam<std::size_t>
{
};

TEST_P(LoggingTest, RunThreads)
{
  run(GetParam());
  SUCCEED();
}

INSTANTIATE_TEST_SUITE_P(LoggingTest, LoggingTest,
                         ::testing::Values(1, 2, 4, 8, 16, 32));

/////////////////////////////////////////////////
// This test is valid (passes) if it runs without segfaults.
int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
