/*
 * Copyright (C) 2014 Open Source Robotics Foundation
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
#include <google/protobuf/text_format.h>
#include "gtest/gtest.h"

#include <gz/msgs/int32.pb.h>
#include <gz/msgs/stringmsg.pb.h>
#include <gz/msgs/vector3d.pb.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "gz/transport/MessageInfo.hh"
#include "gz/transport/Node.hh"
#include "gz/transport/TransportTypes.hh"

#include <gz/utils/Environment.hh>

#include "test_utils.hh"

using namespace gz;

static std::string partition; // NOLINT(*)
static std::string g_FQNPartition; // NOLINT(*)
static std::string g_topic = "/foo"; // NOLINT(*)
static std::string g_topic_remap = "/bar"; // NOLINT(*)
static std::mutex exitMutex;
static std::mutex cbMutex;
static std::condition_variable cbCondition;

static int data = 5;
static bool cbExecuted;
static bool cb2Executed;
static bool genericCbExecuted;
static bool cbVectorExecuted;
static bool srvExecuted;
static bool responseExecuted;
static bool wrongResponseExecuted;
static int counter = 0;
static bool terminatePub = false;

//////////////////////////////////////////////////
/// \brief Initialize some global variables.
void reset()
{
  cbExecuted = false;
  cb2Executed = false;
  genericCbExecuted = false;
  srvExecuted = false;
  cbVectorExecuted = false;
  responseExecuted = false;
  wrongResponseExecuted = false;
  counter = 0;
  terminatePub = false;
}

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
void cb(const msgs::Int32 &_msg)
{
  EXPECT_EQ(_msg.data(), data);
  cbExecuted = true;
  ++counter;

  std::lock_guard<std::mutex> lk(cbMutex);
  cbCondition.notify_all();
}

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
void cb2(const msgs::Int32 &_msg)
{
  EXPECT_EQ(_msg.data(), data);
  cb2Executed = true;
}

//////////////////////////////////////////////////
/// \brief Function called each time a topic update is received.
/// This callback includes message information.
void cbInfo(const msgs::Int32 &_msg,
            const transport::MessageInfo &_info)
{
  EXPECT_EQ(_info.Topic(), g_topic);
  EXPECT_EQ(_msg.data(), data);
  EXPECT_EQ(g_FQNPartition, _info.Partition());
  EXPECT_EQ(_msg.GetTypeName(), _info.Type());
  EXPECT_TRUE(_info.IntraProcess());
  cbExecuted = true;
  ++counter;
}

//////////////////////////////////////////////////
void rawCbInfo(const char *_msgData, const size_t _size,
               const transport::MessageInfo &_info)
{
  EXPECT_EQ(_info.Topic(), g_topic);
  EXPECT_EQ(g_FQNPartition, _info.Partition());
  EXPECT_TRUE(_info.IntraProcess());
  cbExecuted = true;

  msgs::Int32 msg;
  EXPECT_TRUE(msg.ParseFromArray(_msgData, _size));
  EXPECT_EQ(msg.data(), data);

  ++counter;
}

//////////////////////////////////////////////////
/// \brief A generic callback.
void genericCb(const transport::ProtoMsg &_msg)
{
  std::string content;
  ASSERT_TRUE(google::protobuf::TextFormat::PrintToString(_msg, &content));
  EXPECT_TRUE(content.find(std::to_string(data)) != std::string::npos);
  genericCbExecuted = true;
}

//////////////////////////////////////////////////
/// \brief Provide a service call.
bool srvEcho(const msgs::Int32 &_req,
  msgs::Int32 &_rep)
{
  srvExecuted = true;

  EXPECT_EQ(_req.data(), data);
  _rep.set_data(_req.data());
  return true;
}

//////////////////////////////////////////////////
/// \brief Provide a service call without input.
bool srvWithoutInput(msgs::Int32 &_rep)
{
  srvExecuted = true;
  _rep.set_data(data);
  return true;
}

//////////////////////////////////////////////////
/// \brief Provide a service call without waiting for response.
void srvWithoutOutput(const msgs::Int32 &_req)
{
  srvExecuted = true;

  EXPECT_EQ(_req.data(), data);
  ++counter;
}

//////////////////////////////////////////////////
/// \brief Service call response callback.
void response(const msgs::Int32 &_rep, const bool _result)
{
  EXPECT_EQ(_rep.data(), data);
  EXPECT_TRUE(_result);

  responseExecuted = true;
  ++counter;
}

//////////////////////////////////////////////////
/// \brief Service call response callback.
void wrongResponse(const msgs::Vector3d &/*_rep*/, bool /*_result*/)
{
  wrongResponseExecuted = true;
}

//////////////////////////////////////////////////
/// \brief Callback for receiving Vector3d data.
void cbVector(const msgs::Vector3d &/*_msg*/)
{
  cbVectorExecuted = true;
}

//////////////////////////////////////////////////
/// \brief A class for testing subscription, advertisement, and request passing
/// a member function as a callback.

class MyTestClass
{
  /// \brief Class constructor.
  public: MyTestClass()
    : callbackExecuted(false),
      callbackSrvExecuted(false),
      responseExecuted(false)
  {
  }

  /// \brief Create a subscriber.
  public: void Subscribe()
  {
    // Subscribe to an illegal topic.
    EXPECT_FALSE(this->node.Subscribe("Bad Topic", &MyTestClass::Cb, this));

    EXPECT_TRUE(this->node.Subscribe(g_topic, &MyTestClass::Cb, this));
  }

  /// \brief Create a subscriber with a callback that receives message info.
  public: void SubscribeWithMessageInfo()
  {
    // Subscribe to an illegal topic.
    EXPECT_FALSE(this->node.Subscribe("Bad Topic", &MyTestClass::CbInfo, this));

    EXPECT_TRUE(this->node.Subscribe(g_topic, &MyTestClass::CbInfo, this));
  }

  /// \brief Member function used as a callback for responding to a service
  /// call.
  public: bool Echo(const msgs::Int32 &_req,
    msgs::Int32 &_rep)
  {
    EXPECT_EQ(_req.data(), data);
    _rep.set_data(_req.data());
    this->callbackSrvExecuted = true;
    return true;
  }

  /// \brief Member function used as a callback for responding to a service
  /// call without input.
  public: bool WithoutInput(msgs::Int32 &_rep)
  {
    _rep.set_data(data);
    this->callbackSrvExecuted = true;
    return true;
  }

  // Member function used as a callback for responding to a service call
  // without without waiting for a response.
  public: void WithoutOutput(const msgs::Int32 &_req)
  {
    EXPECT_EQ(_req.data(), data);
    this->callbackSrvExecuted = true;
  }

  /// \brief Response callback to a service request.
  public: void EchoResponse(const msgs::Int32 &_rep,
    const bool _result)
  {
    EXPECT_EQ(_rep.data(), data);
    EXPECT_TRUE(_result);

    this->responseExecuted = true;
  }

  /// \brief Response callback to a service request without input.
  public: void WithoutInputResponse(const msgs::Int32 &_rep,
    const bool _result)
  {
    EXPECT_EQ(_rep.data(), data);
    EXPECT_TRUE(_result);

    this->responseExecuted = true;
  }

  /// \brief Member function called each time a topic update is received.
  public: void Cb(const msgs::Int32 &_msg)
  {
    EXPECT_EQ(_msg.data(), data);
    this->callbackExecuted = true;
  };

  /// \brief Member function called each time a topic update is received.
  /// This callback accepts a parameter with some message information.
  public: void CbInfo(const msgs::Int32 &_msg,
                      const transport::MessageInfo &_info)
  {
    EXPECT_EQ(_info.Topic(), g_topic);
    EXPECT_EQ(_msg.data(), data);
    EXPECT_EQ(g_FQNPartition, _info.Partition());
    EXPECT_EQ(_msg.GetTypeName(), _info.Type());
    EXPECT_TRUE(_info.IntraProcess());
    this->callbackExecuted = true;
  };

  /// \brief Advertise a topic and publish a message.
  public: void SendSomeData()
  {
    msgs::Int32 msg;
    msg.set_data(data);

    // Advertise an illegal topic.
    auto pub = this->node.Advertise<msgs::Int32>("invalid topic");
    EXPECT_FALSE(pub);
    EXPECT_FALSE(pub.HasConnections());

    auto pubId = this->node.Advertise<msgs::Int32>("invalid topic");
    EXPECT_FALSE(pubId);
    EXPECT_FALSE(pubId.Valid());

    pubId = this->node.Advertise<msgs::Int32>(g_topic);
    EXPECT_TRUE(pubId);
    EXPECT_TRUE(pubId.Valid());
    EXPECT_TRUE(pubId.HasConnections());
    EXPECT_TRUE(pubId.Publish(msg));
  }

  /// \brief Advertise a service, request a service using non-blocking and
  /// blocking call.
  public: void TestServiceCall()
  {
    msgs::Int32 req;
    msgs::Int32 rep;
    msgs::Vector3d wrongReq;
    msgs::Vector3d wrongRep;
    int timeout = 500;
    bool result;

    req.set_data(data);

    this->Reset();

    // Advertise an illegal service name.
    EXPECT_FALSE(this->node.Advertise("Bad Srv", &MyTestClass::Echo, this));

    // Advertise and request a valid service.
    EXPECT_TRUE(this->node.Advertise(g_topic, &MyTestClass::Echo, this));
    EXPECT_TRUE(this->node.Request(g_topic, req, timeout, rep, result));
    ASSERT_TRUE(result);
    EXPECT_EQ(rep.data(), data);
    EXPECT_TRUE(this->callbackSrvExecuted);

    this->Reset();

    // Request a valid service using a member function callback.
    this->node.Request(g_topic, req, &MyTestClass::EchoResponse, this);
    EXPECT_TRUE(this->responseExecuted);

    this->Reset();

    // Request a valid service using RequestRaw.
    std::string reqStr, repStr, repTypeName;
    EXPECT_TRUE(req.SerializeToString(&reqStr));
    EXPECT_TRUE(this->node.RequestRaw(g_topic, reqStr,
          std::string(req.GetTypeName()),
          std::string(rep.GetTypeName()), timeout, repStr, result));
    EXPECT_TRUE(rep.ParseFromString(repStr));
    ASSERT_TRUE(result);
    EXPECT_EQ(rep.data(), data);
    EXPECT_TRUE(this->callbackSrvExecuted);

    this->Reset();

    // Service requests with wrong types.
    EXPECT_FALSE(this->node.Request(g_topic, wrongReq, timeout, rep, result));
    EXPECT_FALSE(this->node.Request(g_topic, req, timeout, wrongRep, result));
    EXPECT_TRUE(this->node.Request(g_topic, wrongReq, response));
    EXPECT_TRUE(this->node.Request(g_topic, req, wrongResponse));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_FALSE(this->callbackSrvExecuted);
  }

  /// \brief Advertise a service without input, request a service without input
  /// using non-blocking and blocking call.
  public: void TestServiceCallWithoutInput()
  {
    msgs::Int32 rep;
    msgs::Vector3d wrongRep;
    int timeout = 500;
    bool result;

    this->Reset();

    // Advertise an illegal service name without input.
    EXPECT_FALSE(this->node.Advertise("Bad Srv", &MyTestClass::WithoutInput,
      this));

    // Advertise and request a valid service without input.
    EXPECT_TRUE(this->node.Advertise(g_topic, &MyTestClass::WithoutInput,
      this));
    EXPECT_TRUE(this->node.Request(g_topic, timeout, rep, result));
    ASSERT_TRUE(result);
    EXPECT_EQ(rep.data(), data);
    EXPECT_TRUE(this->callbackSrvExecuted);

    this->Reset();

    // Request a valid service without input using a member function callback.
    this->node.Request(g_topic, &MyTestClass::WithoutInputResponse, this);
    EXPECT_TRUE(this->responseExecuted);

    this->Reset();

    // Service requests without input with wrong types.
    EXPECT_FALSE(this->node.Request(g_topic, timeout, wrongRep, result));
    EXPECT_TRUE(this->node.Request(g_topic, wrongResponse));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_FALSE(this->callbackSrvExecuted);
  }

  /// \brief Advertise and request a service without waiting for response.
  public: void TestServiceCallWithoutOutput()
  {
    msgs::Int32 req;
    msgs::Vector3d wrongReq;

    req.set_data(data);

    this->Reset();

    // Advertise an illegal service name without waiting for response.
    EXPECT_FALSE(this->node.Advertise("Bad Srv", &MyTestClass::WithoutOutput,
                                      this));

    // Advertise and request a valid service without waiting for response.
    EXPECT_TRUE(this->node.Advertise(g_topic, &MyTestClass::WithoutOutput,
                                     this));
    EXPECT_TRUE(this->node.Request(g_topic, req));
    EXPECT_TRUE(this->callbackSrvExecuted);

    this->Reset();

    // Service requests without waiting for response with wrong types.
    EXPECT_TRUE(this->node.Request(g_topic, wrongReq));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_FALSE(this->callbackSrvExecuted);
  }

  /// \brief Advertise and request a service without waiting for response.
  public: void TestServiceCallRequestingBeforeAdvertising()
  {
    msgs::Int32 req;
    req.set_data(data);

    this->Reset();

    // Request a valid service using a member function callback.
    this->node.Request(g_topic, req, &MyTestClass::EchoResponse, this);

    // Advertise and request a valid service.
    EXPECT_TRUE(this->node.Advertise(g_topic, &MyTestClass::Echo, this));
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_TRUE(this->responseExecuted);
  }

  public: void Reset()
  {
    this->callbackExecuted = false;
    this->callbackSrvExecuted = false;
    this->responseExecuted = false;
  }

  /// \brief Member variables that flag when the actions are executed.
  public: bool callbackExecuted;
  public: bool callbackSrvExecuted;
  public: bool responseExecuted;

  /// \brief Transport node;
  private: transport::Node node;
};

//////////////////////////////////////////////////
/// \brief Create a subscriber and wait for a callback to be executed.
void CreateSubscriber(const transport::NodeOptions &_nodeOptions)
{
  transport::Node node(_nodeOptions);
  EXPECT_TRUE(node.Subscribe(g_topic, cb));

  int i = 0;
  while (i < 100 && !cbExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }
}

//////////////////////////////////////////////////
/// \brief Use two threads using their own transport nodes. One thread
/// will publish a message, whereas the other thread is subscribed to the topic.
/// \param[in] _scope Scope used to advertise the topic.
void CreatePubSubTwoThreads(
  const transport::NodeOptions &_nodeOptions,
  const transport::Scope_t &_sc = transport::Scope_t::ALL)
{
  reset();

  transport::AdvertiseMessageOptions opts;
  opts.SetScope(_sc);

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node(_nodeOptions);
  auto pub = node.Advertise<msgs::Int32>(g_topic, opts);
  EXPECT_TRUE(pub);

  // Subscribe to a topic in a different thread and wait until the callback is
  // received.
  std::thread subscribeThread(CreateSubscriber, _nodeOptions);

  // Wait some time until the subscriber is alive.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a msg on topic.
  EXPECT_TRUE(pub.Publish(msg));

  // Wait until the subscribe thread finishes.
  subscribeThread.join();

  // Check that the message was received.
  EXPECT_TRUE(cbExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief Test the bool operator of the Node::Publisher class
TEST(NodePubTest, BoolOperatorTest)
{
  transport::Node node;
  transport::Node node2;
  transport::Node::Publisher pub;
  const transport::Node::Publisher pub_const;
  EXPECT_FALSE(pub);
  EXPECT_FALSE(pub_const);
  pub = node.Advertise<msgs::Vector3d>(g_topic);
  const transport::Node::Publisher pub2_const =
      node2.Advertise<msgs::Vector3d>(g_topic);
  EXPECT_TRUE(pub);
  EXPECT_TRUE(pub2_const);
}

//////////////////////////////////////////////////
/// \brief A message should not be published if it is not advertised before.
TEST(NodeTest, PubWithoutAdvertise)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  // Check that an invalid namespace is ignored. The callbacks are expecting an
  // empty namespace.
  transport::NodeOptions optionsNode1;
  transport::NodeOptions optionsNode2;
  optionsNode1.SetPartition(partition);
  optionsNode1.SetNameSpace("invalid namespace");
  optionsNode2.SetPartition(partition);
  transport::Node node1(optionsNode1);
  transport::Node node2(optionsNode2);

  // Check the advertised/subscribed topics and advertised services.
  EXPECT_TRUE(node1.AdvertisedTopics().empty());
  EXPECT_TRUE(node1.SubscribedTopics().empty());
  EXPECT_TRUE(node1.AdvertisedServices().empty());

  auto pub1 = node1.Advertise(g_topic, std::string(msg.GetTypeName()));
  EXPECT_TRUE(pub1);

  auto advertisedTopics = node1.AdvertisedTopics();
  ASSERT_EQ(advertisedTopics.size(), 1u);
  EXPECT_EQ(advertisedTopics.at(0), g_topic);

  auto pub2 = node2.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub2);
  advertisedTopics = node2.AdvertisedTopics();
  ASSERT_EQ(advertisedTopics.size(), 1u);
  EXPECT_EQ(advertisedTopics.at(0), g_topic);

  EXPECT_TRUE(node2.Subscribe(g_topic, cb));
  auto subscribedTopics = node2.SubscribedTopics();
  ASSERT_EQ(subscribedTopics.size(), 1u);
  EXPECT_EQ(subscribedTopics.at(0), g_topic);

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::unique_lock<std::mutex> lk(cbMutex);
  // Publish a message by each node.
  EXPECT_TRUE(pub1.Publish(msg));

  // Wait for the messages to arrive.
  cbCondition.wait(lk, []{return counter >= 1;});

  EXPECT_TRUE(pub2.Publish(msg));
  cbCondition.wait(lk, []{return counter >= 2;});

  // Check that the msg was received twice.
  EXPECT_TRUE(cbExecuted);
  EXPECT_EQ(counter, 2);

  reset();
}

//////////////////////////////////////////////////
/// \brief A thread can create a node, and send and receive messages.
TEST(NodeTest, PubSubSameThread)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;

  // Advertise an illegal topic.
  EXPECT_FALSE(node.Advertise<msgs::Int32>("invalid topic"));

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  // Subscribe to an illegal topic.
  EXPECT_FALSE(node.Subscribe("invalid topic", cb));

  EXPECT_TRUE(node.Subscribe(g_topic, cb));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a first message.
  EXPECT_TRUE(pub.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the message was received.
  EXPECT_TRUE(cbExecuted);

  reset();

  // Publish a second message on topic.
  EXPECT_TRUE(pub.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the data was received.
  EXPECT_TRUE(cbExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief A thread can create a node, and send and receive messages.
TEST(NodeTest, PubSubSameThreadGenericCb)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  EXPECT_TRUE(node.Subscribe(g_topic, genericCb));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a first message.
  EXPECT_TRUE(pub.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the message was received.
  EXPECT_TRUE(genericCbExecuted);

  reset();

  // Publish a second message on topic.
  EXPECT_TRUE(pub.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the data was received.
  EXPECT_TRUE(genericCbExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief A thread can create a node, and send and receive messages.
/// This test uses a callback that accepts a parameter with the message
/// information.
TEST(NodeTest, PubSubSameThreadMessageInfo)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;
  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  EXPECT_TRUE(node.Subscribe(g_topic, cbInfo));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a first message.
  EXPECT_TRUE(pub.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the message was received.
  EXPECT_TRUE(cbExecuted);

  reset();

  // Publish a second message on topic.
  EXPECT_TRUE(pub.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the data was received.
  EXPECT_TRUE(cbExecuted);

  reset();
}

//////////////////////////////////////////////////
TEST(NodeTest, RawPubSubSameThreadMessageInfo)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;
  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  EXPECT_TRUE(node.Subscribe(g_topic, cbInfo));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a first message.
  EXPECT_TRUE(pub.PublishRaw(msg.SerializeAsString(),
        std::string(msg.GetTypeName())));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the message was received.
  EXPECT_TRUE(cbExecuted);

  reset();

  // Publish a second message on topic.
  EXPECT_TRUE(pub.PublishRaw(msg.SerializeAsString(),
        std::string(msg.GetTypeName())));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the data was received.
  EXPECT_TRUE(cbExecuted);

  reset();
}

//////////////////////////////////////////////////
TEST(NodeTest, RawPubRawSubSameThreadMessageInfo)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;
  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  EXPECT_TRUE(node.SubscribeRaw(g_topic, rawCbInfo));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a first message.
  EXPECT_TRUE(pub.PublishRaw(msg.SerializeAsString(),
        std::string(msg.GetTypeName())));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the message was received.
  EXPECT_TRUE(cbExecuted);

  reset();

  // Publish a second message on topic.
  EXPECT_TRUE(pub.PublishRaw(msg.SerializeAsString(),
        std::string(msg.GetTypeName())));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the data was received.
  EXPECT_TRUE(cbExecuted);

  reset();
}

//////////////////////////////////////////////////
TEST(NodeTest, PubRawSubSameThreadMessageInfo)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;
  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  EXPECT_TRUE(node.SubscribeRaw(g_topic, rawCbInfo));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a first message.
  EXPECT_TRUE(pub.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the message was received.
  EXPECT_TRUE(cbExecuted);

  reset();

  // Publish a second message on topic.
  EXPECT_TRUE(pub.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the data was received.
  EXPECT_TRUE(cbExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief Subscribe to a topic using a lambda function.
TEST(NodeTest, PubSubSameThreadLambda)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  std::mutex mutex;
  std::condition_variable condition;

  bool executed = false;
  std::function<void(const msgs::Int32&)> subCb =
    [&executed, &mutex, &condition](const msgs::Int32 &_msg)
  {
    EXPECT_EQ(_msg.data(), data);
    std::lock_guard<std::mutex> lk(mutex);
    executed = true;
    condition.notify_all();
  };

  EXPECT_TRUE(node.Subscribe(g_topic, subCb));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a first message.
  EXPECT_TRUE(pub.Publish(msg));

  // The local publish is asynchronous, which means we need to wait
  // for the callback.
  std::unique_lock<std::mutex> lk(mutex);
  condition.wait(lk, [&executed]{return executed;});

  EXPECT_TRUE(executed);

  reset();
}

//////////////////////////////////////////////////
/// \brief Subscribe to a topic using a lambda function.
/// This test uses a callback that accepts a parameter with the message
/// information.
TEST(NodeTest, PubSubSameThreadLambdaMessageInfo)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  bool executed = false;
  std::function<void(const msgs::Int32&,
                     const transport::MessageInfo &)> subCb =
    [&executed](const msgs::Int32 &_msg,
                const transport::MessageInfo &_info)
  {
    EXPECT_EQ(_info.Topic(), g_topic);
    EXPECT_EQ(_msg.data(), data);
    EXPECT_EQ(g_FQNPartition, _info.Partition());
    EXPECT_EQ(_msg.GetTypeName(), _info.Type());
    EXPECT_TRUE(_info.IntraProcess());
    std::lock_guard<std::mutex> lk(cbMutex);
    executed = true;
    cbCondition.notify_all();
  };

  EXPECT_TRUE(node.Subscribe(g_topic, subCb));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a first message.
  std::unique_lock<std::mutex> lk(cbMutex);
  EXPECT_TRUE(pub.Publish(msg));
  cbCondition.wait(lk, [&executed]{return executed;});

  EXPECT_TRUE(executed);

  reset();
}

//////////////////////////////////////////////////
/// \brief Test the bool operator of the Node::Subscriber class
TEST(NodeSubTest, BoolOperatorTest)
{
  transport::Node node;
  transport::Node::Subscriber sub;
  const transport::Node::Subscriber sub_const;
  EXPECT_FALSE(sub);
  EXPECT_FALSE(sub_const);

  std::function<void(const msgs::Int32 &)> cb =
    [](const msgs::Int32 &) {};

  sub = node.CreateSubscriber(g_topic, cb);
  EXPECT_TRUE(sub);

  EXPECT_TRUE(sub.Unsubscribe());
  EXPECT_FALSE(sub);

  const transport::Node::Subscriber sub2_const =
      node.CreateSubscriber(g_topic, cb);
  EXPECT_TRUE(sub2_const);
}

//////////////////////////////////////////////////
/// \brief Exercise the Subscriber move constructor.
TEST(NodeTest, MoveSubscriber)
{
  transport::Node node;
  std::vector<transport::Node::Subscriber> subscribers;
  subscribers.emplace_back(node.CreateSubscriber(g_topic, cb));
}

//////////////////////////////////////////////////
/// \brief Subscribe to a topic using CreateSubscriber API
TEST(NodeTest, PubSubWithCreateSubscriber)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  std::mutex mutex;
  std::condition_variable condition;

  bool executed = false;
  std::function<void(const msgs::Int32&)> subCb =
    [&executed, &mutex, &condition](const msgs::Int32 &_msg)
  {
    EXPECT_EQ(_msg.data(), data);
    std::lock_guard<std::mutex> lk(mutex);
    executed = true;
    condition.notify_all();
  };

  {
    transport::Node::Subscriber sub;
    sub = node.CreateSubscriber(g_topic, subCb);
    EXPECT_TRUE(sub);

    // Give some time to the subscribers.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Publish a message.
    EXPECT_TRUE(pub.Publish(msg));

    // The local publish is asynchronous, which means we need to wait
    // for the callback.
    std::unique_lock<std::mutex> lk(mutex);
    condition.wait(lk, [&executed]{return executed;});

    EXPECT_TRUE(executed);
  }

  // Publish another message.
  EXPECT_TRUE(pub.Publish(msg));

  // The subscriber went out of scope so it should have already unsubscribed
  // to the topic. The callback should not be invoked.
  std::unique_lock<std::mutex> lk(mutex);
  condition.wait_for(lk, std::chrono::milliseconds(300),
                     [&executed]{return executed;});

  reset();
}

//////////////////////////////////////////////////
/// \brief Subscribe to a topic using dfferent Subscribe APIs
TEST(NodeTest, PubSubWithMixedSubscribeAPIs)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  // Subscriber1: Subscribe to topic using Subscribe(...)
  std::mutex mutex;
  std::condition_variable condition;
  bool executed = false;
  std::function<void(const msgs::Int32&)> subCb =
    [&executed, &mutex, &condition](const msgs::Int32 &_msg)
  {
    EXPECT_EQ(_msg.data(), data);
    std::lock_guard<std::mutex> lk(mutex);
    executed = true;
    condition.notify_all();
  };
  EXPECT_TRUE(node.Subscribe(g_topic, subCb));

  // Subscriber2: Subscribe to topic using CreateSubscriber(...)
  std::mutex mutex2;
  std::condition_variable condition2;
  bool executed2 = false;
  std::function<void(const msgs::Int32&)> subCb2 =
    [&executed2, &mutex2, &condition2](const msgs::Int32 &_msg)
  {
    EXPECT_EQ(_msg.data(), data);
    std::lock_guard<std::mutex> lk2(mutex2);
    executed2 = true;
    condition2.notify_all();
  };
  transport::Node::Subscriber sub2 = node.CreateSubscriber(g_topic, subCb2);
  EXPECT_TRUE(sub2);

  // Subscriber3: Subscribe to topic using CreateSubscriber(...)
  std::mutex mutex3;
  std::condition_variable condition3;
  bool executed3 = false;
  std::function<void(const msgs::Int32&)> subCb3 =
    [&executed3, &mutex3, &condition3](const msgs::Int32 &_msg)
  {
    EXPECT_EQ(_msg.data(), data);
    std::lock_guard<std::mutex> lk2(mutex3);
    executed3 = true;
    condition3.notify_all();
  };
  transport::Node::Subscriber sub3 = node.CreateSubscriber(g_topic, subCb3);
  EXPECT_TRUE(sub3);

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a message.
  EXPECT_TRUE(pub.Publish(msg));

  // The local publish is asynchronous, which means we need to wait
  // for the callback.
  {
    std::unique_lock<std::mutex> lk(mutex);
    condition.wait(lk, [&executed]{return executed;});
    EXPECT_TRUE(executed);

    std::unique_lock<std::mutex> lk2(mutex2);
    condition2.wait(lk2, [&executed2]{return executed2;});
    EXPECT_TRUE(executed2);

    std::unique_lock<std::mutex> lk3(mutex3);
    condition3.wait(lk3, [&executed3]{return executed3;});
    EXPECT_TRUE(executed3);
  }

  executed = false;
  executed2 = false;
  executed3 = false;

  // Manually unsubscribe Subscriber2 and verify that the other subscribers
  // still receive messages
  EXPECT_TRUE(sub2.Unsubscribe());

  // Publish another message.
  EXPECT_TRUE(pub.Publish(msg));

  {
    // Subscriber1 should still receive msgs
    std::unique_lock<std::mutex> lk(mutex);
    condition.wait(lk, [&executed]{return executed;});
    EXPECT_TRUE(executed);

    // Subscriber2 should no longer receive msgs
    std::unique_lock<std::mutex> lk2(mutex2);
    condition2.wait_for(lk2, std::chrono::milliseconds(300),
                        [&executed2]{return executed2;});
    EXPECT_FALSE(executed2);

    // Subscriber3 should still receive msgs
    std::unique_lock<std::mutex> lk3(mutex3);
    condition.wait(lk3, [&executed3]{return executed3;});
    EXPECT_TRUE(executed3);
  }

  executed = false;
  executed2 = false;
  executed3 = false;

  // Unsubscribe node from topic and verify all subscribers no longer receive
  // messages
  EXPECT_TRUE(node.Unsubscribe(g_topic));

  // Publish another message.
  EXPECT_TRUE(pub.Publish(msg));

  {
    std::unique_lock<std::mutex> lk(mutex);
    condition.wait_for(lk, std::chrono::milliseconds(300),
                       [&executed]{return executed;});
    EXPECT_FALSE(executed);

    std::unique_lock<std::mutex> lk2(mutex2);
    condition2.wait_for(lk2, std::chrono::milliseconds(300),
                        [&executed2]{return executed2;});
    EXPECT_FALSE(executed2);

    std::unique_lock<std::mutex> lk3(mutex3);
    condition3.wait_for(lk3, std::chrono::milliseconds(300),
                        [&executed3]{return executed3;});
    EXPECT_FALSE(executed3);
  }

  reset();
}

//////////////////////////////////////////////////
/// \brief Advertise two topics with the same name. It's not possible to do it
/// within the same node but it's valid on separate nodes.
TEST(NodeTest, AdvertiseTwoEqualTopics)
{
  transport::Node node1;
  transport::Node node2;

  auto pub1 = node1.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub1);
  auto pub2 = node1.Advertise<msgs::StringMsg>(g_topic);
  EXPECT_FALSE(pub2);
  auto pub3 = node2.Advertise<msgs::StringMsg>(g_topic);
  EXPECT_TRUE(pub3);
}

//////////////////////////////////////////////////
/// \brief Use two threads using their own transport nodes. One thread
/// will publish a message, whereas the other thread is subscribed to the topic.
TEST(NodeTest, PubSubTwoThreadsSameTopic)
{
  transport::NodeOptions options;
  CreatePubSubTwoThreads(options);
}

//////////////////////////////////////////////////
/// \brief Check that two nodes in different threads are able to communicate
/// advertising a topic with "Process" scope.
TEST(NodeTest, ScopeProcess)
{
  transport::NodeOptions options;
  CreatePubSubTwoThreads(options, transport::Scope_t::PROCESS);
}

//////////////////////////////////////////////////
/// \brief Check that two nodes in different threads are able to communicate
/// advertising a topic with "Host" scope.
TEST(NodeTest, ScopeHost)
{
  transport::NodeOptions options;
  CreatePubSubTwoThreads(options, transport::Scope_t::HOST);
}

//////////////////////////////////////////////////
/// \brief Check that two nodes in different threads are able to communicate
/// advertising a topic with "All" scope.
TEST(NodeTest, ScopeAll)
{
  transport::NodeOptions options;
  CreatePubSubTwoThreads(options, transport::Scope_t::ALL);
}

//////////////////////////////////////////////////
/// \brief Use two threads using their own transport nodes. One thread
/// will publish a message, whereas the other thread is subscribed to the topic.
/// Topic remapping is enabled.
TEST(NodeTest, PubSubTwoThreadsSameTopicRemap)
{
  transport::NodeOptions options;
  options.AddTopicRemap(g_topic, g_topic_remap);
  CreatePubSubTwoThreads(options);
}

//////////////////////////////////////////////////
/// \brief Check that two nodes in different threads are able to communicate
/// advertising a topic with "Process" scope.
/// Topic remapping is enabled.
TEST(NodeTest, ScopeProcessRemap)
{
  transport::NodeOptions options;
  options.AddTopicRemap(g_topic, g_topic_remap);
  CreatePubSubTwoThreads(options, transport::Scope_t::PROCESS);
}

//////////////////////////////////////////////////
/// \brief Check that two nodes in different threads are able to communicate
/// advertising a topic with "Host" scope.
/// Topic remapping is enabled.
TEST(NodeTest, ScopeHostRemap)
{
  transport::NodeOptions options;
  options.AddTopicRemap(g_topic, g_topic_remap);
  CreatePubSubTwoThreads(options, transport::Scope_t::HOST);
}

//////////////////////////////////////////////////
/// \brief Check that two nodes in different threads are able to communicate
/// advertising a topic with "All" scope.
/// Topic remapping is enabled.
TEST(NodeTest, ScopeAllRemap)
{
  transport::NodeOptions options;
  options.AddTopicRemap(g_topic, g_topic_remap);
  CreatePubSubTwoThreads(options, transport::Scope_t::ALL);
}

//////////////////////////////////////////////////
/// \brief Use two different transport nodes on the same thread. Check that
/// both receive the updates when they are subscribed to the same topic. Check
/// also that when one of the nodes unsubscribes, no longer receives updates.
TEST(NodeTest, PubSubOneThreadTwoSubs)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node1;
  transport::Node node2;

  {
    auto pub1 = node1.Advertise<msgs::Int32>(g_topic);
    EXPECT_TRUE(pub1);

    // Subscribe to topic in node1.
    EXPECT_TRUE(node1.Subscribe(g_topic, cb));

    // Subscribe to topic in node2.
    EXPECT_TRUE(node2.Subscribe(g_topic, cb2));

    // Wait some time before publishing.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_TRUE(pub1.Publish(msg));

    // Give some time to the subscribers.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check that the msg was received by node1.
    EXPECT_TRUE(cbExecuted);
    // Check that the msg was received by node2.
    EXPECT_TRUE(cb2Executed);

    auto subscribedTopics = node1.SubscribedTopics();
    ASSERT_EQ(subscribedTopics.size(), 1u);
    EXPECT_EQ(subscribedTopics.at(0), g_topic);

    reset();

    // Try to unsubscribe from an invalid topic.
    EXPECT_FALSE(node1.Unsubscribe("invalid topic"));

    // Node1 is not interested in the topic anymore.
    EXPECT_TRUE(node1.Unsubscribe(g_topic));

    // Give some time to receive the unsubscription.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Publish a second message.
    EXPECT_TRUE(pub1.Publish(msg));

    // Give some time to the subscribers.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Check that the msg was not received by node1.
    EXPECT_FALSE(cbExecuted);
    // Check that the msg was received by node2.
    EXPECT_TRUE(cb2Executed);

    ASSERT_TRUE(node1.SubscribedTopics().empty());

    reset();
  }

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto advertisedTopics = node1.AdvertisedTopics();
  ASSERT_TRUE(advertisedTopics.empty());

  reset();
}

//////////////////////////////////////////////////
/// \brief Use the transport inside a class and check advertise, subscribe and
/// publish.
TEST(NodeTest, ClassMemberCallbackMessage)
{
  MyTestClass client;
  client.Subscribe();

  // Wait for the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  client.SendSomeData();

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_TRUE(client.callbackExecuted);
}

//////////////////////////////////////////////////
/// \brief Use the transport inside a class and check advertise, subscribe and
/// publish. This test uses a callback that accepts message information.
TEST(NodeTest, ClassMemberCallbackMessageInfo)
{
  MyTestClass client;
  client.SubscribeWithMessageInfo();

  // Wait for the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  client.SendSomeData();

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_TRUE(client.callbackExecuted);
}

//////////////////////////////////////////////////
/// \brief Make an asynchronous and synchronous service calls using member
/// function.
TEST(NodeTest, ClassMemberCallbackService)
{
  MyTestClass client;
  client.TestServiceCall();
}

//////////////////////////////////////////////////
/// \brief Make an asynchronous and synchronous service calls without input
/// using member function.
TEST(NodeTest, ClassMemberCallbackServiceWithoutInput)
{
  MyTestClass client;
  client.TestServiceCallWithoutInput();
}

//////////////////////////////////////////////////
/// \brief Make an asynchronous service call, and then, advertise the service.
TEST(NodeTest, ClassMemberRequestServiceBeforeAdvertise)
{
  MyTestClass client;
  client.TestServiceCallRequestingBeforeAdvertising();
}

//////////////////////////////////////////////////
/// \brief Check that the types advertised and published match.
TEST(NodeTest, TypeMismatch)
{
  reset();

  msgs::Int32 rightMsg;
  msgs::Vector3d wrongMsg;
  rightMsg.set_data(1);
  wrongMsg.set_x(1);
  wrongMsg.set_y(2);
  wrongMsg.set_z(3);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  EXPECT_FALSE(pub.Publish(wrongMsg));
  EXPECT_TRUE(pub.Publish(rightMsg));

  reset();
}

//////////////////////////////////////////////////
/// \brief Make an asynchronous service call using free function.
TEST(NodeTest, ServiceCallAsync)
{
  reset();

  msgs::Int32 req;
  req.set_data(data);

  transport::Node node;

  // Advertise an invalid service name.
  EXPECT_FALSE(node.Advertise("invalid service", srvEcho));

  EXPECT_TRUE(node.Advertise(g_topic, srvEcho));

  auto advertisedServices = node.AdvertisedServices();
  ASSERT_EQ(advertisedServices.size(), 1u);
  EXPECT_EQ(advertisedServices.at(0), g_topic);

  // Request an invalid service name.
  EXPECT_FALSE(node.Request("invalid service", req, response));

  EXPECT_TRUE(node.Request(g_topic, req, response));

  int i = 0;
  while (i < 100 && !srvExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 1);

  // Make another request.
  reset();

  EXPECT_TRUE(node.Request(g_topic, req, response));

  i = 0;
  while (i < 100 && !responseExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 1);

  // Try to unadvertise an invalid service.
  EXPECT_FALSE(node.UnadvertiseSrv("invalid service"));

  EXPECT_TRUE(node.UnadvertiseSrv(g_topic));

  ASSERT_TRUE(node.AdvertisedServices().empty());

  reset();
}

//////////////////////////////////////////////////
/// \brief Make an asynchronous service call without input using free function.
TEST(NodeTest, ServiceCallWithoutInputAsync)
{
  reset();

  transport::Node node;

  // Advertise an invalid service name.
  EXPECT_FALSE(node.Advertise("invalid service", srvWithoutInput));

  EXPECT_TRUE(node.Advertise(g_topic, srvWithoutInput));

  auto advertisedServices = node.AdvertisedServices();
  ASSERT_EQ(advertisedServices.size(), 1u);
  EXPECT_EQ(advertisedServices.at(0), g_topic);

  // Request an invalid service name.
  EXPECT_FALSE(node.Request("invalid service", response));

  EXPECT_TRUE(node.Request(g_topic, response));

  int i = 0;
  while (i < 100 && !srvExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 1);

  // Make another request.
  reset();

  EXPECT_TRUE(node.Request(g_topic, response));

  i = 0;
  while (i < 100 && !responseExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 1);

  // Try to unadvertise an invalid service.
  EXPECT_FALSE(node.UnadvertiseSrv("invalid service"));

  EXPECT_TRUE(node.UnadvertiseSrv(g_topic));

  ASSERT_TRUE(node.AdvertisedServices().empty());

  reset();
}

//////////////////////////////////////////////////
/// \brief Make an asynchronous service call without waiting for a response
/// \using free function.
TEST(NodeTest, ServiceWithoutOutputCallAsync)
{
  reset();

  transport::Node node;
  msgs::Int32 req;
  req.set_data(data);

  // Advertise an invalid service name.
  EXPECT_FALSE(node.Advertise("invalid service", srvWithoutOutput));

  EXPECT_TRUE(node.Advertise(g_topic, srvWithoutOutput));

  auto advertisedServices = node.AdvertisedServices();
  ASSERT_EQ(advertisedServices.size(), 1u);
  EXPECT_EQ(advertisedServices.at(0), g_topic);

  // Request an invalid service name.
  EXPECT_FALSE(node.Request("invalid service", req));

  EXPECT_TRUE(node.Request(g_topic, req));

  int i = 0;
  while (i < 100 && !srvExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call was executed.
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 1);

  // Try to unadvertise an invalid service.
  EXPECT_FALSE(node.UnadvertiseSrv("invalid service"));

  EXPECT_TRUE(node.UnadvertiseSrv(g_topic));

  ASSERT_TRUE(node.AdvertisedServices().empty());
}

//////////////////////////////////////////////////
/// \brief Make an asynchronous service call using lambdas.
TEST(NodeTest, ServiceCallAsyncLambda)
{
  reset();

  std::function<bool(const msgs::Int32 &, msgs::Int32 &)>
    advCb = [](const msgs::Int32 &_req, msgs::Int32 &_rep)
    -> bool
  {
    EXPECT_EQ(_req.data(), data);
    _rep.set_data(_req.data());
    return true;
  };

  transport::Node node;
  EXPECT_TRUE((node.Advertise<msgs::Int32,
        msgs::Int32>(g_topic, advCb)));

  bool executed = false;
  std::function<void(const msgs::Int32 &, const bool)> reqCb =
    [&executed](const msgs::Int32 &_rep, const bool _result)
  {
    EXPECT_EQ(_rep.data(), data);
    EXPECT_TRUE(_result);
    executed = true;
  };

  msgs::Int32 req;
  req.set_data(data);

  EXPECT_TRUE((node.Request(g_topic, req, reqCb)));

  EXPECT_TRUE(executed);

  reset();
}

//////////////////////////////////////////////////
/// \brief Make an asynchronous service call without input using lambdas.
TEST(NodeTest, ServiceCallWithoutInputAsyncLambda)
{
  reset();

  std::function<bool(msgs::Int32 &)> advCb =
    [](msgs::Int32 &_rep) -> bool
  {
    _rep.set_data(data);
    return true;
  };

  transport::Node node;
  EXPECT_TRUE((node.Advertise<msgs::Int32>(g_topic, advCb)));

  bool executed = false;
  std::function<void(const msgs::Int32 &, const bool)> reqCb =
    [&executed](const msgs::Int32 &_rep, const bool _result)
  {
    EXPECT_EQ(_rep.data(), data);
    EXPECT_TRUE(_result);
    executed = true;
  };

  EXPECT_TRUE((node.Request(g_topic, reqCb)));

  EXPECT_TRUE(executed);

  reset();
}

//////////////////////////////////////////////////
/// \Make an asynchronous service call without waiting for response using
/// \lambdas.
TEST(NodeTest, ServiceCallWithoutOutputAsyncLambda)
{
  bool executed = false;

  std::function<void(const msgs::Int32 &)> advCb =
    [&executed](const msgs::Int32 &_req)
  {
    EXPECT_EQ(_req.data(), data);
    executed = true;
  };

  transport::Node node;
  EXPECT_TRUE((node.Advertise<msgs::Int32>(g_topic, advCb)));

  msgs::Int32 req;
  req.set_data(data);

  EXPECT_TRUE(node.Request(g_topic, req));
  EXPECT_TRUE(executed);
}

//////////////////////////////////////////////////
/// \brief Request multiple service calls at the same time.
TEST(NodeTest, MultipleServiceCallAsync)
{
  reset();

  msgs::Int32 req;
  req.set_data(data);

  transport::Node node;

  // Advertise an invalid service name.
  EXPECT_FALSE(node.Advertise("invalid service", srvEcho));

  EXPECT_TRUE(node.Advertise(g_topic, srvEcho));

  // Request an invalid service name.
  EXPECT_FALSE(node.Request("invalid service", req, response));

  EXPECT_TRUE(node.Request(g_topic, req, response));

  int i = 0;
  while (i < 100 && !srvExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 1);

  // Make another request.
  reset();

  EXPECT_TRUE(node.Request(g_topic, req, response));
  EXPECT_TRUE(node.Request(g_topic, req, response));
  EXPECT_TRUE(node.Request(g_topic, req, response));

  i = 0;
  while (i < 100 && counter < 3)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 3);

  // Try to unadvertise an invalid service.
  EXPECT_FALSE(node.UnadvertiseSrv("invalid service"));

  EXPECT_TRUE(node.UnadvertiseSrv(g_topic));

  reset();
}

//////////////////////////////////////////////////
/// \brief Request multiple service calls without input at the same time.
TEST(NodeTest, MultipleServiceCallWithoutInputAsync)
{
  reset();

  transport::Node node;

  // Advertise an invalid service name.
  EXPECT_FALSE(node.Advertise("invalid service", srvWithoutInput));

  EXPECT_TRUE(node.Advertise(g_topic, srvWithoutInput));

  // Request an invalid service name.
  EXPECT_FALSE(node.Request("invalid service", response));

  EXPECT_TRUE(node.Request(g_topic, response));

  int i = 0;
  while (i < 100 && !srvExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 1);

  // Make another request.
  reset();

  EXPECT_TRUE(node.Request(g_topic, response));
  EXPECT_TRUE(node.Request(g_topic, response));
  EXPECT_TRUE(node.Request(g_topic, response));

  i = 0;
  while (i < 100 && counter < 3)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call response was executed.
  EXPECT_TRUE(responseExecuted);
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 3);

  // Try to unadvertise an invalid service.
  EXPECT_FALSE(node.UnadvertiseSrv("invalid service"));

  EXPECT_TRUE(node.UnadvertiseSrv(g_topic));

  reset();
}

/// \brief Request multiple service calls without without waiting for a response
/// \ at the same time.
TEST(NodeTest, MultipleServiceWithoutOutputCallAsync)
{
  reset();

  transport::Node node;
  msgs::Int32 req;
  req.set_data(data);

  // Advertise an invalid service name.
  EXPECT_FALSE(node.Advertise("invalid service", srvWithoutOutput));

  EXPECT_TRUE(node.Advertise(g_topic, srvWithoutOutput));

  // Request an invalid service name.
  EXPECT_FALSE(node.Request("invalid service", req));

  EXPECT_TRUE(node.Request(g_topic, req));

  int i = 0;
  while (i < 100 && !srvExecuted)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call was executed.
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 1);

  // Make another request.
  reset();
  EXPECT_TRUE(node.Request(g_topic, req));
  EXPECT_TRUE(node.Request(g_topic, req));
  EXPECT_TRUE(node.Request(g_topic, req));

  i = 0;
  while (i < 100 && counter < 3)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    ++i;
  }

  // Check that the service call was executed.
  EXPECT_TRUE(srvExecuted);
  EXPECT_EQ(counter, 3);

  // Try to unadvertise an invalid service.
  EXPECT_FALSE(node.UnadvertiseSrv("invalid service"));

  EXPECT_TRUE(node.UnadvertiseSrv(g_topic));
}

//////////////////////////////////////////////////
/// \brief Make a synchronous service call.
TEST(NodeTest, ServiceCallSync)
{
  reset();

  msgs::Int32 req;
  msgs::Int32 rep;
  bool result;
  unsigned int timeout = 1000;

  req.set_data(data);

  transport::Node node;
  EXPECT_TRUE(node.Advertise(g_topic, srvEcho));

  // Request an invalid service name.
  EXPECT_FALSE(node.Request("invalid service", req, timeout, rep, result));

  EXPECT_TRUE(node.Request(g_topic, req, timeout, rep, result));

  // Check that the service call response was executed.
  EXPECT_TRUE(result);
  EXPECT_EQ(rep.data(), req.data());

  reset();
}

//////////////////////////////////////////////////
/// \brief Make a synchronous service call without input.
TEST(NodeTest, ServiceCallWithoutInputSync)
{
  reset();

  msgs::Int32 rep;
  bool result;
  unsigned int timeout = 1000;

  transport::Node node;
  EXPECT_TRUE(node.Advertise(g_topic, srvWithoutInput));

  // Request an invalid service name.
  EXPECT_FALSE(node.Request("invalid service", timeout, rep, result));

  EXPECT_TRUE(node.Request(g_topic, timeout, rep, result));

  // Check that the service call response was executed.
  EXPECT_TRUE(result);
  EXPECT_EQ(rep.data(), data);

  reset();
}

//////////////////////////////////////////////////
/// \brief Check a timeout in a synchronous service call.
TEST(NodeTest, ServiceCallSyncTimeout)
{
  reset();

  msgs::Int32 req;
  msgs::Int32 rep;
  bool result;
  int64_t timeout = 1000;

  req.set_data(data);

  transport::Node node;

  auto t1 = std::chrono::steady_clock::now();
  bool executed = node.Request(g_topic, req, static_cast<unsigned int>(timeout),
      rep, result);
  auto t2 = std::chrono::steady_clock::now();

  int64_t elapsed =
    std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

  // Check if the elapsed time was close to the timeout.
  auto diff = std::max(elapsed, timeout) - std::min(elapsed, timeout);
  EXPECT_LE(diff, 200);

  // Check that the service call response was not executed.
  EXPECT_FALSE(executed);

  reset();
}

//////////////////////////////////////////////////
/// \brief Check a timeout in a synchronous service call without input.
TEST(NodeTest, ServiceCallWithoutInputSyncTimeout)
{
  reset();

  msgs::Int32 rep;
  bool result;
  int64_t timeout = 1000;

  transport::Node node;

  auto t1 = std::chrono::steady_clock::now();
  bool executed = node.Request(g_topic, static_cast<unsigned int>(timeout),
      rep, result);
  auto t2 = std::chrono::steady_clock::now();

  int64_t elapsed =
    std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

  // Check if the elapsed time was close to the timeout.
  auto diff = std::max(elapsed, timeout) - std::min(elapsed, timeout);
  EXPECT_LE(diff, 200);

  // Check that the service call response was not executed.
  EXPECT_FALSE(executed);

  reset();
}

//////////////////////////////////////////////////
/// \brief Create a publisher that sends messages "forever". This function will
/// be used emitting a SIGINT or SIGTERM signal, to make sure that the transport
/// library captures the signals, stop all the tasks and signal the event with
/// the method Interrupted().
void createInfinitePublisher()
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);
  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  auto i = 0;
  bool exitLoop = false;
  while (!exitLoop)
  {
    EXPECT_TRUE(pub.Publish(msg));
    ++i;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    {
      std::lock_guard<std::mutex> lock(exitMutex);
      if (terminatePub)
        exitLoop = true;
    }
  }

  EXPECT_LT(i, 200);

  reset();
}

//////////////////////////////////////////////////
/// \brief Capture SIGINT and SIGTERM and flag that we want to exit.
void signal_handler(int _signal)
{
  std::lock_guard<std::mutex> lock(exitMutex);
  if (_signal == SIGINT || _signal == SIGTERM)
    terminatePub = true;
}

//////////////////////////////////////////////////
/// \brief Check that an external program can capture a SIGINT and terminate
/// the program without problems.
TEST(NodeTest, SigIntTermination)
{
  reset();

  // Install a signal handler for SIGINT.
  std::signal(SIGINT, signal_handler);

  auto thread = std::thread(createInfinitePublisher);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::raise(SIGINT);

  if (thread.joinable())
    thread.join();
}

//////////////////////////////////////////////////
/// \brief Check that an external program can capture a SIGTERM and terminate
/// the program without problems.
TEST(NodeTest, SigTermTermination)
{
  reset();

  // Install a signal handler for SIGTERM.
  std::signal(SIGTERM, signal_handler);

  auto thread = std::thread(createInfinitePublisher);

  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  std::raise(SIGTERM);

  if (thread.joinable())
    thread.join();
}

//////////////////////////////////////////////////
/// \brief Check that a message is not published if the type does not match
/// the type advertised.
TEST(NodeTest, PubSubWrongTypesOnPublish)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);
  msgs::Vector3d msgV;
  msgV.set_x(1);
  msgV.set_y(2);
  msgV.set_z(3);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  EXPECT_TRUE(node.Subscribe(g_topic, cb));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Send a message with a wrong type.
  EXPECT_FALSE(pub.Publish(msgV));

  // Check that the message was not received.
  EXPECT_FALSE(cbExecuted);

  reset();

  // Publish a second message on topic.
  EXPECT_TRUE(pub.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the data was received.
  EXPECT_TRUE(cbExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief Check that a message is not received if the callback does not use
/// the advertised types.
TEST(NodeTest, PubSubWrongTypesOnSubscription)
{
  reset();

  msgs::Vector3d msgV;
  msgV.set_x(1);
  msgV.set_y(2);
  msgV.set_z(3);

  transport::Node node;

  auto pub = node.Advertise<msgs::Vector3d>(g_topic);
  EXPECT_TRUE(pub);

  EXPECT_TRUE(node.Subscribe(g_topic, cb));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Send a message with a wrong type.
  EXPECT_TRUE(pub.Publish(msgV));

  // Check that the message was not received.
  EXPECT_FALSE(cbExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns two subscribers on the same topic. One of the
/// subscribers has a wrong callback (types in the callback does not match the
/// advertised type). Check that only the good callback is executed.
TEST(NodeTest, PubSubWrongTypesTwoSubscribers)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node1;
  transport::Node node2;

  auto pub1 = node1.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub1);

  // Good subscriber.
  EXPECT_TRUE(node1.Subscribe(g_topic, cb));

  // Bad subscriber: cbVector does not match the types advertised by node1.
  EXPECT_TRUE(node2.Subscribe(g_topic, cbVector));

  // Wait some time before publishing.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(pub1.Publish(msg));

  // Give some time to the subscribers.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Check that the message was received by node1.
  EXPECT_TRUE(cbExecuted);

  // Check that the message was not received by node2.
  EXPECT_FALSE(cbVectorExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test creates one publisher and one subscriber. The publisher
/// publishes at higher frequency than the rate set by the subscriber.
TEST(NodeTest, SubThrottled)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  transport::SubscribeOptions opts;
  opts.SetMsgsPerSec(1u);
  EXPECT_TRUE(node.Subscribe(g_topic, cb, opts));

  for (auto i = 0; i < 3; ++i)
  {
    // Should always be true, because this is unthrottled.
    EXPECT_TRUE(pub.ThrottledUpdateReady());
    EXPECT_TRUE(pub.Publish(msg));

    // Rate: 10 msgs/sec.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Node published 3 messages in ~0.3 sec. We should only receive 1 message.
  EXPECT_EQ(1, counter);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test creates one publisher and one subscriber. The publisher
/// publishes at a throttled frequency .
TEST(NodeTest, PubThrottled)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;

  transport::AdvertiseMessageOptions opts;
  opts.SetMsgsPerSec(1u);
  auto pub = node.Advertise<msgs::Int32>(g_topic, opts);
  EXPECT_TRUE(pub);

  EXPECT_TRUE(node.Subscribe(g_topic, cb));

  // Should be true the first time
  EXPECT_TRUE(pub.ThrottledUpdateReady());
  for (auto i = 0; i < 3; ++i)
  {
    EXPECT_TRUE(pub.Publish(msg));
    // False afterwards, because targeting 1 msg/second.
    EXPECT_FALSE(pub.ThrottledUpdateReady());

    // Rate: 10 msgs/sec.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Node published 3 messages in ~0.3 sec. We should only receive 1 message.
  EXPECT_EQ(1, counter);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test creates one local publisher and subscriber and
/// checks that no messages are received when using SetIgnoreLocalMessages
/// is set to true.
TEST(NodeTest, IgnoreLocalMessages)
{
  reset();

  msgs::Int32 msg;
  msg.set_data(data);

  transport::Node node;

  auto pub = node.Advertise<msgs::Int32>(g_topic);
  EXPECT_TRUE(pub);

  transport::SubscribeOptions opts;
  EXPECT_FALSE(opts.IgnoreLocalMessages());
  opts.SetIgnoreLocalMessages(true);
  EXPECT_TRUE(opts.IgnoreLocalMessages());
  EXPECT_TRUE(node.Subscribe(g_topic, cb, opts));

  // Should be true the first time
  for (auto i = 0; i < 3; ++i)
  {
    EXPECT_TRUE(pub.Publish(msg));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // No messages should be received.
  EXPECT_EQ(0, counter);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns a service responser and a service requester. The
/// requester uses a wrong type for the request argument. The test should verify
/// that the service call does not succeed.
TEST(NodeTest, SrvRequestWrongReq)
{
  reset();

  msgs::Vector3d wrongReq;
  msgs::Int32 rep;
  bool result;
  unsigned int timeout = 1000;

  wrongReq.set_x(1);
  wrongReq.set_y(2);
  wrongReq.set_z(3);

  transport::Node node;
  EXPECT_TRUE(node.Advertise(g_topic, srvEcho));

  // Request an asynchronous service call with wrong type in the request.
  EXPECT_TRUE(node.Request(g_topic, wrongReq, response));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_FALSE(responseExecuted);

  // Request a synchronous service call with wrong type in the request.
  EXPECT_FALSE(node.Request(g_topic, wrongReq, timeout, rep, result));

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns a service responser and a service requester. The
/// requester uses a wrong type for the response argument. The test should
/// verify that the service call does not succeed.
TEST(NodeTest, SrvRequestWrongRep)
{
  reset();

  msgs::Int32 req;
  msgs::Vector3d wrongRep;
  bool result;
  unsigned int timeout = 1000;

  req.set_data(data);

  transport::Node node;
  EXPECT_TRUE(node.Advertise(g_topic, srvEcho));

  // Request an asynchronous service call with wrong type in the response.
  EXPECT_TRUE(node.Request(g_topic, req, wrongResponse));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_FALSE(wrongResponseExecuted);

  // Request a synchronous service call with wrong type in the response.
  EXPECT_FALSE(node.Request(g_topic, req, timeout, wrongRep, result));

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns a service that doesn't accept input parameters. The
/// service requester uses a wrong type for the response argument. The test
/// should verify that the service call does not succeed.
TEST(NodeTest, SrvWithoutInputRequestWrongRep)
{
  reset();

  msgs::Vector3d wrongRep;
  bool result;
  unsigned int timeout = 1000;

  transport::Node node;
  EXPECT_TRUE(node.Advertise(g_topic, srvWithoutInput));

  // Request an asynchronous service call with wrong type in the response.
  EXPECT_TRUE(node.Request(g_topic, wrongResponse));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_FALSE(wrongResponseExecuted);

  // Request a synchronous service call with wrong type in the response.
  EXPECT_FALSE(node.Request(g_topic, timeout, wrongRep, result));

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns a service responser and two service requesters. The
/// service requesters use incorrect types in some of the requests. The test
/// should verify that a response is received only when the appropriate types
/// are used.
TEST(NodeTest, SrvTwoRequestsOneWrong)
{
  reset();

  msgs::Int32 req;
  msgs::Int32 goodRep;
  msgs::Vector3d badRep;
  bool result;
  unsigned int timeout = 1000;

  req.set_data(data);

  transport::Node node;
  EXPECT_TRUE(node.Advertise(g_topic, srvEcho));

  // Request service calls with wrong types in the response.
  EXPECT_FALSE(node.Request(g_topic, req, timeout, badRep, result));
  EXPECT_TRUE(node.Request(g_topic, req, wrongResponse));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_FALSE(wrongResponseExecuted);

  // Valid service requests.
  EXPECT_TRUE(node.Request(g_topic, req, timeout, goodRep, result));
  EXPECT_TRUE(node.Request(g_topic, req, response));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_TRUE(responseExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test spawns a service that doesn't accept input parameters. The
/// service requesters use incorrect types in some of the requests. The test
/// should verify that a response is received only when the appropriate types
/// are used.
TEST(NodeTest, SrvWithoutInputTwoRequestsOneWrong)
{
  reset();

  msgs::Int32 goodRep;
  msgs::Vector3d badRep;
  bool result;
  unsigned int timeout = 1000;

  transport::Node node;
  EXPECT_TRUE(node.Advertise(g_topic, srvWithoutInput));

  // Request service calls with wrong types in the response.
  EXPECT_FALSE(node.Request(g_topic, timeout, badRep, result));
  EXPECT_TRUE(node.Request(g_topic, wrongResponse));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_FALSE(wrongResponseExecuted);

  // Valid service requests.
  EXPECT_TRUE(node.Request(g_topic, timeout, goodRep, result));
  EXPECT_TRUE(node.Request(g_topic, response));
  std::this_thread::sleep_for(std::chrono::milliseconds(300));
  EXPECT_TRUE(responseExecuted);

  reset();
}

//////////////////////////////////////////////////
/// \brief This test creates two nodes and advertises some topics. The test
/// verifies that TopicList() returns the list of all the topics advertised.
TEST(NodeTest, TopicList)
{
  std::vector<std::string> topics;
  transport::Node node1;
  transport::Node node2;

  auto pub1 = node1.Advertise<msgs::Int32>("topic1");
  auto pub2 = node2.Advertise<msgs::Int32>("topic2");

  node1.TopicList(topics);
  EXPECT_EQ(topics.size(), 2u);
  topics.clear();

  auto start = std::chrono::steady_clock::now();
  node1.TopicList(topics);
  auto end = std::chrono::steady_clock::now();
  EXPECT_EQ(topics.size(), 2u);

  // The first TopicList() call might block if the discovery is still
  // initializing (it may happen if we run this test alone).
  // However, the second call should never block.
  auto elapsed = end - start;
  EXPECT_LT(std::chrono::duration_cast<std::chrono::milliseconds>
      (elapsed).count(), 2);
}

//////////////////////////////////////////////////
/// \brief This test creates two nodes and advertises some topics. The test
/// verifies that TopicList() returns the list of all the topics advertised.
/// Topic remapping is enabled.
TEST(NodeTest, TopicListRemap)
{
  std::vector<std::string> topics;
  transport::NodeOptions nodeOptions;
  nodeOptions.AddTopicRemap(g_topic, g_topic_remap);
  transport::Node node(nodeOptions);

  auto pub = node.Advertise<msgs::Int32>(g_topic);

  node.TopicList(topics);
  ASSERT_EQ(1u, topics.size());

  // The topic advertised should be remapped.
  EXPECT_EQ(g_topic_remap, topics.at(0));
}

//////////////////////////////////////////////////
/// \brief This test creates two nodes and advertises some services. The test
/// verifies that ServiceList() returns the list of all the services advertised.
TEST(NodeTest, ServiceList)
{
  std::vector<std::string> services;
  transport::Node node;

  node.Advertise(g_topic, srvEcho);

  node.ServiceList(services);
  EXPECT_EQ(services.size(), 1u);
  services.clear();

  auto start = std::chrono::steady_clock::now();
  node.ServiceList(services);
  auto end = std::chrono::steady_clock::now();
  EXPECT_EQ(services.size(), 1u);

  // The first TopicList() call might block if the discovery is still
  // initializing (it may happen if we run this test alone).
  // However, the second call should never block.
  auto elapsed = end - start;
  EXPECT_LT(std::chrono::duration_cast<std::chrono::milliseconds>
      (elapsed).count(), 2);
}

//////////////////////////////////////////////////
/// \brief This test creates two nodes and advertises some services. The test
/// verifies that ServiceList() returns the list of all the services advertised.
/// Topic remapping is enabled.
TEST(NodeTest, ServiceListRemap)
{
  std::vector<std::string> services;
  transport::NodeOptions nodeOptions;
  nodeOptions.AddTopicRemap(g_topic, g_topic_remap);
  transport::Node node(nodeOptions);

  node.Advertise(g_topic, srvEcho);

  node.ServiceList(services);
  ASSERT_EQ(1u, services.size());
  EXPECT_EQ(g_topic_remap, services.at(0));
}

//////////////////////////////////////////////////
/// \brief Check bad topic remap use cases.
TEST(NodeTest, WrongTopicRemap)
{
  transport::NodeOptions nodeOptions;

  // Invalid topics.
  EXPECT_FALSE(nodeOptions.AddTopicRemap("  ", g_topic_remap));
  EXPECT_FALSE(nodeOptions.AddTopicRemap(g_topic, "  "));

  // Repeated topic.
  EXPECT_TRUE(nodeOptions.AddTopicRemap(g_topic, g_topic_remap));
  EXPECT_FALSE(nodeOptions.AddTopicRemap(g_topic, g_topic_remap));
}

/////////////////////////////////////////////////
/// \brief Check the high water mark of the receiving message buffer.
TEST(NodeTest, RcvHwm)
{
  EXPECT_EQ(transport::kDefaultRcvHwm, transport::rcvHwm());
}

//////////////////////////////////////////////////
/// \brief Check the high water mark of the sending message buffer.
TEST(NodeTest, SndHwm)
{
  EXPECT_EQ(transport::kDefaultSndHwm, transport::sndHwm());
}

//////////////////////////////////////////////////
/// \brief Check that we destruct a Node object before a Node::Publisher.
TEST(NodePubTest, DestructionOrder)
{
  transport::Node::Publisher pub;

  {
    transport::Node node;
    pub = node.Advertise<msgs::Vector3d>(g_topic);
  }
}

//////////////////////////////////////////////////
/// \brief Create a separate thread, block it calling waitForShutdown() and
/// emit a SIGINT signal. Check that the transport library captures the signal
/// and is able to terminate.
TEST(NodeTest, waitForShutdownSIGINT)
{
  std::thread aThread([]{transport::waitForShutdown();});
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  raise(SIGINT);
  aThread.join();
}

//////////////////////////////////////////////////
/// \brief Create a separate thread, block it calling waitForShutdown() and
/// emit a SIGTERM signal. Check that the transport library captures the signal
/// and is able to terminate.
TEST(NodeTest, waitForShutdownSIGTERM)
{
  std::thread aThread([]{transport::waitForShutdown();});
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  raise(SIGTERM);
  aThread.join();
}

//////////////////////////////////////////////////
/// \brief Test topic statistics with no statistics available.
TEST(NodeTest, statistics)
{
  transport::Node node;
  EXPECT_TRUE(node.EnableStats("/test", true));
  EXPECT_EQ(std::nullopt, node.TopicStats("/test"));
}

//////////////////////////////////////////////////
/// \brief Test adding and querying relays
TEST(NodeTest, relay) {
  transport::Node node;

  // Make sure the relay we're about to add hasn't already been configured.
  const std::vector<std::string> relaysBeforeAdd = node.GlobalRelays();
  {
    auto relaysBeforeIt = std::find_if(
        relaysBeforeAdd.cbegin(), relaysBeforeAdd.cend(),
        [](const std::string &relay) { return relay == "127.0.0.123"; });
    ASSERT_EQ(relaysBeforeIt, relaysBeforeAdd.cend());
  }
  node.AddGlobalRelay("127.0.0.123");

  const std::vector<std::string> relaysAfterAdd = node.GlobalRelays();
  {
    EXPECT_EQ(relaysAfterAdd.size(), relaysBeforeAdd.size() + 1);
    auto relayIt = std::find_if(
        relaysAfterAdd.cbegin(), relaysAfterAdd.cend(),
        [](const std::string &relay) { return relay == "127.0.0.123"; });
    EXPECT_NE(relayIt, relaysAfterAdd.cend());
  }
}

//////////////////////////////////////////////////
int main(int argc, char **argv)
{
  // Get a random partition name.
  partition = testing::getRandomNumber();
  g_FQNPartition = std::string("/") + partition;

  // Set the partition name for this process.
  gz::utils::setenv("GZ_PARTITION", partition);

  // Enable verbose mode.
  gz::utils::setenv("GZ_VERBOSE", "1");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
