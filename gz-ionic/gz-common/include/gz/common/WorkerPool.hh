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

#ifndef GZ_COMMON_WORKER_POOL_HH_
#define GZ_COMMON_WORKER_POOL_HH_

#include <chrono>
#include <functional>

#include <gz/common/Export.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \brief forward declaration
    class WorkerPoolPrivate;

    /// \brief A pool of worker threads that do stuff in parallel
    class GZ_COMMON_VISIBLE WorkerPool
    {
      /// \brief Creates worker threads. The number of worker threads is
      /// determined by max(std::thread::hardware_concurrency, _minThreadCount).
      /// \param[in] _minThreadCount The minimum number of threads to
      /// create in the pool. A value of zero is converted to a value of 1.
      /// \note It's not recommended to set _minThreadCount greater than
      /// std::thread::hardware_concurrency.
      public: explicit WorkerPool(const unsigned int _minThreadCount = 1u);

      /// \brief closes worker threads
      public: ~WorkerPool();

      /// \brief Adds work to the worker pool with optional callback
      /// \param[in] _work function to do one piece of work
      /// \param[in] _cb optional callback when the work is done
      /// \remark Typical work is a function bound with arguments. It must
      //               return within a finite amount of time.
      public: void AddWork(std::function<void()> _work,
                  std::function<void()> _cb = std::function<void()>());

      /// \brief Waits until all work is done and threads are idle
      /// \param[in] _timeout How long to wait, default to forever
      /// \returns true if all work was finished
      /// \remarks The return value can be false even when waiting forever if
      //           the WorkerPool is destructed before all work is completed
      public: bool WaitForResults(
        const std::chrono::steady_clock::duration &_timeout =
          std::chrono::steady_clock::duration::zero());

      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
  }
}

#endif
