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

#include <gtest/gtest.h>

#include <gz/transport/log/Log.hh>
#include <gz/transport/log/Playback.hh>
#include <gz/transport/log/Recorder.hh>
#include <gz/transport/Node.hh>

#include <gz/utils/Environment.hh>
#include <gz/utils/ExtraTestMacros.hh>
#include <gz/utils/Subprocess.hh>

#include "ChirpParams.hh"

#include "test_utils.hh"

static std::string partition;

struct MessageInformation
{
  public: std::string data;
  public: std::string type;
  public: std::string topic;
};

static std::mutex dataMutex;

//////////////////////////////////////////////////
/// \brief This is used within lambda callbacks to keep track of incoming
/// messages.
/// \param[out] _archive A vector that will store the incoming message
/// information. This must be passed from a lambda which has captured a vector.
/// \param[in] _data The data passed by the SubscribeRaw
/// \param[in] _len The length of data passed by the SubscribeRaw
/// \param[in] _msgInfo The metainfo about the message, provided by the
/// SubscribeRaw.
void TrackMessages(std::vector<MessageInformation> &_archive,
                   const char *_data,
                   std::size_t _len,
                   const gz::transport::MessageInfo &_msgInfo)
{
  MessageInformation info;
  info.data = std::string(_data, _len);
  info.type = _msgInfo.Type();
  info.topic = _msgInfo.Topic();

  std::unique_lock<std::mutex> lock(dataMutex);
  _archive.push_back(info);
}


//////////////////////////////////////////////////
/// \brief Compares two messages.
/// \param[in] _recorded message that was recorded
/// \param[in] _played message that was published
/// \param[out] a boolean
bool MessagesAreEqual(
    const MessageInformation &_recorded,
    const MessageInformation &_played)
{
  return _recorded.data  == _played.data &&
        _recorded.type  == _played.type &&
        _recorded.topic == _played.topic;
}

//////////////////////////////////////////////////
/// \brief Compares two vectors of messages.
/// \param[in] _recorded vector of messages that were recorded
/// \param[in] _played vector of messages that were published
/// \param[out] a boolean
bool ExpectSameMessages(
    const std::vector<MessageInformation> &_recorded,
    const std::vector<MessageInformation> &_played)
{
  for (std::size_t i = 0; i < _recorded.size() && i < _played.size(); ++i)
  {
    if (!MessagesAreEqual(_recorded[i], _played[i])) return false;
  }
  if (_recorded.size() != _played.size()) return false;
  return true;
}


//////////////////////////////////////////////////
/// \brief Record a log and then play it back. Verify that the playback matches
/// the original.
TEST(playback, GZ_UTILS_TEST_DISABLED_ON_MAC(ReplayLog))
{
  std::vector<std::string> topics = {"/foo", "/bar", "/baz"};

  std::vector<MessageInformation> incomingData;

  auto callback = [&incomingData](
      const char *_data,
      std::size_t _len,
      const gz::transport::MessageInfo &_msgInfo)
  {
    TrackMessages(incomingData, _data, _len, _msgInfo);
  };

  gz::transport::Node node;
  gz::transport::log::Recorder recorder;

  for (const std::string &topic : topics)
  {
    node.SubscribeRaw(topic, callback);
    recorder.AddTopic(topic);
  }

  const std::string logName = "file:playbackReplayLog?mode=memory&cache=shared";
  EXPECT_EQ(gz::transport::log::RecorderError::SUCCESS,
    recorder.Start(logName));

  const int numChirps = 100;
  auto chirper =
    gz::transport::log::test::BeginChirps(topics, numChirps, partition);

  // Wait for the chirping to finish
  chirper.Join();

  // Wait to make sure our callbacks are done processing the incoming messages
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Create playback before stopping so sqlite memory database is shared
  gz::transport::log::Playback playback(logName);
  recorder.Stop();

  // Make a copy of the data so we can compare it later
  std::vector<MessageInformation> originalData = incomingData;

  // Clear out the old data so we can recreate it during the playback
  incomingData.clear();

  for (const std::string &topic : topics)
  {
    playback.AddTopic(topic);
  }

  const auto handle = playback.Start();
  std::cout << "Waiting to for playback to finish..." << std::endl;
  handle->WaitUntilFinished();
  std::cout << " Done waiting..." << std::endl;
  handle->Stop();
  std::cout << "Playback finished!" << std::endl;

  // Ensure playback times are reasonable.
  const std::chrono::milliseconds expectedDuration{
    numChirps * gz::transport::log::test::DelayBetweenChirps_ms};
  // Windows uses system clock for sleep, and playback uses a steady clock.
  // This can lead to errors.
#ifdef _WIN32
  EXPECT_GE((handle->EndTime() - handle->StartTime()).count(),
      expectedDuration.count() * 0.5);
#else
  EXPECT_GE(handle->EndTime() - handle->StartTime(), expectedDuration);
#endif
  EXPECT_EQ(handle->EndTime(), handle->CurrentTime());

  // Wait to make sure our callbacks are done processing the incoming messages
  // (Strangely, Windows throws an exception when this is ~1s or more)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(ExpectSameMessages(originalData, incomingData));
}


//////////////////////////////////////////////////
TEST(playback, GZ_UTILS_TEST_DISABLED_ON_MAC(ReplayNoSuchTopic))
{
  gz::transport::log::Recorder recorder;
  const std::string logName =
    "file:playbackReplayNoSuchTopic?mode=memory&cache=shared";
  EXPECT_EQ(gz::transport::log::RecorderError::SUCCESS,
    recorder.Start(logName));

  gz::transport::log::Playback playback(logName);
  recorder.Stop();

  EXPECT_FALSE(playback.AddTopic("/DNE"));
  EXPECT_EQ(0, playback.AddTopic(std::regex("/DNE")));
}


//////////////////////////////////////////////////
/// \brief Record a log and then play it back. Verify that the playback matches
/// the original.
TEST(playback, GZ_UTILS_TEST_DISABLED_ON_MAC(ReplayLogRegex))
{
  std::vector<std::string> topics = {"/foo", "/bar", "/baz"};

  std::vector<MessageInformation> incomingData;

  auto callback = [&incomingData](
      const char *_data,
      std::size_t _len,
      const gz::transport::MessageInfo &_msgInfo)
  {
    TrackMessages(incomingData, _data, _len, _msgInfo);
  };

  gz::transport::Node node;
  gz::transport::log::Recorder recorder;

  for (const std::string &topic : topics)
  {
    node.SubscribeRaw(topic, callback);
  }
  recorder.AddTopic(std::regex(".*"));

  const std::string logName =
    "file:playbackReplayLogRegex?mode=memory&cache=shared";
  EXPECT_EQ(gz::transport::log::RecorderError::SUCCESS,
    recorder.Start(logName));

  const int numChirps = 100;
  auto chirper =
      gz::transport::log::test::BeginChirps(topics, numChirps, partition);

  // Wait for the chirping to finish
  chirper.Join();

  // Wait to make sure our callbacks are done processing the incoming messages
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Create playback before stopping so sqlite memory database is shared
  gz::transport::log::Playback playback(logName);
  recorder.Stop();

  // Make a copy of the data so we can compare it later
  std::vector<MessageInformation> originalData = incomingData;

  // Clear out the old data so we can recreate it during the playback
  incomingData.clear();

  const auto handle = playback.Start();
  EXPECT_FALSE(handle->Finished());
  std::cout << "Waiting to for playback to finish..." << std::endl;
  handle->WaitUntilFinished();
  std::cout << " Done waiting..." << std::endl;
  handle->Stop();
  std::cout << "Playback finished!" << std::endl;
  EXPECT_TRUE(handle->Finished());

  // Wait to make sure our callbacks are done processing the incoming messages
  // (Strangely, Windows throws an exception when this is ~1s or more)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(ExpectSameMessages(originalData, incomingData));
}

//////////////////////////////////////////////////
/// \brief Record a log and then play it back after removing some topics. Verify
/// that the playback matches the original.
TEST(playback, GZ_UTILS_TEST_DISABLED_ON_MAC(RemoveTopic))
{
  std::vector<std::string> topics = {"/foo", "/bar", "/baz"};

  std::vector<MessageInformation> incomingData;

  auto callback = [&incomingData](
      const char *_data,
      std::size_t _len,
      const gz::transport::MessageInfo &_msgInfo)
  {
    TrackMessages(incomingData, _data, _len, _msgInfo);
  };

  gz::transport::Node node;
  gz::transport::log::Recorder recorder;

  for (const std::string &topic : topics)
  {
    node.SubscribeRaw(topic, callback);
  }
  recorder.AddTopic(std::regex(".*"));

  const std::string logName =
    "file:playbackReplayLogRegex?mode=memory&cache=shared";
  EXPECT_EQ(gz::transport::log::RecorderError::SUCCESS,
    recorder.Start(logName));

  const int numChirps = 100;
  auto chirper =
      gz::transport::log::test::BeginChirps(topics, numChirps, partition);

  // Wait for the chirping to finish
  chirper.Join();

  // Wait to make sure our callbacks are done processing the incoming messages
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Create playback before stopping so sqlite memory database is shared
  gz::transport::log::Playback playback(logName);
  recorder.Stop();

  // Clear out the old data so we can recreate it during the playback
  incomingData.clear();

  // Remove some topics without calling AddTopic(). This tells the Playback that
  // it should play all topics except for these.
  EXPECT_TRUE(playback.RemoveTopic("/foo"));
  EXPECT_TRUE(playback.RemoveTopic("/baz"));

  {
    const auto handle = playback.Start();
    EXPECT_FALSE(handle->Finished());
    std::cout << "Waiting to for playback to finish..." << std::endl;
    handle->WaitUntilFinished();
    std::cout << " Done waiting..." << std::endl;
    handle->Stop();
    std::cout << "Playback finished!" << std::endl;
    EXPECT_TRUE(handle->Finished());

    // Wait to make sure our callbacks are done processing the incoming messages
    // (Strangely, Windows throws an exception when this is ~1s or more)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Every message that we received should be from the /bar topic, because
  // we removed the other two.
  EXPECT_EQ(numChirps, static_cast<int>(incomingData.size()));
  for (const MessageInformation &info : incomingData)
  {
    EXPECT_EQ("/bar", info.topic);
  }


  // Clear out the old data so we can recreate it during the playback
  incomingData.clear();

  // Add the original two topics, but then remove all topics beginning with /b
  EXPECT_TRUE(playback.AddTopic("/foo"));
  EXPECT_TRUE(playback.AddTopic("/baz"));
  EXPECT_EQ(2, playback.RemoveTopic(std::regex("/b.*")));

  {
    const auto handle = playback.Start();
    EXPECT_FALSE(handle->Finished());
    std::cout << "Waiting to for playback to finish..." << std::endl;
    handle->WaitUntilFinished();
    std::cout << " Done waiting..." << std::endl;
    handle->Stop();
    std::cout << "Playback finished!" << std::endl;
    EXPECT_TRUE(handle->Finished());

    // Wait to make sure our callbacks are done processing the incoming messages
    // (Strangely, Windows throws an exception when this is ~1s or more)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Every message that we received should be from the /foo topic, because
  // we removed the other two.
  EXPECT_EQ(numChirps, static_cast<int>(incomingData.size()));
  for (const MessageInformation &info : incomingData)
  {
    EXPECT_EQ("/foo", info.topic);
  }
}

//////////////////////////////////////////////////
/// \brief Record a log and then play it back. Verify that the playback matches
/// the original.
TEST(playback, GZ_UTILS_TEST_DISABLED_ON_MAC(ReplayLogMoveInstances))
{
  std::vector<std::string> topics = {"/foo", "/bar", "/baz"};

  std::vector<MessageInformation> incomingData;

  auto callback = [&incomingData](
      const char *_data,
      std::size_t _len,
      const gz::transport::MessageInfo &_msgInfo)
  {
    TrackMessages(incomingData, _data, _len, _msgInfo);
  };

  gz::transport::Node node;
  gz::transport::log::Recorder recorder_orig;

  for (const std::string &topic : topics)
  {
    node.SubscribeRaw(topic, callback);
  }
  recorder_orig.AddTopic(std::regex(".*"));

  gz::transport::log::Recorder recorder(std::move(recorder_orig));

  const std::string logName =
    "file:playbackReplayLogRegex?mode=memory&cache=shared";
  EXPECT_EQ(gz::transport::log::RecorderError::SUCCESS,
    recorder.Start(logName));

  const int numChirps = 100;
  auto chirper =
      gz::transport::log::test::BeginChirps(topics, numChirps, partition);

  // Wait for the chirping to finish
  chirper.Join();

  // Wait to make sure our callbacks are done processing the incoming messages
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Create playback before stopping so sqlite memory database is shared
  gz::transport::log::Playback playback_orig(logName);
  recorder.Stop();

  // Make a copy of the data so we can compare it later
  std::vector<MessageInformation> originalData = incomingData;

  // Clear out the old data so we can recreate it during the playback
  incomingData.clear();

  playback_orig.AddTopic(std::regex(".*"));
  gz::transport::log::Playback playback(std::move(playback_orig));
  const auto handle = playback.Start();

  std::cout << "Waiting to for playback to finish..." << std::endl;
  handle->WaitUntilFinished();
  std::cout << " Done waiting..." << std::endl;
  handle->Stop();
  std::cout << "Playback finished!" << std::endl;

  // Wait to make sure our callbacks are done processing the incoming messages
  // (Strangely, Windows throws an exception when this is ~1s or more)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(ExpectSameMessages(originalData, incomingData));
}

//////////////////////////////////////////////////
/// \brief Record a log and then play it back calling the Pause and Resume
/// methods to control the playback flow.
TEST(playback, GZ_UTILS_TEST_DISABLED_ON_MAC(ReplayPauseResume))
{
  std::vector<std::string> topics = {"/foo", "/bar", "/baz"};

  std::vector<MessageInformation> incomingData;

  auto callback = [&incomingData](
      const char *_data,
      std::size_t _len,
      const gz::transport::MessageInfo &_msgInfo)
  {
    TrackMessages(incomingData, _data, _len, _msgInfo);
  };

  gz::transport::Node node;
  gz::transport::log::Recorder recorder;

  for (const std::string &topic : topics)
  {
    node.SubscribeRaw(topic, callback);
    recorder.AddTopic(topic);
  }

  const std::string logName = "file:playbackReplayLog?mode=memory&cache=shared";
  EXPECT_EQ(gz::transport::log::RecorderError::SUCCESS,
    recorder.Start(logName));

  const int numChirps = 100;
  auto chirper =
    gz::transport::log::test::BeginChirps(topics, numChirps, partition);

  // Wait for the chirping to finish
  chirper.Join();

  // Wait to make sure our callbacks are done processing the incoming messages
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Create playback before stopping so sqlite memory database is shared
  gz::transport::log::Playback playback(logName);
  recorder.Stop();

  // Make a copy of the data so we can compare it later
  std::vector<MessageInformation> originalData = incomingData;

  // Clear out the old data so we can recreate it during the playback
  incomingData.clear();

  for (const std::string &topic : topics)
  {
    playback.AddTopic(topic);
  }

  const auto handle = playback.Start();

  // Wait until approximately half of the chirps have been played back
  std::this_thread::sleep_for(
        std::chrono::milliseconds(
          gz::transport::log::test::DelayBetweenChirps_ms *
          numChirps / 2));

  // Pause Playback
  handle->Pause();

  // Wait for incomingData to catch up with the played back messages
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // The playback must be paused now
  EXPECT_TRUE(handle->IsPaused());

  // Make a copy of the last received message
  const MessageInformation originalMessage{incomingData.back()};

  // Pause for an arbitrary amount of time.
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // If the playback has been successfully paused,
  // the last incoming message shouldn't change over time.
  MessageInformation lastReceivedMessage{incomingData.back()};

  EXPECT_TRUE(MessagesAreEqual(originalMessage, lastReceivedMessage));

  std::cout << "Resuming playback..." << std::endl;

  handle->Resume();

  // Playback around a quarter of the total number of chirps
  std::this_thread::sleep_for(
        std::chrono::milliseconds(
          gz::transport::log::test::DelayBetweenChirps_ms *
          numChirps / 4));

  handle->Pause();

  // Wait for incomingData to catch up with the played back messages
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // If the playback hasn't been paused, the message received must differ
  // from the one recorded an instant before.
  lastReceivedMessage = incomingData.back();

  EXPECT_FALSE(MessagesAreEqual(originalMessage, lastReceivedMessage));

  handle->Resume();

  std::cout << "Waiting to for playback to finish..." << std::endl;
  handle->WaitUntilFinished();
  std::cout << " Done waiting..." << std::endl;
  handle->Stop();
  std::cout << "Playback finished!" << std::endl;

  // Checks that the stream of messages hasn't been corrupted in between
  // pausing and resuming.
  EXPECT_TRUE(ExpectSameMessages(originalData, incomingData));

  // Wait to make sure our callbacks are done processing the incoming messages
  // (Strangely, Windows throws an exception when this is ~1s or more)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

//////////////////////////////////////////////////
/// \brief Record a log and then play it back calling the Step method to control
/// the playback workflow.
TEST(playback, GZ_UTILS_TEST_ENABLED_ONLY_ON_LINUX(ReplayStep))
{
  std::vector<std::string> topics = {"/foo", "/bar", "/baz"};

  std::vector<MessageInformation> incomingData;

  auto callback = [&incomingData](
      const char *_data,
      std::size_t _len,
      const gz::transport::MessageInfo &_msgInfo)
  {
    TrackMessages(incomingData, _data, _len, _msgInfo);
  };

  gz::transport::Node node;
  gz::transport::log::Recorder recorder;

  for (const std::string &topic : topics)
  {
    node.SubscribeRaw(topic, callback);
    recorder.AddTopic(topic);
  }

  const std::string logName = "file:playbackReplayLog?mode=memory&cache=shared";
  EXPECT_EQ(gz::transport::log::RecorderError::SUCCESS,
    recorder.Start(logName));

  const int numChirps = 100;
  auto chirper =
    gz::transport::log::test::BeginChirps(topics, numChirps, partition);

  // Wait for the chirping to finish
  chirper.Join();

  // Wait to make sure our callbacks are done processing the incoming messages
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Create playback before stopping so sqlite memory database is shared
  gz::transport::log::Playback playback(logName);
  recorder.Stop();

  // Make a copy of the data so we can compare it later
  std::vector<MessageInformation> originalData = incomingData;

  // Clear out the old data so we can recreate it during the playback
  incomingData.clear();

  for (const std::string &topic : topics)
  {
    playback.AddTopic(topic);
  }

  const auto handle = playback.Start();

  std::chrono::milliseconds totalDurationMs(
      gz::transport::log::test::DelayBetweenChirps_ms * numChirps);

  // Wait until approximately an tenth of the chirps have been played back
  std::this_thread::sleep_for(totalDurationMs / 10);

  // Pause Playback
  handle->Pause();

  // Wait for incomingData to catch up with the played back messages
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Make a copy of the last received message
  const MessageInformation firstMessageData{incomingData.back()};

  std::cout << "Stepping playback..." << std::endl;

  // Step for 10 milliseconds
  handle->Step(std::chrono::milliseconds(10));

  // Wait for incomingData to catch up with the played back messages
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  const MessageInformation secondMessageData{incomingData.back()};

  // The last message received after the Step was executed must differ from
  // the one received before executing it
  EXPECT_FALSE(MessagesAreEqual(firstMessageData, secondMessageData));

  // Step for 10 milliseconds
  handle->Step(std::chrono::milliseconds(10));

  // Wait for incomingData to catch up with the played back messages
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Make a copy of the last received message
  const MessageInformation thirdMessageData{incomingData.back()};

  // The last message received after the Step was executed must differ from
  // the one received before executing it
  EXPECT_FALSE(MessagesAreEqual(secondMessageData, thirdMessageData));

  handle->Resume();

  std::cout << "Waiting to for playback to finish..." << std::endl;
  handle->WaitUntilFinished();
  std::cout << " Done waiting..." << std::endl;
  handle->Stop();
  std::cout << "Playback finished!" << std::endl;

  // Checks that the stream of messages hasn't been corrupted in between
  // pausing and resuming.
  EXPECT_TRUE(ExpectSameMessages(originalData, incomingData));

  // Wait to make sure our callbacks are done processing the incoming messages
  // (Strangely, Windows throws an exception when this is ~1s or more)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

//////////////////////////////////////////////////
/// \brief Record a log and then play it back calling the Seek method to control
/// the playback workflow.
TEST(playback, GZ_UTILS_TEST_DISABLED_ON_MAC(ReplaySeek))
{
  std::vector<std::string> topics = {"/foo", "/bar", "/baz"};

  std::vector<MessageInformation> incomingData;

  auto callback = [&incomingData](
      const char *_data,
      std::size_t _len,
      const gz::transport::MessageInfo &_msgInfo)
  {
    TrackMessages(incomingData, _data, _len, _msgInfo);
  };

  gz::transport::Node node;
  gz::transport::log::Recorder recorder;

  for (const std::string &topic : topics)
  {
    node.SubscribeRaw(topic, callback);
    recorder.AddTopic(topic);
  }

  const std::string logName = "file:playbackReplayLog?mode=memory&cache=shared";
  EXPECT_EQ(gz::transport::log::RecorderError::SUCCESS,
    recorder.Start(logName));

  const int numChirps = 100;
  auto chirper =
    gz::transport::log::test::BeginChirps(topics, numChirps, partition);

  // Wait for the chirping to finish
  chirper.Join();

  // Wait to make sure our callbacks are done processing the incoming messages
  std::this_thread::sleep_for(std::chrono::seconds(1));

  // Create playback before stopping so sqlite memory database is shared
  gz::transport::log::Playback playback(logName);
  recorder.Stop();

  // Clear out the old data so we can recreate it during the playback
  incomingData.clear();

  for (const std::string &topic : topics)
  {
    playback.AddTopic(topic);
  }

  const auto handle = playback.Start();

  std::chrono::milliseconds totalDurationMs(
      gz::transport::log::test::DelayBetweenChirps_ms * numChirps);

  // Wait until approximately an tenth of the chirps have been played back
  std::this_thread::sleep_for(totalDurationMs / 10);

  handle->Pause();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  using namespace std::chrono_literals;

  // Seek to time after about 10 messages have been published
  // and play two messages from that point of time.
  handle->Seek(std::chrono::milliseconds(
      gz::transport::log::test::DelayBetweenChirps_ms * 10));
  handle->Step(std::chrono::milliseconds(
      gz::transport::log::test::DelayBetweenChirps_ms * 2));

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  const MessageInformation firstMessageData{incomingData.back()};

  handle->Resume();

  // Play about 5 messages before pausing again
  std::this_thread::sleep_for(
      std::chrono::milliseconds(
        gz::transport::log::test::DelayBetweenChirps_ms * 5));

  handle->Pause();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  const MessageInformation secondMessageData{incomingData.back()};

  EXPECT_FALSE(MessagesAreEqual(firstMessageData, secondMessageData));

  // Seek to time after about 10 messages have been published
  // and play two messages from that point of time.
  handle->Seek(std::chrono::milliseconds(
      gz::transport::log::test::DelayBetweenChirps_ms * 10));
  handle->Step(std::chrono::milliseconds(
      gz::transport::log::test::DelayBetweenChirps_ms * 2));

  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  const MessageInformation thirdMessageData{incomingData.back()};

  // Expect the same message as the previous Seek, since reproduction should
  // be in the exact same position as the previous one
  EXPECT_TRUE(MessagesAreEqual(firstMessageData, thirdMessageData));

  // Resume Playback
  handle->Resume();

  std::cout << "Waiting to for playback to finish..." << std::endl;
  handle->WaitUntilFinished();
  std::cout << " Done waiting..." << std::endl;
  handle->Stop();
  std::cout << "Playback finished!" << std::endl;

  // Wait to make sure our callbacks are done processing the incoming messages
  // (Strangely, Windows throws an exception when this is ~1s or more)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Get a random partition name to avoid topic collisions between processes.
  partition = testing::getRandomNumber();

  // Set the partition name for this process.
  gz::utils::setenv("GZ_PARTITION", partition);

  gz::utils::setenv("GZ_TRANSPORT_LOG_SQL_PATH",
                    GZ_TRANSPORT_LOG_SQL_PATH);

  gz::utils::setenv(gz::transport::log::SchemaLocationEnvVar,
                    GZ_TRANSPORT_LOG_SQL_PATH);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
