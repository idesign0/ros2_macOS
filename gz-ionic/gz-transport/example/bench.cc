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

//////////////////////////////////////////////////
/// Usage: ./bench <options>
///
/// Options:
///
/// -h Help
/// -l Latency test
/// -t Throughput test
/// -p Publish node
/// -r Reply node
///
/// Choose one of [-l, -t], and one (or none for in-process
/// testing) [-p,-r].
///
/// See `latency.gp` and `throughput.gp` to plot output.
//////////////////////////////////////////////////

#ifdef __linux__
#include <sys/utsname.h>
#endif

#include <gflags/gflags.h>

#include <cstdint>
#include <iomanip>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <gz/msgs.hh>
#include <gz/transport.hh>

DEFINE_bool(h, false, "Show help");
DEFINE_bool(t, false, "Throughput testing");
DEFINE_bool(l, false, "Latency testing");
DEFINE_bool(r, false, "Relay node");
DEFINE_bool(p, false, "Publishing node");
DEFINE_uint64(f, 0, "Flood the network with extra publishers and subscribers");
DEFINE_uint64(i, 1000, "Number of iterations");
DEFINE_string(o, "", "Output filename");

std::condition_variable gCondition;
std::mutex gMutex;
bool gStop = false;

/// \brief A class that subscribes to all of the `/benchmark/flood/*`
/// topics. FloodSub and FloodPub can be enabled with the `-f <num>` command
/// line argument. Flooding adds <num> extra publishers and subscribers. The
/// purpose is to "flood" the network with extra messages while performing
/// benchmark analysis.
class FloodSub
{
  /// \brief Create the subscribers.
  /// \param[in] _count The number of subscribers to create.
  public: explicit FloodSub(uint64_t _count)
  {
    // Create flood publishers
    for (uint64_t i = 0; i < _count; ++i)
    {
      std::ostringstream stream;
      stream << "/benchmark/flood/"  << i;
      this->node.Subscribe(stream.str(), &FloodSub::OnMsg, this);
    }
  }

  /// \brief Dummy callback.
  /// \param[in] _msg The message.
  public: void OnMsg(const gz::msgs::Bytes & /*_msg*/)
  {
  }

  /// \brief Communication node.
  private: gz::transport::Node node;
};

/// \brief A class that publishes on a number of `/benchmark/flood/*`
/// topics. FloodSub and FloodPub can be enabled with the `-f <num>` command
/// line argument. Flooding adds <num> extra publishers and subscribers. The
/// purpose is to "flood" the network with extra messages while performing
/// benchmark analysis.
class FloodPub
{
  /// \brief Create a number of publishers.
  /// \param[in] _count Number of publishers to create.
  public: explicit FloodPub(uint64_t _count)
  {
    // Create flood publishers
    for (uint64_t i = 0; i < _count; ++i)
    {
      std::ostringstream stream;
      stream << "/benchmark/flood/"  << i;
      this->floodPubs.push_back(
          this->node.Advertise<gz::msgs::Bytes>(stream.str()));
    }
    if (!this->floodPubs.empty())
      this->runThread = std::thread(&FloodPub::RunLoop, this);
  }

  /// \brief Destructor.
  public: ~FloodPub()
  {
    this->Stop();
    if (this->runThread.joinable())
      this->runThread.join();
  }

  /// \brief Stop the publishers.
  public: void Stop()
  {
    this->running = false;
  }

  /// \brief Run the publishers.
  private: void RunLoop()
  {
    gz::msgs::Bytes msg;
    int size = 1000;
    char *byteData = new char[size];
    std::memset(byteData, '0', size);
    msg.set_data(byteData);

    this->running = true;
    while (this->running)
    {
      for (gz::transport::Node::Publisher &pub : this->floodPubs)
      {
        pub.Publish(msg);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  /// \brief Communication node.
  private: gz::transport::Node node;

  /// \brief Run thread.
  private: std::thread runThread;

  /// \brief True when running.
  private: bool running{false};

  /// \brief The publishers.
  private: std::vector<gz::transport::Node::Publisher> floodPubs;
};

/// \brief The ReplyTester subscribes to the benchmark topics, and relays
/// incoming messages on a corresponding "reply" topic.
///
/// A publisher should send messages on either:
///
///   1. /benchmark/latency/request For latency testing
///   2. /benchmark/throughput/request For throughput testing.
///
/// The incoming and outgoing message types are gz::msgs::Bytes.
class ReplyTester
{
  /// Constructor that creates the publishers and subscribers.
  public: ReplyTester()
  {
    // Advertise on the throughput reply topic
    this->throughputPub = this->node.Advertise<gz::msgs::Bytes>(
        "/benchmark/throughput/reply");
    if (!this->throughputPub)
    {
      std::cerr << "Error advertising topic /benchmark/throughput/reply"
                << std::endl;
      return;
    }

    // Advertise on the latency reply topic
    this->latencyPub = this->node.Advertise<gz::msgs::Bytes>(
        "/benchmark/latency/reply");
    if (!this->latencyPub)
    {
      std::cerr << "Error advertising topic /benchmark/latency/reply"
                << std::endl;
      return;
    }

    // Subscribe to the throughput request topic.
    if (!node.Subscribe("/benchmark/throughput/request",
          &ReplyTester::ThroughputCb, this))
    {
      std::cerr << "Error subscribing to topic /benchmark/throughput/request"
                << std::endl;
      return;
    }

    // Subscribe to the latency request topic.
    if (!node.Subscribe("/benchmark/latency/request",
          &ReplyTester::LatencyCb, this))
    {
      std::cerr << "Error subscribing to topic /benchmark/latency/request"
                << std::endl;
      return;
    }

    // Kick discovery.
    // \todo: Improve discovery so that this is not required.
    std::vector<std::string> topics;
    this->node.TopicList(topics);
  }

  /// \brief Function called each time a throughput message is received.
  /// \param[in] _msg Incoming message of variable size.
  private: void ThroughputCb(const gz::msgs::Bytes &_msg)
  {
    if (this->prevStamp > 0 && _msg.header().stamp().sec() != 0)
    {
      if (_msg.header().stamp().sec() != this->prevStamp+1)
      {
        std::cerr << "Received[" << _msg.header().stamp().sec()
          << "] Expected[" << this->prevStamp+1 << "]\n";

        std::terminate();
      }
    }

    this->prevStamp = _msg.header().stamp().sec();
    // Debug:: std::cout << _msg.header().stamp().sec() << std::endl;

    this->throughputPub.Publish(_msg);
  }

  /// \brief Function called each time a latency message is received.
  /// \param[in] _msg Incoming message of variable size.
  private: void LatencyCb(const gz::msgs::Bytes &_msg)
  {
    this->latencyPub.Publish(_msg);
  }

  /// \brief The transport node
  private: gz::transport::Node node;

  /// \brief The throughput publisher
  private: gz::transport::Node::Publisher throughputPub;

  /// \brief The latency publisher
  private: gz::transport::Node::Publisher latencyPub;

  private: int prevStamp = 0;
};

/// \brief The PubTester is used to collect data on latency or throughput.
/// Latency is the measure of time from message publication to message
/// reception. Latency is calculated by dividing the complete roundtrip
/// time of a message in half. This avoids time synchronization issues.
///
/// Throughput is measured by sending N messages, and measuring the time
/// required to send those messages. Again, half of the complete roundtrip
/// time is used to avoid time synchronization issues.
///
/// The latency topics are:
///
///   1. /benchmark/latency/request Outbound data, sent by this class.
///   2. /benchmark/latency/reply Inbound data, sent by ReplyTester.
///
/// The throughput topics are:
///
///   1. /benchmark/throughput/request Outbound data, sent by this class.
///   2. /benchmark/throughput/reply Inbound data, sent by ReplyTester.
class PubTester
{
  /// \brief Default constructor.
  public: PubTester() = default;

  /// \brief Set the output filename. Use empty string to output to the
  /// console.
  /// \param[in] _filename Output filename
  public: void SetOutputFilename(const std::string &_filename)
  {
    this->filename = _filename;
  }

  /// \brief Set the number of iterations.
  /// \param[in] _iters Number of iterations.
  public: void SetIterations(const uint64_t _iters)
  {
    this->sentMsgs = _iters;
  }

  /// \brief Create the publishers and subscribers.
  public: void Init()
  {
    // Throughput publisher
    this->throughputPub = this->node.Advertise<gz::msgs::Bytes>(
        "/benchmark/throughput/request");
    if (!this->throughputPub)
    {
      std::cerr << "Error advertising topic /benchmark/throughput/request"
                << std::endl;
      return;
    }

    // Latency publisher
    this->latencyPub = this->node.Advertise<gz::msgs::Bytes>(
        "/benchmark/latency/request");
    if (!this->latencyPub)
    {
      std::cerr << "Error advertising topic /benchmark/latency/request"
                << std::endl;
      return;
    }

    // Subscribe to the throughput reply topic.
    if (!node.Subscribe("/benchmark/throughput/reply",
                        &PubTester::ThroughputCb, this))
    {
      std::cerr << "Error subscribing to topic /benchmark/throughput/reply"
                << std::endl;
      return;
    }

    // Subscribe to the latency reply topic.
    if (!node.Subscribe("/benchmark/latency/reply",
                        &PubTester::LatencyCb, this))
    {
      std::cerr << "Error subscribing to topic /benchmark/latency/reply"
                << std::endl;
      return;
    }

    // Kick discovery.
    // \todo: Improve discovery so that this is not required.
    std::vector<std::string> topics;
    this->node.TopicList(topics);
  }

  /// \brief Used to stop the test.
  public: void Stop()
  {
    std::unique_lock<std::mutex> lk(this->mutex);
    this->stop = true;
    this->condition.notify_all();
  }

  /// \brief Output header information
  /// \param[in] _stream Stream pointer
  private: void OutputHeader(std::ostream *_stream)
  {
    std::time_t t = std::time(NULL);
    std::tm tm = *std::localtime(&t);

    (*_stream) << "# " << std::put_time(&tm, "%FT%T%Z") << std::endl;
    (*_stream) << "# Gazebo Transport Version "
               << GZ_TRANSPORT_VERSION_FULL << std::endl;

#ifdef __linux__
    struct utsname unameData;
    uname(&unameData);
    (*_stream) << "# " << unameData.sysname << " " << unameData.release
               << " " << unameData.version << " " << unameData.machine
               << std::endl;
#endif
  }

  /// \brief Measure throughput. The output contains three columns:
  ///    1. Message size in bytes
  ///    2. Throughput in megabytes per second
  ///    3. Throughput in thousounds of messages per second
  public: void Throughput()
  {
    // Wait for subscriber
    while (!this->throughputPub.HasConnections() && !this->stop)
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Short circuit in case this test was stopped while waiting for
    // a subscriber
    if (this->stop)
      return;

    std::ostream *stream = &std::cout;
    std::ofstream fstream;

    if (!this->filename.empty())
    {
      fstream.open(this->filename);
      stream = &fstream;
    }

    this->OutputHeader(stream);

    // Column headers.
    (*stream) << "# Test\tSize(B)\t\tMB/s\t\tKmsg/s\n";

    int testNum = 1;
    // Iterate over each of the message sizes
    for (auto msgSize : this->msgSizes)
    {
      if (this->stop)
        return;

      // Reset counters
      this->totalBytes = 0;
      this->msgCount = 0;

      // Create the message of the given size
      this->PrepMsg(msgSize);

      // Start the clock
      auto timeStart = std::chrono::high_resolution_clock::now();

      // Send all the messages as fast as possible
      for (int i = 0; i < this->sentMsgs && !this->stop; ++i)
      {
        this->msg.mutable_header()->mutable_stamp()->set_sec(i);
        // Debug: std::cout << this->msg.header().stamp().sec() << std::endl;
        this->throughputPub.Publish(this->msg);
      }

      // Wait for all the reply messages. This will add little overhead
      // to the time, but should be negligible.
      std::unique_lock<std::mutex> lk(this->mutex);
      this->condition.wait(lk, [this] {
          return gStop || this->msgCount >= this->sentMsgs;});

      // Compute the number of microseconds
      uint64_t duration =
        std::chrono::duration_cast<std::chrono::microseconds>(
            this->timeEnd - timeStart).count();

      // Convert to seconds
      double seconds = (duration * 1e-6);

      // Output the data
      (*stream) << std::fixed << testNum++ << "\t" << this->dataSize << "\t\t"
                << (this->totalBytes * 1e-6) / seconds << "\t"
                << (this->msgCount * 1e-3) / seconds << "\t" <<  std::endl;
      this->expectedStamp = 0;
    }
  }

  /// \brief Measure latency. The output contains two columns:
  ///    1. Message size in bytes.
  ///    2. Latency in microseconds.
  public: void Latency()
  {
    // Wait for subscriber
    while (!this->latencyPub.HasConnections() && !this->stop)
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Short circuit in case this test was stopped while waiting for
    // a subscriber
    if (this->stop)
      return;

    std::ostream *stream = &std::cout;
    std::ofstream fstream;

    if (!this->filename.empty())
    {
      fstream.open(this->filename);
      stream = &fstream;
    }

    this->OutputHeader(stream);

    // Column headers.
    (*stream) << "# Test\tSize(B)\tAvg_(us)\tMin_(us)\tMax_(us)\n";

    uint64_t maxLatency = 0;
    uint64_t minLatency = std::numeric_limits<uint64_t>::max();
    int testNum = 1;
    // Iterate over each of the message sizes
    for (auto msgSize : this->msgSizes)
    {
      if (this->stop)
        return;

      // Create the message of the given size
      this->PrepMsg(msgSize);

      uint64_t sum = 0;

      // Send each message.
      for (int i = 0; i < this->sentMsgs && !this->stop; ++i)
      {
        // Lock so that we wait on a condition variable.
        std::unique_lock<std::mutex> lk(this->mutex);

        // Start the clock
        auto timeStart = std::chrono::high_resolution_clock::now();
        this->timeEnd = timeStart;

        // Send the message.
        this->latencyPub.Publish(this->msg);

        // Wait for the response.
        this->condition.wait(lk, [this, &timeStart] {
            return gStop || this->timeEnd > timeStart;});

        // Compute the number of microseconds
        uint64_t duration =
          std::chrono::duration_cast<std::chrono::microseconds>(
              this->timeEnd - timeStart).count();

        if (duration > maxLatency)
          maxLatency = duration;
        if (duration < minLatency)
          minLatency = duration;

        // Add to the sum of microseconds
        sum += duration;
      }

      // Output data.
      (*stream) << std::fixed << testNum++ << "\t" << this->dataSize << "\t"
                << (sum / static_cast<double>(this->sentMsgs)) * 0.5 << "\t"
                << minLatency * 0.5 << "\t"
                << maxLatency * 0.5 << std::endl;
    }
  }

  /// \brief Callback that handles throughput replies
  /// \param[in] _msg The reply message
  private: void ThroughputCb(const gz::msgs::Bytes &_msg)
  {
    // Lock
    std::unique_lock<std::mutex> lk(this->mutex);

    // Add to the total bytes received.
    this->totalBytes += this->dataSize;

    // Add to the total messages received.
    this->msgCount++;
    if (_msg.header().stamp().sec() != this->expectedStamp)
    {
      std::cerr << "Received[" << _msg.header().stamp().sec()
        << "] Expected[" << this->expectedStamp << "]\n";
      std::terminate();
    }
    this->expectedStamp++;

    // Notify Throughput() when all messages have been received.
    if (this->msgCount >= this->sentMsgs)
    {
      // End the clock.
      this->timeEnd = std::chrono::high_resolution_clock::now();
      condition.notify_all();
    }
  }

  /// \brief Callback that handles latency replies
  /// \param[in] _msg The reply message
  private: void LatencyCb(const gz::msgs::Bytes &_msg)
  {
    // End the time.
    this->timeEnd = std::chrono::high_resolution_clock::now();

    // Lock and notify
    std::unique_lock<std::mutex> lk(this->mutex);

    this->condition.notify_all();
  }

  /// \brief Create a new message of a give size.
  /// \param[in] _size Size (bytes) of the message to create.
  private: void PrepMsg(const int _size)
  {
    // Prepare the message.
    char *byteData = new char[_size];
    std::memset(byteData, '0', _size);
    msg.set_data(byteData);

    // Serialize so that we know how big the message is
    std::string data;
    this->msg.SerializeToString(&data);
    this->dataSize = data.size();
  }

  /// \brief Set of messages sizes to test (bytes).
  private: std::vector<int> msgSizes =
    {
      256, 512, 1000, 2000, 4000, 8000, 16000, 32000, 64000,
      128000, 256000, 512000, 1000000, 2000000, 4000000
    };

  /// \brief Condition variable used for synchronization.
  private: std::condition_variable condition;

  /// \brief Mutex used for synchronization.
  private: std::mutex mutex;

  /// \brief Message that is sent.
  private: gz::msgs::Bytes msg;

  /// \brief Size of the message currently under test
  private: uint64_t dataSize = 0;

  /// \brief Total bytes received, used for throughput testing
  private: uint64_t totalBytes = 0;

  /// \brief Total messages received, used for throughput testing
  private: uint64_t msgCount = 0;

  /// \brief Number of test iterations.
  private: uint64_t sentMsgs = 100;

  /// \brief Communication node
  private: gz::transport::Node node;

  /// \brief Throughput publisher
  private: gz::transport::Node::Publisher throughputPub;

  /// \brief Latency publisher
  private: gz::transport::Node::Publisher latencyPub;

  /// \brief Used to stop the test.
  private: bool stop = false;

  /// \brief End time point.
  private: std::chrono::time_point<std::chrono::high_resolution_clock> timeEnd;

  /// \brief Output filename or empty string for console output.
  private: std::string filename;

  private: int expectedStamp = 0;
};

// The PubTester is global so that the signal handler can easily kill it.
// Ugly, but fine for this example.
PubTester gPubTester;

//////////////////////////////////////////////////
void signalHandler(int _signal)
{
  if (_signal == SIGINT || _signal == SIGTERM)
  {
    gStop = true;
    gCondition.notify_all();
    gPubTester.Stop();
  }
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Install a signal handler for SIGINT and SIGTERM.
  std::signal(SIGINT,  signalHandler);
  std::signal(SIGTERM, signalHandler);

  // Simple usage.
  std::string usage = "Benchmark testing program."
    " Usage:\n ./bench <options>\n\n"
    " Example intraprocess latency:\n\t./bench -l\n"
    " Example interprocess latency:\n"
    " \tTerminal 1: ./bench -l -r\n"
    " \tTerminal 2: ./bench -l -p\n"
    " Example intraprocess throughput:\n\t./bench -t\n"
    " Example interprocess throughput:\n"
    " \tTerminal 1: ./bench -t -r\n"
    " \tTerminal 2: ./bench -t -p\n";

  gflags::SetUsageMessage(usage);

  // Parse command line arguments
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);

  // Show help, if specified
  if (FLAGS_h)
  {
    gflags::SetCommandLineOptionWithMode("help", "false",
        gflags::SET_FLAGS_DEFAULT);
    gflags::SetCommandLineOptionWithMode("helpshort", "true",
        gflags::SET_FLAGS_DEFAULT);
  }
  gflags::HandleCommandLineHelpFlags();

  // Set the number of iterations.
  gPubTester.SetIterations(FLAGS_i);
  gPubTester.SetOutputFilename(FLAGS_o);

  // Run the responder
  if (FLAGS_r)
  {
    FloodSub floodSub(FLAGS_f);

    ReplyTester replyTester;
    std::unique_lock<std::mutex> lk(gMutex);
    gCondition.wait(lk, []{return gStop;});
  }
  // Run the publisher
  else if (FLAGS_p)
  {
    FloodPub floodPub(FLAGS_f);

    gPubTester.Init();

    if (FLAGS_t)
      gPubTester.Throughput();
    else
      gPubTester.Latency();
  }
  // Single process with both publisher and responder
  else
  {
    ReplyTester replyTester;
    gPubTester.Init();

    if (FLAGS_t)
      gPubTester.Throughput();
    else
      gPubTester.Latency();
  }
  return 0;
}
