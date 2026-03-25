/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include <map>
#include <memory>

#include "gz/transport/Node.hh"
#include "gz/transport/SubscribeOptions.hh"
#include "gz/transport/CIface.h"

/// \brief A wrapper to store a Gazebo Transport node and its publishers.
struct GzTransportNode
{
  /// \brief Pointer to the node.
  std::unique_ptr<gz::transport::Node> nodePtr;

  /// \brief All publishers of this node.
  std::map<std::string, gz::transport::Node::Publisher> publishers;
};

/////////////////////////////////////////////////
GzTransportNode *gzTransportNodeCreate(const char *_partition)
{
  GzTransportNode *gzTransportNode = new GzTransportNode();
  gz::transport::NodeOptions opts;
  if (_partition)
    opts.SetPartition(_partition);
  gzTransportNode->nodePtr = std::make_unique<gz::transport::Node>(opts);
  return gzTransportNode;
}

/////////////////////////////////////////////////
void gzTransportNodeDestroy(GzTransportNode **_node)
{
  if (*_node)
  {
    delete *_node;
    *_node = nullptr;
  }
}

/////////////////////////////////////////////////
int gzTransportAdvertise(GzTransportNode *_node, const char *_topic,
    const char *_msgType)
{
  if (!_node)
    return 1;

  // Create a publisher if one does not exist.
  if (_node->publishers.find(_topic) == _node->publishers.end())
    _node->publishers[_topic] = _node->nodePtr->Advertise(_topic, _msgType);

  return 0;
}

/////////////////////////////////////////////////
int gzTransportPublish(GzTransportNode *_node, const char *_topic,
    const void *_data, const char *_msgType)
{
  if (!_node)
    return 1;

  // Create a publisher if one does not exist.
  if (gzTransportAdvertise(_node, _topic, _msgType) == 0)
  {
    // Publish the message.
    return _node->publishers[_topic].PublishRaw(
      reinterpret_cast<const char*>(_data), _msgType) ? 0 : 1;
  }

  return 1;
}

/////////////////////////////////////////////////
int gzTransportSubscribe(GzTransportNode *_node, const char *_topic,
    void (*_callback)(const char *, size_t, const char *, void *),
    void *_userData)
{
  if (!_node)
    return 1;

  return _node->nodePtr->SubscribeRaw(_topic,
      [_callback, _userData](const char *_msg,
                  const size_t _size,
                  const gz::transport::MessageInfo &_info) -> void
                  {
                    _callback(_msg, _size, _info.Type().c_str(), _userData);
                  }) ? 0 : 1;
}

/////////////////////////////////////////////////
int gzTransportSubscribeOptions(GzTransportNode *_node, const char *_topic,
    SubscribeOpts _opts,
    void (*_callback)(const char *, size_t, const char *, void *),
    void *_userData)
{
  if (!_node)
    return 1;

  gz::transport::SubscribeOptions opts;
  opts.SetMsgsPerSec(_opts.msgsPerSec);

  return _node->nodePtr->SubscribeRaw(
      _topic,
      [_callback, _userData](const char *_msg,
        const size_t _size,
        const gz::transport::MessageInfo &_info) -> void
        {
          _callback(_msg, _size, _info.Type().c_str(), _userData);
        },
      gz::transport::kGenericMessageType,
      opts) ? 0 : 1;
}


/////////////////////////////////////////////////
int gzTransportSubscribeNonConst(GzTransportNode *_node, char *_topic,
    void (*_callback)(char *, size_t, char *, void *), void *_userData)
{
  if (!_node)
    return 1;

  return _node->nodePtr->SubscribeRaw(_topic,
      [_callback, _userData](const char *_msg,
                  const size_t _size,
                  const gz::transport::MessageInfo &_info) -> void
                  {
                    _callback(const_cast<char *>(_msg), _size,
                              const_cast<char *>(_info.Type().c_str()),
                              _userData);
                  }) ? 0 : 1;
}

/////////////////////////////////////////////////
int gzTransportUnsubscribe(GzTransportNode *_node, const char *_topic)
{
  if (!_node)
    return 1;

  return _node->nodePtr->Unsubscribe(_topic) ? 0 : 1;
}


/////////////////////////////////////////////////
void gzTransportWaitForShutdown()
{
  gz::transport::waitForShutdown();
}
