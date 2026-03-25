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

#include <sqlite3.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include <gz/transport/Node.hh>
#include <gz/transport/log/Log.hh>
#include <gz/transport/log/Playback.hh>
#include "Console.hh"
#include "build_config.hh"
#include "raii-sqlite3.hh"

using namespace gz::transport;
using namespace gz::transport::log;

// We check whether sqlite3 is potentially threadsafe. Note that this only
// knows whether sqlite3 was compiled with multi-threading capabilities. It
// might not catch changes to sqlite3's runtime settings.
// See: https://www.sqlite.org/threadsafe.html
static const bool kSqlite3Threadsafe = (sqlite3_threadsafe() != 0);

//////////////////////////////////////////////////
/// \brief Private implementation of Playback
class gz::transport::log::Playback::Implementation
{
  /// \brief Constructor. Creates and initializes the log file
  /// \param[in] _file The full path of the file to open
  public: Implementation(
    const std::string &_file, const NodeOptions &_nodeOptions)
    : logFile(std::make_shared<Log>()),
      addTopicWasUsed(false),
      nodeOptions(_nodeOptions)
  {
    if (!this->logFile->Open(_file, std::ios_base::in))
    {
      LERR("Could not open file [" << _file << "]\n");
    }
    else
    {
      LDBG("Playback opened file [" << _file << "]\n");
    }
  }

  /// \brief This gets used by RemoveTopic(~) to make sure we follow the correct
  /// behavior.
  void DefaultToAllTopics()
  {
    if (!this->addTopicWasUsed)
    {
      const Descriptor *desc = this->logFile->Descriptor();
      const Descriptor::NameToMap &allTopics = desc->TopicsToMsgTypesToId();
      for (const auto &entry : allTopics)
        this->topicNames.insert(entry.first);

      // Topics have been set, so we change this flag to true
      this->addTopicWasUsed = true;
    }
  }

  /// \brief log file to play from
  public: std::shared_ptr<Log> logFile;

  /// \brief topics that are being played back
  public: std::unordered_set<std::string> topicNames;

  /// \brief True if a call to either overload of AddTopic() has been made.
  public: bool addTopicWasUsed;

  /// \brief This is the last handle that was produced by the Playback object.
  /// This is only used to ensure safety in special cases where multi-threaded
  /// sqlite3 is known to be unavailable.
  public: std::weak_ptr<PlaybackHandle> lastHandle;

  /// \brief The node options.
  public: NodeOptions nodeOptions;
};

//////////////////////////////////////////////////
/// \brief Private implementation of PlaybackHandle
class PlaybackHandle::Implementation
{
  /// \brief Constructor
  /// \param[in] _logFile A reference to the Log instance
  /// \param[in] _topics A set of all topics to publish
  /// \param[in] _waitAfterAdvertising How long to wait after advertising the
  /// topics
  /// \param[in] _msgWaiting True to wait between publication of
  /// messages based on the message timestamps. False to playback
  /// messages as fast as possible. Default value is true.
  public: Implementation(
      const std::shared_ptr<Log> &_logFile,
      const std::unordered_set<std::string> &_topics,
      const std::chrono::nanoseconds &_waitAfterAdvertising,
      const NodeOptions &_nodeOptions,
      bool _msgWaiting);

  /// \brief Look through the types of data that _topic can publish and create
  /// a publisher for each type.
  /// \param[in] _topic Name of the topic to publish
  public: void AddTopic(const std::string &_topic);

  /// \brief Create a publisher of a given topic name and type
  /// \param[in] _topic Topic name to publish to
  /// \param[in] _type The message type name to publish
  public: void CreatePublisher(
      const std::string &_topic,
      const std::string &_type);

  /// \brief Begin playing messages in another thread
  public: void StartPlayback();

  /// \brief Stop the playback
  public: void Stop();

  /// \brief Step the playback by a given amount of nanoseconds
  /// \pre Playback must be previously paused
  /// \param[in] _stepDuration Length of the step in nanoseconds
  public: void Step(const std::chrono::nanoseconds &_stepDuration);

  /// \brief Jump current playback time to a specific elapsed time
  /// \param[in] _newElapsedTime Elapsed time at which playback will jump
  public: void Seek(const std::chrono::nanoseconds &_newElapsedTime);

  /// \brief Puts the calling thread to sleep until a given time is achieved.
  /// \param[in] _targetTime Time at which the wait must finish. Measured in
  /// POSIX time (time since epoch) in nanoseconds
  /// \return True if the wait ends successfully or false if a pause or
  /// stop event interrupt it
  public: bool WaitUntil(const std::chrono::nanoseconds &_targetTime);

  /// \brief Pauses the playback
  public: void Pause();

  /// \brief Unpauses the playback
  public: void Resume();

  /// \brief Check pause status
  public: bool IsPaused() const;

  /// \brief Wait until playback has finished playing
  public: void WaitUntilFinished();

  /// \brief node used to create publishers
  /// \note This member needs to come before the publishers member so that they
  /// get destructed in the correct order
  public: std::unique_ptr<gz::transport::Node> node;

  /// \brief Map whose key is a topic name and value is another map whose
  /// key is a message type name and value is a publisher
  public: std::unordered_map<std::string,
          std::unordered_map<std::string,
            gz::transport::Node::Publisher>> publishers;

  /// \brief a mutex to use when waiting for playback to finish
  public: std::mutex waitMutex;

  /// \brief Condition variable to wake up threads waiting on playback
  public: std::condition_variable waitConditionVariable;

  /// \brief Condition variable to wake up the playback thread if it's waiting
  /// to publish the next message.
  public: std::condition_variable stopConditionVariable;

  /// \brief True if the thread should be stopped
  public: std::atomic_bool stop;

  /// \brief This will be switched to true as the playback thread exits. This is
  /// different from `stop` because it cannot be changed by the user; it only
  /// changes exactly when the playback thread exits.
  public: std::atomic_bool finished;

  /// \brief True if the thread should be paused
  public: std::atomic_bool paused;

  // \brief Start time in the playback frame
  public: std::chrono::nanoseconds playbackStartTime;

  // \brief End time in the playback frame
  public: std::chrono::nanoseconds playbackEndTime;

  // \brief Current time in the playback frame
  public: std::chrono::nanoseconds playbackTime;

  // \brief Time until next step in the playback frame
  public: std::chrono::nanoseconds boundaryTime;

  // \brief Time read from the next message to playback
  public: std::chrono::nanoseconds nextMessageTime;

  // \brief Time at which occurred the last event in the realtime frame
  public: std::chrono::nanoseconds lastEventTime;

  /// \brief A mutex to use when waiting for playback to be resumed
  public: std::mutex pauseMutex;

  /// \brief Condition variable to wake up the playback thread if it's waiting
  /// to get out of a pause state
  public: std::condition_variable pauseConditionVariable;

  /// \brief thread running playback
  public: std::thread playbackThread;

  /// \brief log file to play from
  public: std::shared_ptr<Log> logFile;

  /// \brief List of topics currently tracked
  public: const std::unordered_set<std::string> trackedTopics;

  /// \brief mutex for thread safety with log file
  public: std::mutex logFileMutex;

  // \brief Set of messages to be played-back
  public: Batch batch;

  // \brief Mutex to operate the batch variable in a thread-safe way
  public: std::mutex batchMutex;

  // \brief Iterator to loop over the messages found in batch
  public: Batch::iterator messageIter;

  // \brief The wall clock time of the first message in batch
  public: const std::chrono::nanoseconds firstMessageTime;

  /// \brief True to wait between publication of
  /// messages based on the message timestamps. False to playback
  /// messages as fast as possible.
  public: bool msgWaiting = true;
};

//////////////////////////////////////////////////
Playback::Playback(const std::string &_file, const NodeOptions &_nodeOptions)
  : dataPtr(new Implementation(_file, _nodeOptions))
{
  // Do nothing
}

//////////////////////////////////////////////////
Playback::Playback(Playback &&_other)  // NOLINT
  : dataPtr(std::move(_other.dataPtr))
{
  // Do nothing
}

//////////////////////////////////////////////////
Playback::~Playback()
{
  // Do nothing
}

//////////////////////////////////////////////////
PlaybackHandlePtr Playback::Start(
    const std::chrono::nanoseconds &_waitAfterAdvertising,
    bool _msgWaiting) const
{
  if (!this->dataPtr->logFile->Valid())
  {
    LERR("Could not start: Failed to open log file\n");
    return nullptr;
  }

  if (!kSqlite3Threadsafe)
  {
    // If we know that threadsafety is not available, then we will insist on
    // not creating a new PlaybackHandle until the last one is finished.
    PlaybackHandlePtr lastHandle = this->dataPtr->lastHandle.lock();
    if (lastHandle && !lastHandle->Finished())
    {
      LWRN("You have linked to a single-threaded sqlite3. We can only spawn "
           "one PlaybackHandle at a time\n");
      return nullptr;
    }
  }

  std::unordered_set<std::string> topics;
  if (!this->dataPtr->addTopicWasUsed)
  {
    LDBG("No topics added, defaulting to all topics\n");
    const Descriptor *desc = this->dataPtr->logFile->Descriptor();
    const Descriptor::NameToMap &allTopics = desc->TopicsToMsgTypesToId();
    for (const auto &entry : allTopics)
      topics.insert(entry.first);
  }
  else
  {
    topics = this->dataPtr->topicNames;
  }

  PlaybackHandlePtr newHandle(
        new PlaybackHandle(
          std::make_unique<PlaybackHandle::Implementation>(
            this->dataPtr->logFile, topics, _waitAfterAdvertising,
            this->dataPtr->nodeOptions, _msgWaiting)));

  // We only need to store this if sqlite3 was not compiled in threadsafe mode.
  if (!kSqlite3Threadsafe)
    this->dataPtr->lastHandle = newHandle;

  return newHandle;
}

//////////////////////////////////////////////////
bool Playback::Valid() const
{
  return this->dataPtr->logFile->Valid();
}

//////////////////////////////////////////////////
bool Playback::AddTopic(const std::string &_topic)
{
  // We set this to true whether or not the function call succeeds, because by
  // calling this function, the user has expressed an intention to explicitly
  // specify which topics to publish.
  this->dataPtr->addTopicWasUsed = true;

  if (!this->dataPtr->logFile->Valid())
  {
    LERR("Failed to open log file\n");
    return false;
  }

  const Descriptor *desc = this->dataPtr->logFile->Descriptor();
  const Descriptor::NameToMap &allTopics = desc->TopicsToMsgTypesToId();

  const Descriptor::NameToMap::const_iterator it = allTopics.find(_topic);
  if (it == allTopics.end())
  {
    LWRN("Topic [" << _topic << "] is not in the log\n");
    return false;
  }

  this->dataPtr->topicNames.insert(_topic);

  return true;
}

//////////////////////////////////////////////////
int64_t Playback::AddTopic(const std::regex &_topic)
{
  // We set this to true whether or not the function call succeeds, because by
  // calling this function, the user has expressed an intention to explicitly
  // specify which topics to publish.
  this->dataPtr->addTopicWasUsed = true;

  if (!this->dataPtr->logFile->Valid())
  {
    LERR("Failed to open log file\n");
    return -1;
  }

  int64_t numMatches = 0;
  const Descriptor *desc = this->dataPtr->logFile->Descriptor();
  const Descriptor::NameToMap &allTopics = desc->TopicsToMsgTypesToId();

  for (const auto &topicEntry : allTopics)
  {
    const std::string &topic = topicEntry.first;
    if (!std::regex_match(topic, _topic))
      continue;

    this->dataPtr->topicNames.insert(topic);
    ++numMatches;
  }
  return numMatches;
}

//////////////////////////////////////////////////
bool Playback::RemoveTopic(const std::string &_topic)
{
  this->dataPtr->DefaultToAllTopics();

  return (this->dataPtr->topicNames.erase(_topic) > 0);
}

//////////////////////////////////////////////////
int64_t Playback::RemoveTopic(const std::regex &_topic)
{
  this->dataPtr->DefaultToAllTopics();

  uint64_t count = 0;
  std::unordered_set<std::string>::iterator it =
      this->dataPtr->topicNames.begin();

  while (it != this->dataPtr->topicNames.end())
  {
    if (std::regex_match(*it, _topic))
    {
      it = this->dataPtr->topicNames.erase(it);
      ++count;
    }
    else
    {
      ++it;
    }
  }

  return count;
}

//////////////////////////////////////////////////
PlaybackHandle::Implementation::Implementation(
    const std::shared_ptr<Log> &_logFile,
    const std::unordered_set<std::string> &_topics,
    const std::chrono::nanoseconds &_waitAfterAdvertising,
    const NodeOptions &_nodeOptions,
    bool _msgWaiting)
  : stop(true),
    finished(false),
    paused(false),
    logFile(_logFile),
    trackedTopics(_topics),
    batch(logFile->QueryMessages(TopicList::Create(_topics))),
    messageIter(batch.begin()),
    firstMessageTime(messageIter->TimeReceived()),
    msgWaiting(_msgWaiting)
{
  this->node.reset(new transport::Node(_nodeOptions));

  for (const std::string &topic : _topics)
  {
    this->AddTopic(topic);
  }

  std::this_thread::sleep_for(_waitAfterAdvertising);

  if (this->batch.begin() == this->batch.end())
  {
    LWRN("There are no messages to play\n");
  }

  this->StartPlayback();
}

//////////////////////////////////////////////////
void PlaybackHandle::Implementation::AddTopic(
    const std::string &_topic)
{
  const Descriptor *desc = this->logFile->Descriptor();
  const Descriptor::NameToMap &allTopics = desc->TopicsToMsgTypesToId();

  const Descriptor::NameToMap::const_iterator it = allTopics.find(_topic);
  for (const auto &typeEntry : it->second)
  {
    const std::string &type = typeEntry.first;
    LDBG("Playing back [" << _topic << "] : [" << type << "]\n");
    this->CreatePublisher(_topic, type);
  }
}

//////////////////////////////////////////////////
void PlaybackHandle::Implementation::CreatePublisher(
    const std::string &_topic,
    const std::string &_type)
{
  auto firstMapIter = this->publishers.find(_topic);
  if (firstMapIter == this->publishers.end())
  {
    // Create a map for the message topic
    this->publishers[_topic] = std::unordered_map<std::string,
      Node::Publisher>();
    firstMapIter = this->publishers.find(_topic);
  }

  auto secondMapIter = firstMapIter->second.find(_type);
  if (secondMapIter != firstMapIter->second.end())
  {
    // A publisher which matches this (topic, type) pair has already been
    // created, so we stop here.
    return;
  }

  // Create a publisher for the topic and type combo
  firstMapIter->second[_type] = this->node->Advertise(_topic, _type);
  LDBG("Creating publisher for " << _topic << " " << _type << "\n");
}

//////////////////////////////////////////////////
void PlaybackHandle::Implementation::WaitUntilFinished()
{
  if (this->logFile->Valid() && !this->stop)
  {
    std::unique_lock<std::mutex> lk(this->waitMutex);
    this->waitConditionVariable.wait(lk, [this]{return this->finished.load();});
  }
}


//////////////////////////////////////////////////
void PlaybackHandle::Implementation::StartPlayback()
{
  this->stop = false;

  // Set time boundary to infinite, which will get overwritten on a step request
  this->boundaryTime = std::chrono::nanoseconds::max();

  // Set time in the playback frame equal to the first message in batch
  // so that it gets played back right after playback starts
  this->playbackStartTime = this->logFile->StartTime();
  this->playbackTime = this->playbackStartTime;
  this->playbackEndTime = this->logFile->EndTime();

  this->nextMessageTime = this->messageIter->TimeReceived();

  this->lastEventTime = std::chrono::steady_clock::now().time_since_epoch();

  this->playbackThread = std::thread([this] () mutable
    {
      while (!this->stop && (this->messageIter != this->batch.end())) {
        // Lock if paused
        if (this->paused)
        {
          std::unique_lock<std::mutex> lk(this->pauseMutex);
          // If paused, the thread will be blocked here
          this->pauseConditionVariable.wait(lk,
            [this]{return !this->paused.load();});
          this->lastEventTime =
              std::chrono::steady_clock::now().time_since_epoch();
          // Abort current iteration after coming back from pause
          continue;
        }
        // If not executing a requested step (regular non-paused playback flow)
        if (this->nextMessageTime <= this->boundaryTime)
        {
          // The timeDelta becomes the time remaining until next message
          const std::chrono::nanoseconds timeDelta(
              this->nextMessageTime - this->playbackTime);
          const std::chrono::nanoseconds timeToWaitUntil(
              this->lastEventTime + timeDelta);
          // Wait until target time is reached or playback is stopped/paused
          // In the latter case, break the iteration step
          if (this->msgWaiting && !this->WaitUntil(timeToWaitUntil))
          {
            continue;
          }
          // Publish the message
          {
          std::unique_lock<std::mutex> lk(this->batchMutex);
          LDBG("publishing\n");
          this->publishers[
            this->messageIter->Topic()][
              this->messageIter->Type()].PublishRaw(
                this->messageIter->Data(), this->messageIter->Type());
          // Advance iterator to next message
          ++this->messageIter;
          this->playbackTime = this->nextMessageTime;
          this->lastEventTime =
              std::chrono::steady_clock::now().time_since_epoch();
          this->nextMessageTime = messageIter->TimeReceived();
          }
        }
        // If a custom step has been requested, always from a paused state,
        // playback gets resumed until the step requested is completed,
        // then goes back to paused.
        else
        {
          // The timeDelta is equal to the step size passed to the step function
          const std::chrono::nanoseconds timeDelta(
              this->boundaryTime - this->playbackTime);
          // Target time in the realtime frame
          const std::chrono::nanoseconds timeToWaitUntil(
              this->lastEventTime + timeDelta);
          // Wait until target time is reached or playback is stopped/paused
          // In the latter case, break the iteration step
          if (!this->WaitUntil(timeToWaitUntil))
          {
            continue;
          }
          this->Pause();
        }
      }
      this->finished = true;
      this->waitConditionVariable.notify_all();
  });
}

//////////////////////////////////////////////////
bool PlaybackHandle::Implementation::WaitUntil(
    const std::chrono::nanoseconds &_targetTime)
{
  const auto waitStartTime =
    std::chrono::steady_clock::now().time_since_epoch();

  // Lambda used as predicate below to check for spurious wake-ups
  auto FinishedWaiting = [this, &_targetTime]() -> bool
  {
    const auto now =
      std::chrono::steady_clock::now().time_since_epoch();
    return _targetTime <= now || this->stop || this->paused;
  };

  // Passing a lock to wait_for is just a formality (we don't actually
  // want to unlock any mutex while waiting), so we create a temporary
  // mutex and lock to satisfy the function.
  //
  // According to the C++11 standard, it is undefined behavior to pass
  // different mutexes into the condition_variable::wait_for()
  // function of the same condition_variable instance from different
  // threads. However, this current thread should be the only thread
  // that is ever waiting on stopConditionVariable because we are
  // keeping playbackLock locked this whole time. Therefore, it's okay
  // for it lock its own local, unique mutex.
  //
  // This is really a substitute for the sleep_for function. This
  // alternative allows us to interrupt the sleep in case the user
  // calls Playback::Stop() while we are waiting between messages.
  std::mutex tempMutex;
  std::unique_lock<std::mutex> tempLock(tempMutex);

  // Wait until reaching _targetTime or until a stop signal is received.
  // The function will return true if the wait finished with a timeout state,
  // (having successfully achieved the time to wait) or false if the predicate
  // evaluates to true, which means that a pause or stop order was received,
  // interrupting the wait.
  return this->stopConditionVariable.wait_for(
      tempLock, _targetTime - waitStartTime, FinishedWaiting);
}

//////////////////////////////////////////////////
void PlaybackHandle::Implementation::Step(
    const std::chrono::nanoseconds &_stepDuration)
{
  if (_stepDuration.count() == 0) return;
  this->boundaryTime = this->playbackTime + _stepDuration;
  this->Resume();
}

//////////////////////////////////////////////////
void PlaybackHandle::Implementation::Seek(
    const std::chrono::nanoseconds &_newElapsedTime)
{
  if (this->stop)
  {
    LERR("Seek can't be called from a stopped playback.\n");
    return;
  }
  const QualifiedTime beginTime(this->firstMessageTime + _newElapsedTime);
  const QualifiedTime endTime(std::chrono::nanoseconds::max());
  const QualifiedTimeRange timeRange(beginTime, endTime);
  {
    std::unique_lock<std::mutex> lk(this->batchMutex);
    this->batch = this->logFile->QueryMessages(
        TopicList::Create(this->trackedTopics, timeRange));
    this->messageIter = this->batch.begin();
  }
  this->playbackTime = this->messageIter->TimeReceived();
  this->nextMessageTime = this->messageIter->TimeReceived();
  this->boundaryTime = std::chrono::nanoseconds::max();
  this->lastEventTime = std::chrono::steady_clock::now().time_since_epoch();
}

//////////////////////////////////////////////////
void PlaybackHandle::Implementation::Stop()
{
  if (!this->logFile->Valid())
  {
    return;
  }

  this->stop = true;
  this->stopConditionVariable.notify_all();

  if (this->paused)
  {
    std::unique_lock<std::mutex> lk(this->pauseMutex);
    this->pauseConditionVariable.notify_all();
    this->paused = false;
  }

  if (this->playbackThread.joinable())
    this->playbackThread.join();
}

//////////////////////////////////////////////////
void PlaybackHandle::Implementation::Pause()
{
  std::unique_lock<std::mutex> lk(this->pauseMutex);
  if (!this->paused)
  {
    this->paused = true;
    std::chrono::nanoseconds now(
        std::chrono::steady_clock::now().time_since_epoch());
    // Advance time in the playback frame to the moment when pause started
    this->playbackTime = this->playbackTime +
        now - this->lastEventTime;
    // Update last event time in the realtime frame.
    this->lastEventTime = now;
    this->boundaryTime = std::chrono::nanoseconds::max();
  }
}

//////////////////////////////////////////////////
void PlaybackHandle::Implementation::Resume()
{
  std::unique_lock<std::mutex> lk(this->pauseMutex);
  if (this->paused)
  {
    this->paused = false;
    this->pauseConditionVariable.notify_all();
  }
}

//////////////////////////////////////////////////
bool PlaybackHandle::Implementation::IsPaused() const
{
  return this->paused;
}

//////////////////////////////////////////////////
PlaybackHandle::~PlaybackHandle()
{
  if (this->dataPtr)
  {
    this->dataPtr->Stop();
  }
}

//////////////////////////////////////////////////
void PlaybackHandle::Stop()
{
  this->dataPtr->Stop();
}

//////////////////////////////////////////////////
void PlaybackHandle::Pause()
{
  this->dataPtr->Pause();
}

//////////////////////////////////////////////////
void PlaybackHandle::Step(const std::chrono::nanoseconds &_stepDuration)
{
  this->dataPtr->Step(_stepDuration);
}

//////////////////////////////////////////////////
void PlaybackHandle::Seek(const std::chrono::nanoseconds &_newElapsedTime)
{
  this->dataPtr->Seek(_newElapsedTime);
}

//////////////////////////////////////////////////
void PlaybackHandle::Resume()
{
  this->dataPtr->Resume();
}

//////////////////////////////////////////////////
bool PlaybackHandle::IsPaused() const
{
  return this->dataPtr->IsPaused();
}

//////////////////////////////////////////////////
void PlaybackHandle::WaitUntilFinished()
{
  this->dataPtr->WaitUntilFinished();
}

//////////////////////////////////////////////////
bool PlaybackHandle::Finished() const
{
  return this->dataPtr->finished;
}

//////////////////////////////////////////////////
std::chrono::nanoseconds PlaybackHandle::StartTime() const
{
  return this->dataPtr->playbackStartTime;
}

//////////////////////////////////////////////////
std::chrono::nanoseconds PlaybackHandle::CurrentTime() const
{
  return this->dataPtr->playbackTime;
}

//////////////////////////////////////////////////
std::chrono::nanoseconds PlaybackHandle::EndTime() const
{
  return this->dataPtr->playbackEndTime;
}

//////////////////////////////////////////////////
PlaybackHandle::PlaybackHandle(
  std::unique_ptr<Implementation> &&_internal) // NOLINT
  : dataPtr(std::move(_internal))
{
  // Do nothing
}
