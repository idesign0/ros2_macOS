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

#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <regex>
#include <set>
#include <utility>
#include <vector>
#include <thread>

#include <gz/transport/Clock.hh>
#include <gz/transport/Discovery.hh>
#include <gz/transport/log/Log.hh>
#include <gz/transport/log/Recorder.hh>
#include <gz/transport/MessageInfo.hh>
#include <gz/transport/Node.hh>
#include <gz/transport/TransportTypes.hh>

#include "Console.hh"
#include "raii-sqlite3.hh"
#include "build_config.hh"

using namespace gz::transport;
using namespace gz::transport::log;

/// \brief Private implementation
class gz::transport::log::Recorder::Implementation
{
  /// \brief Data type stored in dataQueue
  public: struct LogData
  {
    /// \brief Constructor
    LogData(std::chrono::nanoseconds _stamp, std::vector<char> &&_msgData, // NOLINT
            const transport::MessageInfo &_msgInfo)
        : stamp(_stamp), msgData(std::move(_msgData)), msgInfo(_msgInfo)
    {
    }
    /// \brief Time stamp of when the message was received by the log recorder
    std::chrono::nanoseconds stamp;
    /// Serialized message data
    std::vector<char> msgData;
    /// Extra information about the message, such as its topic.
    transport::MessageInfo msgInfo;
  };

  /// \brief constructor
  public: Implementation();

  /// \brief Destructor
  public: ~Implementation();

  /// \brief Subscriber callback
  /// \param[in] _data Data of the message
  /// \param[in] _len The size of the message data
  /// \param[in] _info The meta-info of the message
  public: void OnMessageReceived(
          const char *_data,
          std::size_t _len,
          const transport::MessageInfo &_info);

  /// \brief Callback that listens for newly advertised topics
  /// \param[in] _publisher The Publisher that has advertised
  public: void OnAdvertisement(const Publisher &_publisher);

  /// \sa Recorder::AddTopic(const std::string&)
  public: RecorderError AddTopic(const std::string &_topic);

  /// \sa Recorder::AddTopic(const std::regex&)
  public: int64_t AddTopic(const std::regex &_pattern);

  /// \brief Worker thread function that writes data from the dataQueue to the
  /// database
  public: void DataWriterThread();

  /// \brief Start the data writer thread
  public: void StartDataWriter();

  /// \brief Stop the data writer thread
  public: void StopDataWriter();

  /// \brief Decrement buffer size by given amount
  /// \param[in] _len The amount to decrement
  public: void DecrementBufferSize(std::size_t _len);

  /// \brief Write any data left in the queue to the log file
  public: void FlushDataQueue();

  /// \brief Write data to log file
  /// \param[in] _logData data to be written
  public: void WriteToLogFile(const LogData &_logData);

  /// \brief log file or nullptr if not recording
  public: std::unique_ptr<Log> logFile;

  /// \brief A set of topic patterns that we want to subscribe to
  public: std::vector<std::regex> patterns;

  /// \brief A set of topic names that we have already subscribed to. When new
  /// publishers advertise topics that we are already subscribed to, our
  /// OnAdvertisement callback can just ignore it.
  public: std::set<std::string> alreadySubscribed;

  /// \brief mutex for thread safety when evaluating newly advertised topics
  public: std::mutex topicMutex;

  /// \brief mutex for thread safety with log file
  public: std::mutex logFileMutex;

  /// \brief node used to create subscriptions
  public: Node node;

  /// \brief Clock to synchronize and stamp messages with.
  public: const Clock *clock;

  /// \brief callback used on every subscriber
  public: RawCallback rawCallback;

  /// \brief Object for discovering new publishers as they advertise themselves
  public: std::unique_ptr<MsgDiscovery> discovery;

  /// \brief Maximum size of the buffer (in bytes) that is used to store data
  /// from topic callbacks.
  public: std::atomic<std::size_t> maxBufferSize{1000<<20};

  /// \brief Current size of the buffer (in bytes). This is computed every time
  /// data is added or removed from the queue. Because of that, we'll use
  /// `dataQueueMutex` to protect it.
  public: std::size_t bufferSize{0};

  /// \brief This is a temporary FIFO queue that is used to store data from
  /// callbacks until they are written to disk. If the queue fills up before
  /// the dataWriter thread has a chance to process it, old data will be
  /// overwritten. Thus, it is important to set the queue size appropriately for
  /// your application. The maximum size of this queue is determined by
  /// `maxBufferSize`. The current size of the buffer is calculated from
  /// `msgData`.
  public: std::deque<LogData> dataQueue;

  /// \brief Mutex to synchronize access to dataQueue and bufferSize
  public: std::mutex dataQueueMutex;

  /// \brief Condition variable to synchronize access to dataQueue
  public: std::condition_variable dataQueueCondVar;

  /// \brief Handle to worker thread that writes data from the dataQueue to the
  /// database
  public: std::thread dataWriter;

  /// \brief State of dataWriter thread.
  /// True: Data writer thread has started or is starting.
  /// False: Data writer thread has not started or is shutting down.
  public: std::atomic<bool> dataWriterState{false};

  /// \brief Whether the OnMessageReceived should stop queuing received
  /// messages. This will be set to true when `Recorder::Stop` is called
  public: std::atomic<bool> stopQueue{false};
};

//////////////////////////////////////////////////
Recorder::Implementation::Implementation()
{
  // Use wall clock for synchronization by default.
  this->clock = WallClock::Instance();
  // Make a lambda to wrap a member function callback
  this->rawCallback = [this](
      const char *_data, std::size_t _len, const transport::MessageInfo &_info)
  {
    this->OnMessageReceived(_data, _len, _info);
  };

  auto shared = NodeShared::Instance();

  this->discovery = std::make_unique<MsgDiscovery>(
      Uuid().ToString(), shared->discoveryIP, shared->msgDiscPort);

  DiscoveryCallback<Publisher> cb = [this](const Publisher &_publisher)
  {
    this->OnAdvertisement(_publisher);
  };

  this->discovery->ConnectionsCb(cb);
  this->discovery->Start();
}

//////////////////////////////////////////////////
Recorder::Implementation::~Implementation()
{
  this->StopDataWriter();
}

//////////////////////////////////////////////////
void Recorder::Implementation::OnMessageReceived(
          const char *_data,
          std::size_t _len,
          const MessageInfo &_info)
{
  LDBG("RX'" << _info.Topic() << "'[" << _info.Type() << "]\n");

  if (!this->clock->IsReady()) {
    LWRN("Clock isn't ready yet. Dropping message\n");
  }

  // Don't store anything in the queue unless the data writer has started, which
  // happens when Recorder::Start is called.
  if (this->dataWriterState)
  {
    std::vector<char> tmp(_data, _data+_len);

    std::lock_guard<std::mutex> lock(this->dataQueueMutex);
    // If the maxBufferSize is zero, we have an infinite queue
    if (this->maxBufferSize > 0)
    {
      // Only pop if we have a message in the queue.
      if ((this->bufferSize + _len > this->maxBufferSize) &&
          !this->dataQueue.empty())
      {
        this->DecrementBufferSize(this->dataQueue.front().msgData.size());
        this->dataQueue.pop_front();
      }
    }

    this->bufferSize += _len;
    // If the message being added here is larger than maxBufferSize, it should
    // still be recorded. It just means that the buffer cannot hold another
    // message until it is recorded.
    this->dataQueue.emplace_back(this->clock->Time(), std::move(tmp), _info);
    this->dataQueueCondVar.notify_one();
  }
}

//////////////////////////////////////////////////
void Recorder::Implementation::OnAdvertisement(const Publisher &_publisher)
{
  std::string partition;
  std::string topic;

  TopicUtils::DecomposeFullyQualifiedTopic(
        _publisher.Topic(), partition, topic);

  const std::string &nodePartition = this->node.Options().Partition();

  // If the first character in our node's partition is a slash, then we should
  // start our comparison from index 0 of the incoming publisher's partition.
  // Otherwise, we should start the comparison from index 1. The incoming
  // publisher's partition is guaranteed to begin with the forward slash.
  const std::size_t startCmp = nodePartition[0] == '/' ? 0 : 1;

  // If the advertised partition does not match ours, ignore this advertisement
  if (strcmp(nodePartition.c_str(), partition.c_str() + startCmp) != 0)
    return;

  // If we are already subscribed to the topic, ignore this advertisement
  if (this->alreadySubscribed.find(topic) != this->alreadySubscribed.end())
    return;

  for (const std::regex &pattern : this->patterns)
  {
    if (std::regex_match(topic, pattern))
    {
      this->AddTopic(topic);
    }
  }
}

//////////////////////////////////////////////////
RecorderError Recorder::Implementation::AddTopic(const std::string &_topic)
{
  // Do not subscribe to a topic if we are already subscribed.
  if (this->alreadySubscribed.find(_topic) == this->alreadySubscribed.end())
  {
    LDBG("Recording [" << _topic << "]\n");
    // Subscribe to the topic whether it exists or not
    if (!this->node.SubscribeRaw(_topic, this->rawCallback))
    {
      LERR("Failed to subscribe to [" << _topic << "]\n");
      return RecorderError::FAILED_TO_SUBSCRIBE;
    }
    this->alreadySubscribed.insert(_topic);
    return RecorderError::SUCCESS;
  }

  return RecorderError::ALREADY_SUBSCRIBED_TO_TOPIC;
}

//////////////////////////////////////////////////
int64_t Recorder::Implementation::AddTopic(const std::regex &_pattern)
{
  int numSubscriptions = 0;
  std::vector<std::string> allTopics;
  this->node.TopicList(allTopics);
  for (auto topic : allTopics)
  {
    if (std::regex_match(topic, _pattern))
    {
      // Subscribe to the topic
      if (this->AddTopic(topic) == RecorderError::FAILED_TO_SUBSCRIBE)
      {
        return static_cast<int64_t>(RecorderError::FAILED_TO_SUBSCRIBE);
      }
      ++numSubscriptions;
    }
    else
    {
      LDBG("Not recording " << topic << "\n");
    }
  }

  this->patterns.push_back(_pattern);

  return numSubscriptions;
}

//////////////////////////////////////////////////
void Recorder::Implementation::DataWriterThread()
{
  while (this->dataWriterState)
  {
    std::unique_lock<std::mutex> lock(this->dataQueueMutex);
    if (this->dataQueue.empty())
    {
      this->dataQueueCondVar.wait(lock,
        [this]
        {
          return !this->dataQueue.empty() || !this->dataWriterState;
        });

      if (this->dataQueue.empty())
      {
        continue;
      }
    }

    const auto logData = std::move(this->dataQueue.front());
    this->dataQueue.pop_front();
    // Unlock before locking another mutex.
    lock.unlock();

    this->WriteToLogFile(logData);
  }
}

//////////////////////////////////////////////////
void Recorder::Implementation::StartDataWriter()
{
  this->dataWriterState = true;

  this->dataWriter =
      std::thread(&Recorder::Implementation::DataWriterThread, this);
}

//////////////////////////////////////////////////
void Recorder::Implementation::StopDataWriter()
{
  this->dataWriterState = false;
  this->dataQueueCondVar.notify_one();
  if (this->dataWriter.joinable())
  {
    this->dataWriter.join();
  }
}

//////////////////////////////////////////////////
void Recorder::Implementation::DecrementBufferSize(std::size_t _len)
{
  if (this->bufferSize >= _len)
  {
    this->bufferSize -= _len;
  }
  else
  {
    // This shouldn't happen
    LERR("Buffer size was decremented to a value less than zero. "
        "This should not happen\n");
    this->bufferSize = 0;
  }
}

//////////////////////////////////////////////////
void Recorder::Implementation::FlushDataQueue()
{
  while (true)
  {
    std::unique_lock<std::mutex> lock(this->dataQueueMutex);
    if (this->dataQueue.empty())
      return;

    const auto logData = std::move(this->dataQueue.front());
    this->dataQueue.pop_front();
    // Unlock before locking another mutex.
    lock.unlock();

    this->WriteToLogFile(logData);
  }
}

//////////////////////////////////////////////////
void Recorder::Implementation::WriteToLogFile(const LogData &_logData)
{
  std::lock_guard<std::mutex> logLock(this->logFileMutex);
  // Note: this->logFile will only be a nullptr before Start() has been
  // called or after Stop() has been called. If it is a nullptr, then we are
  // not recording anything yet, so we can just skip inserting the message.
  if (this->logFile &&
      !this->logFile->InsertMessage(
        _logData.stamp, _logData.msgInfo.Topic(), _logData.msgInfo.Type(),
        reinterpret_cast<const void *>(_logData.msgData.data()),
        _logData.msgData.size()))
  {
    LWRN("Failed to insert message into log file\n");
  }
  // TODO(anyone) It would be nice for testing to simulate long delays
  // associated with disk writes. In the mean time, a sleep can be added here
  // for testing.
  // std::this_thread::sleep_for(std::chrono::milliseconds(30));
}

//////////////////////////////////////////////////
Recorder::Recorder()
  : dataPtr(new Implementation)
{
}

//////////////////////////////////////////////////
Recorder::Recorder(Recorder &&_other)  // NOLINT
  : dataPtr(std::move(_other.dataPtr))
{
}

//////////////////////////////////////////////////
Recorder::~Recorder()
{
  if (this->dataPtr)
  {
    this->Stop();
  }
}

//////////////////////////////////////////////////
RecorderError Recorder::Sync(const Clock *_clockIn) {
  if (this->dataPtr->logFile)
  {
    LERR("Recording is already in progress\n");
    return RecorderError::ALREADY_RECORDING;
  }
  this->dataPtr->clock = _clockIn;
  return RecorderError::SUCCESS;
}

//////////////////////////////////////////////////
RecorderError Recorder::Start(const std::string &_file)
{
  std::lock_guard<std::mutex> lock(this->dataPtr->logFileMutex);
  if (this->dataPtr->logFile)
  {
    LWRN("Recording is already in progress\n");
    return RecorderError::ALREADY_RECORDING;
  }

  this->dataPtr->logFile.reset(new Log());
  if (!this->dataPtr->logFile->Open(_file, std::ios_base::out))
  {
    LERR("Failed to open or create file [" << _file << "]\n");
    this->dataPtr->logFile.reset(nullptr);
    return RecorderError::FAILED_TO_OPEN;
  }

  this->dataPtr->StartDataWriter();
  LMSG("Started recording to [" << _file << "]\n");

  return RecorderError::SUCCESS;
}

//////////////////////////////////////////////////
void Recorder::Stop()
{
  {
    std::lock_guard<std::mutex> lock(this->dataPtr->logFileMutex);
    // If the logFile is null, the recorder has already stopped.
    if (nullptr == this->dataPtr->logFile)
      return;
  }
  this->dataPtr->stopQueue = true;
  this->dataPtr->StopDataWriter();
  // If there is any data left in the dataQueue, write it all to disk
  LMSG("Log Recorder finalizing log file. This might take some time...");
  this->dataPtr->FlushDataQueue();
  LMSG("Done\n");

  std::lock_guard<std::mutex> lock(this->dataPtr->logFileMutex);
  this->dataPtr->logFile.reset(nullptr);
}

//////////////////////////////////////////////////
RecorderError Recorder::AddTopic(const std::string &_topic)
{
  return this->dataPtr->AddTopic(_topic);
}

//////////////////////////////////////////////////
int64_t Recorder::AddTopic(const std::regex &_topic)
{
  return this->dataPtr->AddTopic(_topic);
}

//////////////////////////////////////////////////
std::string Recorder::Filename() const
{
  return this->dataPtr->logFile == nullptr ? "" :
         this->dataPtr->logFile->Filename();
}

//////////////////////////////////////////////////
const std::set<std::string> &Recorder::Topics() const
{
  return this->dataPtr->alreadySubscribed;
}

//////////////////////////////////////////////////
std::size_t Recorder::BufferSize() const
{
  // Shift by 20 to convert to MB
  return this->dataPtr->maxBufferSize >> 20;
}

//////////////////////////////////////////////////
void Recorder::SetBufferSize(std::size_t _size)
{
  // Shift by 20 to convert to bytes
  this->dataPtr->maxBufferSize = _size << 20;
}
