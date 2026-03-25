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
#include <atomic>
#include <cstring>
#include <csignal> // NOLINT(*)
#include <functional> // NOLINT(*)
#include <fcntl.h>
#ifndef _WIN32
  #include <unistd.h>
#else
  #include <io.h>
#endif
#include <map> // NOLINT(*)
#include <mutex> // NOLINT(*)
#include <utility> // NOLINT(*)
#include <vector> // NOLINT(*)
#include "gz/common/Console.hh" // NOLINT(*)

using namespace gz;
using namespace common;

// A wrapper for the sigaction sa_handler.
// TODO(azeey) We should avoid using objects with non-trivial destructors as
// globals.
GZ_COMMON_VISIBLE std::map<int, std::function<void(int)>> gOnSignalWrappers;
std::mutex gWrapperMutex;

#ifdef _WIN32
  #define write _write
  #define read _read
#endif
namespace
{

/// \brief Index of the read file descriptor
constexpr int kReadFd = 0;
/// \brief Index of the write file descriptor
constexpr int kWriteFd = 1;

/// \brief Class to encalpsulate the self-pipe trick which is a way enable
/// the user of non async-signal-safe functions in downstream signal handler
/// callbacks.
///
/// It works by creating a pipe between the actual signal handler and
/// a servicing thread. When a signal is received the signal handler
/// writes a byte to the pipe and returns. The servicing thread reads the
/// byte from the pipe and calls all of the registered callbacks. Since
/// the registered callbacks are called from a regular thread instead of
/// an actual signal handler, the callbacks are free to use any function
/// (e.g. call gzdbg).
class SelfPipe {

  /// \brief The two pipes the comprise the self-pipe
  public: static int pipeFd[2];

  /// \brief Static function to create a singleton SelfPipe object
  public: static void Initialize();

  /// \brief Destructor.
  public: ~SelfPipe();

  /// \brief Constructor
  /// Creates the pipes, applies configuration flags and starts the servicing
  /// thread
  private: SelfPipe();

  /// \brief Servicing thread
  private: void CheckPipe();

  /// \brief Handle for CheckPipe thread
  private: std::thread checkPipeThread;

  /// \brief Whether the program is running. This is set to true by the
  /// Constructor and set to false by the destructor
  private: std::atomic<bool> running{false};
};

int SelfPipe::pipeFd[2];

/////////////////////////////////////////////////
/// \brief Callback to execute when a signal is received.
/// This simply writes a byte to a pipe and returns
/// \param[in] _value Signal number.
void onSignalWriteToSelfPipe(int _value)
{
#ifdef _WIN32
  // Windows resets the signal handler every time a signal is handled.
  std::signal(_value, onSignalWriteToSelfPipe);
#endif
  auto valueByte = static_cast<std::uint8_t>(_value);
  if (write(SelfPipe::pipeFd[kWriteFd], &valueByte, 1) == -1)
  {
    // TODO(azeey) Not clear what to do here.
  }
}

/////////////////////////////////////////////////
SelfPipe::SelfPipe()
{
#ifdef _WIN32
  if (_pipe(this->pipeFd, 256, O_BINARY) == -1)
#else
  if (pipe(this->pipeFd) == -1)
#endif
  {
    gzerr << "Unable to create pipe\n";
  }

#ifndef _WIN32
  int flags = fcntl(this->pipeFd[kWriteFd], F_GETFL);
  if (fcntl(this->pipeFd[kWriteFd], F_SETFL, flags | O_NONBLOCK) < 0)
  {
    gzerr << "Failed to set flags on pipe. "
          << "Signal handling may not work properly" << std::endl;
  }
#endif
  this->running = true;
  this->checkPipeThread = std::thread(&SelfPipe::CheckPipe, this);
}

/////////////////////////////////////////////////
SelfPipe::~SelfPipe()
{
  this->running = false;
  // Write a dummy signal value to the pipe. This is not a real signal, but we
  // need to wakeup the CheckPipe thread so it can cleanup properly. The value
  // was chosen to make it clear that this is not one of the standard signals.
  onSignalWriteToSelfPipe(127);
  this->checkPipeThread.join();
}

/////////////////////////////////////////////////
void SelfPipe::Initialize()
{
  // We actually need this object to be destructed in order to join the thread,
  // so we can't use gz::utils::NeverDestroyed here.
  static SelfPipe selfPipe;
}

/////////////////////////////////////////////////
void SelfPipe::CheckPipe()
{
  while (this->running)
  {
    std::uint8_t signal;
    if (read(SelfPipe::pipeFd[kReadFd], &signal, 1) != -1)
    {
      if (this->running)
      {
        std::lock_guard<std::mutex> lock(gWrapperMutex);
        // Send the signal to each wrapper
        for (std::pair<int, std::function<void(int)>> func : gOnSignalWrappers)
        {
          func.second(signal);
        }
      }
    }
    else
    {
      gzerr << errno << " " << std::strerror(errno) << std::endl;
    }
  }
}

}  // namespace

/////////////////////////////////////////////////
class common::SignalHandlerPrivate
{
  /// \brief Handle a signal.
  /// \param[in] _sig Signal number
  public: void OnSignal(int _sig);

  /// \brief the callbacks to execute when a signal is received.
  public: std::vector<std::function<void(int)>> callbacks;

  /// \brief Callback mutex.
  public: std::mutex cbMutex;

  /// \brief True if signal handlers were successfully initialized.
  public: std::atomic<bool> initialized = {false};

  /// \brief Index in the global gOnSignalWrappers map.
  public: int wrapperIndex = -1;
};

/////////////////////////////////////////////////
SignalHandler::SignalHandler()
  : dataPtr(new SignalHandlerPrivate)
{
  static int counter = 0;
  std::lock_guard<std::mutex> lock(gWrapperMutex);

  SelfPipe::Initialize();
  if (std::signal(SIGINT, onSignalWriteToSelfPipe) == SIG_ERR)
  {
    gzerr << "Unable to catch SIGINT.\n"
           << " Please visit http://community.gazebosim.org for help.\n";
    return;
  }

  if (std::signal(SIGTERM, onSignalWriteToSelfPipe) == SIG_ERR)
  {
    gzerr << "Unable to catch SIGTERM.\n"
           << " Please visit http://community.gazebosim.org for help.\n";
    return;
  }

  gOnSignalWrappers[counter] = std::bind(&SignalHandlerPrivate::OnSignal,
      this->dataPtr, std::placeholders::_1);
  this->dataPtr->wrapperIndex = counter;

  ++counter;

  this->dataPtr->initialized = true;
}

/////////////////////////////////////////////////
SignalHandler::~SignalHandler()
{
  std::lock_guard<std::mutex> lock(gWrapperMutex);
  gOnSignalWrappers.erase(this->dataPtr->wrapperIndex);
  delete this->dataPtr;
  this->dataPtr = nullptr;
}

//////////////////////////////////////////////////
bool SignalHandler::Initialized() const
{
  return this->dataPtr->initialized;
}

//////////////////////////////////////////////////
void SignalHandler::SetInitialized(const bool _init)
{
  this->dataPtr->initialized = _init;
}

/////////////////////////////////////////////////
bool SignalHandler::AddCallback(std::function<void(int)> _cb)
{
  bool result = this->dataPtr->initialized;
  if (result)
  {
    std::lock_guard<std::mutex> lock(this->dataPtr->cbMutex);
    this->dataPtr->callbacks.push_back(_cb);
  }
  else
  {
    gzerr << "The SignalHandler was not initialized. Adding a callback will "
      << "have no effect.\n";
  }
  return result;
}

//////////////////////////////////////////////////
void SignalHandlerPrivate::OnSignal(int _sig)
{
  std::lock_guard<std::mutex> lock(this->cbMutex);
  gzdbg << "Received signal[" << _sig << "].\n";
  for (std::function<void(int)> cb : this->callbacks)
    cb(_sig);
}
