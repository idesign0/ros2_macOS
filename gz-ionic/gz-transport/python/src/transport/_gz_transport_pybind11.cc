/*
 * Copyright (C) 2023 Open Source Robotics Foundation
 * Copyright (C) 2022 Rhys Mainwaring
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

#include <google/protobuf/message.h>
#include <gz/transport/Node.hh>

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <functional>
#include <tuple>
#include <utility>

namespace py = pybind11;

namespace gz
{
namespace transport
{
namespace python
{

PYBIND11_MODULE(BINDINGS_MODULE_NAME, m) {
    py::class_<AdvertiseOptions>(
      m, "AdvertiseOptions",
      "A class for customizing the publication options for"
      " a topic or service advertised")
      .def(py::init<>())
      .def(py::init<const AdvertiseOptions &>())
      .def_property("scope",
          &AdvertiseOptions::Scope,
          &AdvertiseOptions::SetScope,
          "The scope used in this topic/service")
      .def("__copy__", [](const AdvertiseOptions &self)
          {
            return AdvertiseOptions(self);
          })
      .def("__deepcopy__",[](const AdvertiseOptions &self, pybind11::dict)
          {
            return AdvertiseOptions(self);
          });

    py::class_<AdvertiseMessageOptions, AdvertiseOptions>(
      m, "AdvertiseMessageOptions",
      "A class for customizing the publication options for a topic")
      .def(py::init<>())
      .def(py::init<const AdvertiseMessageOptions &>())
      .def_property_readonly("throttled",
          &AdvertiseMessageOptions::Throttled,
          "Whether the publication has been throttled")
      .def_property("msgs_per_sec",
          &AdvertiseMessageOptions::MsgsPerSec,
          &AdvertiseMessageOptions::SetMsgsPerSec,
          "The maximum number of messages per second to be published")
      .def("__copy__", 
          [](const AdvertiseMessageOptions &self)
          {
            return AdvertiseMessageOptions(self);
          })
      .def("__deepcopy__",
          [](const AdvertiseMessageOptions &self, pybind11::dict)
          {
            return AdvertiseMessageOptions(self);
          });

    py::class_<NodeOptions>(
      m, "NodeOptions",
      "A class to provide different options for the node")
      .def(py::init<>())
      .def(py::init<const NodeOptions &>())
      .def_property("namespace",
          &NodeOptions::NameSpace,
          &NodeOptions::SetNameSpace,
          "Set the node's namespace.")
      .def_property("partition",
          &NodeOptions::Partition,
          &NodeOptions::SetPartition,
          "Set the node's partition name.")
      .def("add_topic_remap",
          &NodeOptions::AddTopicRemap,
          py::arg("from_topic"),
          py::arg("to_topic"),
          "Add a new topic remapping. If a topic is remapped, the 'from_topic'"
          "topic will be renamed to 'to_topic. Is not possible to add two remaps"
          "over the same 'from_topic'.")
      .def("topic_remap",[](
          NodeOptions &self,
          const std::string &fromTopic)
          {
            std::string toTopic;
            bool result{false};
            result = self.TopicRemap(fromTopic, toTopic);
            return py::make_tuple(result, toTopic);
          },
          py::arg("from_topic"),
          "Get a topic remapping. Returns a pair with the result of the method"
          "and the remapped name of the topic. The topic name remains empty if" 
          "there isn't any remap for the topic")
      .def("__copy__", 
          [](const NodeOptions &self)
          {
            return NodeOptions(self);
          })
      .def("__deepcopy__",
          [](const NodeOptions &self, pybind11::dict)
          {
            return NodeOptions(self);
          });

    py::class_<SubscribeOptions>(
      m, "SubscribeOptions",
      "A class to provide different options for a subscription")
      .def(py::init<>())
      .def(py::init<const SubscribeOptions &>())
      .def_property_readonly("throttled",
          &SubscribeOptions::Throttled,
          "Whether the subscription has been throttled")
      .def_property("msgs_per_sec",
          &SubscribeOptions::MsgsPerSec,
          &SubscribeOptions::SetMsgsPerSec,
          "Set the maximum number of messages per second received per topic")
      .def("__copy__", 
          [](const SubscribeOptions &self)
          {
            return SubscribeOptions(self);
          })
      .def("__deepcopy__",
          [](const SubscribeOptions &self, pybind11::dict)
          {
            return SubscribeOptions(self);
          });

    // We are leaving this as an opaque class in order to be able to add bindings
    // to it later without breaking ABI.
    py::class_<MessageInfo>(
      m, "MessageInfo",
      "A class that provides information about the message received.")
      .def(py::init<>());

    py::class_<MessagePublisher>(
      m, "MessagePublisher",
      "This class stores all the information about a message publisher.")
      .def(py::init<>())
      .def_property_readonly("ctrl",
          &MessagePublisher::Ctrl,
          "Get the ZeroMQ control address of the publisher.")
      .def_property_readonly("msg_type_name",
          &MessagePublisher::MsgTypeName,
          "Get the message type advertised by this publisher.")
      .def_property_readonly("options",
          &MessagePublisher::Options,
          "Get the advertised options.");

    py::class_<ServicePublisher>(
      m, "ServicePublisher",
      "This class stores all the information about a service publisher.")
      .def(py::init<>())
      .def_property_readonly("socket_id",
          &ServicePublisher::SocketId,
          "Get the ZeroMQ socket ID for this publisher.")
      .def_property_readonly("req_type_name",
          &ServicePublisher::ReqTypeName,
          "Get the name of the request's protobuf message advertised.")
      .def_property_readonly("rep_type_name",
          &ServicePublisher::RepTypeName,
          "Get the advertised options.")
      .def_property_readonly("options",
          &ServicePublisher::Options,
          "Get the advertised options.");

    py::class_<TopicStatistics>(
      m, "TopicStatistics",
      "This class encapsulates statistics for a single topic.")
      .def(py::init<>());

    auto node = py::class_<Node>(m, "Node",
      "A class that allows a client to communicate with other peers."
      " There are two main communication modes: pub/sub messages"
      " and service calls")
      .def(py::init<>())
      .def(py::init<const NodeOptions &>())
      .def("advertise", static_cast<
          Node::Publisher (Node::*) (
              const std::string &,
              const std::string &,
              const AdvertiseMessageOptions &
          )>(&Node::Advertise),
          py::arg("topic"),
          py::arg("msg_type_name"),
          py::arg("options"),
          "Advertise a new topic. If a topic is currently advertised,"
          " you cannot advertise it a second time (regardless of its type)")
      .def("advertised_topics", &Node::AdvertisedTopics,
          "Get the list of topics advertised by this node")
      .def("subscribed_topics", &Node::SubscribedTopics,
          "Get the list of topics subscribed by this node")
      .def("unsubscribe", &Node::Unsubscribe,
          py::arg("topic"),
          "Unsubscribe from a topic")
      // Send a service request using the blocking interface
      .def("request_raw", [](
          Node &_node,
          const std::string &_service,
          const std::string &_request,
          const std::string &_reqType,
          const std::string &_repType,
          const unsigned int &_timeout)
          {
            bool result{false};
            std::string _response;
            result = _node.RequestRaw(_service, _request, _reqType,
                            _repType, _timeout, _response, result);
            return py::make_tuple(result, py::bytes(_response.c_str(), _response.size()));
          },
          py::arg("topic"),
          py::arg("request"),
          py::arg("request_type"),
          py::arg("response_type"),
          py::arg("timeout"),
          "Request a new service without input parameter using"
          " a blocking call")
      .def("topic_list", [](
          Node &_node)
          {
            std::vector<std::string> topics;
            _node.TopicList(topics);
            return topics;
          },
          "Get the list of topics currently advertised in the network")
      .def("topic_info", [](
          Node &_node,
          const std::string &_topic)
          {
            std::vector<MessagePublisher> publishers;
            std::vector<MessagePublisher> subscribers;
            _node.TopicInfo(_topic, publishers, subscribers);
            return py::make_tuple(publishers, subscribers);
          },
          py::arg("topic"),
          "Get the information about a topic")
      .def("service_list", [](
          Node &_node)
          {
            std::vector<std::string> services;
            _node.ServiceList(services);
            return services;
          },
          "Get the list of topics currently advertised in the network")
      .def("service_info", [](
          Node &_node,
          const std::string &_service)
          {
            std::vector<ServicePublisher> publishers;
            _node.ServiceInfo(_service, publishers);
            return publishers;
          },
          py::arg("service"),
          "Get the information about a service")
      .def("subscribe_raw", [](
          Node &_node,
          const std::string &_topic,
          std::function<void(py::bytes _msgData,
                           const MessageInfo &_info)> &_callback,
          const std::string &_msgType,
          const SubscribeOptions &_opts)
          {
            auto _cb = [_callback](const char *_msgData, const size_t _size,
                           const MessageInfo &_info){
                pybind11::gil_scoped_acquire acq;
                return _callback(py::bytes(_msgData, _size), _info);
            };
            return _node.SubscribeRaw(_topic, _cb, _msgType, _opts);
          },
          py::arg("topic"),
          py::arg("callback"),
          py::arg("msg_type"),
          py::arg("options"))
      .def_property_readonly("options", &Node::Options,
          "Get the reference to the current node options.")
      .def("enable_stats", &Node::EnableStats,
          py::arg("topic"),
          py::arg("enable"),
          py::arg("publication_topic") = "/statistics",
          py::arg("publication_rate") = 1,
          "Turn topic statistics on or off.")
      .def("topic_stats", &Node::TopicStats,
          py::arg("topic"),
          "Get the current statistics for a topic. Statistics must"
          "have been enabled using the EnableStats function, otherwise"
          "the return value will be null.");
      
  // Register Node::Publisher as a subclass of Node
  py::class_<gz::transport::Node::Publisher>(node, "Publisher",
      "A class that is used to store information about an"
      " advertised publisher.")
      .def(py::init<>())
      .def(py::init<const gz::transport::Node::Publisher &>())
      .def("valid", &gz::transport::Node::Publisher::Valid,
          "Return true if valid information, such as a non-empty"
          " topic name, is present.")
      .def("publish_raw", &gz::transport::Node::Publisher::PublishRaw,
          py::arg("msg_data"),
          py::arg("msg_type"))
      .def("throttled_update_ready",
          &gz::transport::Node::Publisher::ThrottledUpdateReady,
          "")
      .def("has_connections",
          &gz::transport::Node::Publisher::HasConnections,
          "Return true if this publisher has subscribers");
}  // gz-transport14 module

}  // python
}  // transport
}  // gz
