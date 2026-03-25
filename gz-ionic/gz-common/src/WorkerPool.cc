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

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

#include "gz/common/WorkerPool.hh"

namespace gz
{
  namespace common
  {
    /// \brief info needed to perform work
    class WorkOrder
    {
      /// \brief Default constructor
      public: WorkOrder() = default;

      /// \brief Constructor used by std::deque::emplace.
      /// \param[in] _work Work function.
      /// \param[in] _cb Callback function.
      public: WorkOrder(const std::function<void()> &_work,
                 const std::function<void()> &_cb)
        : work(_work), callback(_cb) {}

      /// \brief method that does the work
      public: std::function<void()> work = std::function<void()>();

      /// \brief callback to invoke after working
      public: std::function<void()> callback = std::function<void()>();
    };

    /// \brief Private implementation
    class WorkerPool::Implementation
    {
      /// \brief Does work until signaled to shut down
      public: void Worker();

      /// \brief threads that do work
      public: std::vector<std::thread> workers;

      /// \brief queue of work for workers
      public: std::queue<WorkOrder> workOrders;

      /// \brief used to count how many threads are actively working
      public: int activeOrders = 0;

      /// \brief lock for workOrders access
      public: std::mutex queueMtx;

      /// \brief used to signal when all work is done
      public: std::condition_variable signalWorkDone;

      /// \brief used to signal when new work is available
      public: std::condition_variable signalNewWork;

      /// \brief used to signal when the pool is being shut down
      public: bool done = false;
    };

//////////////////////////////////////////////////
void WorkerPool::Implementation::Worker()
{
  WorkOrder order;

  // Run until pool is destructed, waiting for work
  while (true)
  {
    // Scoped to release lock before doing work
    {
      std::unique_lock<std::mutex> queueLock(this->queueMtx);

      // Wait for a work order
      while (!this->done && this->workOrders.empty())
        this->signalNewWork.wait(queueLock);

      // Destructor may have signaled to shutdown workers
      if (this->done)
        break;

      // Take a work order from the queue
      ++(this->activeOrders);
      order = std::move(workOrders.front());
      workOrders.pop();
    }

    // Do the work
    if (order.work)
      order.work();

    if (order.callback)
      order.callback();

    {
      std::unique_lock<std::mutex> queueLock(this->queueMtx);
      --(this->activeOrders);
      if (this->workOrders.empty() && this->activeOrders <= 0)
        this->signalWorkDone.notify_all();
    }
  }
}

//////////////////////////////////////////////////
WorkerPool::WorkerPool(const unsigned int _minThreadCount)
  : dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
  unsigned int numWorkers = std::max(std::thread::hardware_concurrency(),
      std::max(_minThreadCount, 1u));

  // create worker threads
  for (unsigned int w = 0; w < numWorkers; ++w)
  {
    this->dataPtr->workers.push_back(
        std::thread(&WorkerPool::Implementation::Worker, this->dataPtr.get()));
  }
}

//////////////////////////////////////////////////
WorkerPool::~WorkerPool()
{
  // shutdown worker threads
  {
    std::unique_lock<std::mutex> queueLock(this->dataPtr->queueMtx);
    this->dataPtr->done = true;
  }
  this->dataPtr->signalNewWork.notify_all();

  for (auto &t : this->dataPtr->workers)
  {
    t.join();
  }

  // Signal in case anyone is still waiting for work to finish
  this->dataPtr->signalWorkDone.notify_all();
}

//////////////////////////////////////////////////
void WorkerPool::AddWork(std::function<void()> _work, std::function<void()> _cb)
{
  std::unique_lock<std::mutex> queueLock(this->dataPtr->queueMtx);
  this->dataPtr->workOrders.emplace(_work, _cb);
  this->dataPtr->signalNewWork.notify_one();
}

//////////////////////////////////////////////////
bool WorkerPool::WaitForResults(
  const std::chrono::steady_clock::duration &_timeout)
{
  bool signaled = true;
  std::unique_lock<std::mutex> queueLock(this->dataPtr->queueMtx);

  // Lambda to keep logic in one place for both cases
  std::function<bool()> haveResults = [this] () -> bool
    {
      return this->dataPtr->done ||
        (this->dataPtr->workOrders.empty() && !this->dataPtr->activeOrders);
    };

  if (!haveResults())
  {
    if (std::chrono::steady_clock::duration::zero() == _timeout)
    {
      // Wait forever
      this->dataPtr->signalWorkDone.wait(queueLock);
    }
    else
    {
      // Wait for timeout
      signaled = this->dataPtr->signalWorkDone.wait_for(queueLock,
          _timeout,
          haveResults);
    }
  }
  return signaled && !this->dataPtr->done;
}

}
}
