/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

// Suppressing cpplint.py because tools/cpplint.py is old. Remove the NOLINT
// comments when upgrading to gz-cmake's "make codecheck"
#include "gz/common/SignalHandler.hh" // NOLINT(*)
#include <gtest/gtest.h> // NOLINT(*)
#include <chrono>
#include <csignal> // NOLINT(*)
#include <condition_variable> // NOLINT(*)
#include <map> // NOLINT(*)
#include <mutex> // NOLINT(*)
#include <thread> // NOLINT(*)
#include "gz/common/Util.hh" // NOLINT(*)

using namespace gz;

// Capture the gOnSignalWrappers map from SignalHandlers.cc
#ifndef _WIN32
extern std::map<int, std::function<void(int)>> gOnSignalWrappers;
#endif

int gHandler1Sig = -1;
int gHandler2Sig = -1;
int gHandler3Sig = -1;
int gHandler4Sig = -1;

/////////////////////////////////////////////////
void handler1Cb(int _sig)
{
  gHandler1Sig = _sig;
}

/////////////////////////////////////////////////
void handler2Cb(int _sig)
{
  gHandler2Sig = _sig;
}

/////////////////////////////////////////////////
void handler3Cb(int _sig)
{
  gHandler3Sig = _sig;
}

/////////////////////////////////////////////////
void handler4Cb(int _sig)
{
  gHandler4Sig = _sig;
}

/////////////////////////////////////////////////
void resetSignals()
{
  gHandler1Sig = -1;
  gHandler2Sig = -1;
  gHandler3Sig = -1;
  gHandler4Sig = -1;
}

/////////////////////////////////////////////////
TEST(SignalHandler, Single)
{
  resetSignals();

  common::SignalHandler handler1;
  EXPECT_TRUE(handler1.AddCallback(handler1Cb));
  std::raise(SIGTERM);
  std::this_thread::sleep_for(std::chrono::milliseconds(11));
  EXPECT_EQ(SIGTERM, gHandler1Sig);
}

/////////////////////////////////////////////////
TEST(SignalHandler, Multiple)
{
  resetSignals();

  common::SignalHandler handler1;
  common::SignalHandler handler2;

  EXPECT_TRUE(handler1.Initialized());
  EXPECT_TRUE(handler2.Initialized());

  EXPECT_TRUE(handler1.AddCallback(handler3Cb));
  EXPECT_TRUE(handler2.AddCallback(handler4Cb));

  std::raise(SIGINT);

  std::this_thread::sleep_for(std::chrono::milliseconds(11));
  EXPECT_EQ(-1, gHandler1Sig);
  EXPECT_EQ(-1, gHandler2Sig);

  EXPECT_EQ(SIGINT, gHandler3Sig);
  EXPECT_EQ(SIGINT, gHandler4Sig);
}


/////////////////////////////////////////////////
TEST(SignalHandler, InitFailure)
{
  resetSignals();

  class NotInitializedSignalHandler : public common::SignalHandler
  {
    /// \brief Constructor
    public: NotInitializedSignalHandler()
            : common::SignalHandler()
    {
      this->SetInitialized(false);
    }
  };

  NotInitializedSignalHandler badSigHandler;
  EXPECT_FALSE(badSigHandler.Initialized());
  EXPECT_FALSE(badSigHandler.AddCallback(handler1Cb));

  std::raise(SIGINT);

  std::this_thread::sleep_for(std::chrono::milliseconds(11));
  EXPECT_EQ(-1, gHandler1Sig);
  EXPECT_EQ(-1, gHandler2Sig);
}

/////////////////////////////////////////////////
TEST(SignalHandler, Thread)
{
  resetSignals();

  std::mutex mutex;
  std::condition_variable cv;

  // Create a lock, which will allow us to wait for the thread to create
  // its signal handler
  std::unique_lock<std::mutex> outerLock(mutex);

  // Create the thread. Inside the thread, a new signal handler is created.
  std::thread thread1([&mutex, &cv] ()
  {
    common::SignalHandler handler;
    EXPECT_TRUE(handler.Initialized());

    // This signal handler will unblock the thread, allowing it to exit.
    EXPECT_TRUE(handler.AddCallback([&cv] (int _sig)
    {
      gHandler1Sig = _sig;
      cv.notify_all();
    }));

    std::unique_lock<std::mutex> threadLock(mutex);
    // Unblock the test.
    cv.notify_all();

    // Wait for this thread's signal handler.
    cv.wait(threadLock);
  });

  // Create a signal handler outside the thread.
  common::SignalHandler handler;
  EXPECT_TRUE(handler.Initialized());
  EXPECT_TRUE(handler.AddCallback([] (int _sig)
  {
    gHandler2Sig = _sig;
  }));

  // Wait for the thread to finish creating its signal handler
  cv.wait(outerLock);

  // Raise the signal and join the thread.
  std::raise(SIGINT);
  outerLock.unlock();
  thread1.join();

  EXPECT_EQ(SIGINT, gHandler1Sig);
  EXPECT_EQ(SIGINT, gHandler2Sig);
}

/////////////////////////////////////////////////
TEST(SignalHandler, MultipleThreads)
{
  resetSignals();

  // Create a lock, which will allow us to wait for the thread to create
  // its signal handler
  std::mutex outerMutex;
  std::unique_lock<std::mutex> outerLock(outerMutex);
  std::condition_variable outerCv;

  std::vector<std::thread*> threads;
  std::vector<int> results;

  // A lot of threads
#if _WIN64 || __amd64__
  int threadCount = 1000;
#else
  int threadCount = std::thread::hardware_concurrency();
#endif

  // Create all the threads.
  for (int i = 0; i < threadCount; ++i)
  {
    // Create the thread. Inside the thread, a new signal handler is created.
    std::thread *thread = new std::thread(
        [&outerMutex, &outerCv, &results, threadCount] ()
    {
      common::SignalHandler handler;
      EXPECT_TRUE(handler.Initialized());

      int index = -1;
      std::mutex localMutex;
      std::condition_variable localCv;

      // This signal handler will unblock the thread, allowing it to exit.
      EXPECT_TRUE(handler.AddCallback(
            [&results, &index, &localMutex, &localCv] (int _sig)
      {
        std::unique_lock<std::mutex> localLock(localMutex);
        ASSERT_LT(index, static_cast<int>(results.size()));
        results[index] = _sig;
        localCv.notify_all();
      }));

      // Create a new index and result entry one-by-one
      outerMutex.lock();
      index = results.size();
      EXPECT_LT(-1, index);
      EXPECT_GT(threadCount, index);
      results.push_back(0);

      std::unique_lock<std::mutex> localLock(localMutex);
      // Unblock the outer test when all the results have been created.
      if (static_cast<int>(results.size()) >= threadCount)
        outerCv.notify_all();

      outerMutex.unlock();

      // Wait for this thread's signal handler.
      localCv.wait(localLock);
    });

    // Store the threads
    threads.push_back(thread);
  }

  // Wait for all the threads to finish their creation process.
  outerCv.wait(outerLock);

  EXPECT_EQ(threadCount, static_cast<int>(results.size()));
  EXPECT_EQ(threadCount, static_cast<int>(threads.size()));

#ifndef _WIN32
  EXPECT_EQ(threadCount, static_cast<int>(gOnSignalWrappers.size()));

  // Check that all the indices in gOnSignalWrappers are increasing by one.
  int index = gOnSignalWrappers.begin()->first;
  for (std::pair<int, std::function<void(int)>> func : gOnSignalWrappers)
  {
    EXPECT_EQ(index, func.first);
    ++index;
  }
#endif

  // Raise the signal and join the thread.
  std::raise(SIGINT);

  // Join all threads and cleanup
  for (std::thread *thread : threads)
  {
    if (thread && thread->joinable())
    {
      thread->join();
      delete thread;
      thread = nullptr;
    }
  }

  // Check the results
  for (int i = 0; i < threadCount; ++i)
    EXPECT_EQ(SIGINT, results[i]);
}

/////////////////////////////////////////////////
TEST(SignalHandler, RapidFire)
{
  resetSignals();
  std::condition_variable cv;
  std::mutex countMutex;
  int countHandlerCalls = 0;
  constexpr int kNumSignals = 100;
  auto cb = [&](int _sig)
  {
    if (_sig == SIGTERM)
    {
      std::lock_guard<std::mutex> lk(countMutex);
      ++countHandlerCalls;
      if (countHandlerCalls >= kNumSignals)
      {
        cv.notify_one();
      }
    }
  };
  common::SignalHandler handler1;
  EXPECT_TRUE(handler1.AddCallback(cb));

  for (int i=0; i < kNumSignals; ++i)
  {
    std::raise(SIGTERM);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }

  // wait for callback to be called kNumSignal times with a timeout
  std::unique_lock<std::mutex> lk(countMutex);
  cv.wait_for(lk, std::chrono::seconds(5),
              [&] { return countHandlerCalls >= kNumSignals; });
  EXPECT_GE(countHandlerCalls, kNumSignals);
}
