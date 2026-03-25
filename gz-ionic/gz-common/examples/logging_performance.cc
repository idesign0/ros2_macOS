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

/* This code is originally from:
 * https://gist.github.com/KjellKod/0fde0d2564e2eb1b3560
 * For testing worst-case console logging latency with a variable number of
 * threads.
 */

#include <algorithm>
#include <atomic>
#include <map>
#include <thread>
#include <vector>

#include <gz/common/Console.hh>

namespace {
const uint64_t g_iterations{1000000};

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
  //std::cout << content;
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
  gz::common::Console::SetVerbosity(4);
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

  uint64_t total_time_in_us =
      std::chrono::duration_cast<std::chrono::microseconds>(
          stop_time_application_total - start_time_application_total)
          .count();

  PrintStats(filename_result, threads_result, total_time_in_us);
  SaveResultToBucketFile(filename_result, threads_result);
}

/////////////////////////////////////////////////
int main(int argc, char **argv)
{
  size_t number_of_threads {0};
  if (argc == 2) {
    number_of_threads = atoi(argv[1]);
  }
  if (argc != 2 || number_of_threads == 0) {
    std::cerr << "USAGE is: " << argv[0] << " number_threads" << std::endl;
    return 1;
  }

  run(number_of_threads);
}
