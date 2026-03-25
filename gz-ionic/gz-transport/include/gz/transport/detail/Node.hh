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

#ifndef GZ_TRANSPORT_DETAIL_NODE_HH_
#define GZ_TRANSPORT_DETAIL_NODE_HH_

#include <gz/msgs/empty.pb.h>

#include <memory>
#include <string>
#include <utility>

#include "gz/transport/Node.hh"

namespace gz::transport
{
  //////////////////////////////////////////////////
  template<typename MessageT>
  Node::Publisher Node::Advertise(
      const std::string &_topic,
      const AdvertiseMessageOptions &_options)
  {
    return this->Advertise(_topic,
        std::string(MessageT().GetTypeName()), _options);
  }

  //////////////////////////////////////////////////
  template<typename MessageT>
  bool Node::Subscribe(
      const std::string &_topic,
      void(*_cb)(const MessageT &_msg),
      const SubscribeOptions &_opts)
  {
    return this->SubscribeImpl(_topic, _cb, _opts) != nullptr;
  }

  //////////////////////////////////////////////////
  template<typename MessageT>
  bool Node::Subscribe(
      const std::string &_topic,
      std::function<void(const MessageT &_msg)> _cb,
      const SubscribeOptions &_opts)
  {
    return this->SubscribeImpl(_topic, _cb, _opts) != nullptr;
  }

  //////////////////////////////////////////////////
  template<typename ClassT, typename MessageT>
  bool Node::Subscribe(
      const std::string &_topic,
      void(ClassT::*_cb)(const MessageT &_msg),
      ClassT *_obj,
      const SubscribeOptions &_opts)
  {
    return this->SubscribeImpl(_topic, _cb, _obj, _opts) != nullptr;
  }

  //////////////////////////////////////////////////
  template<typename MessageT>
  bool Node::Subscribe(
      const std::string &_topic,
      void(*_cb)(const MessageT &_msg, const MessageInfo &_info),
      const SubscribeOptions &_opts)
  {
    return this->SubscribeImpl(_topic, _cb, _opts) != nullptr;
  }

  //////////////////////////////////////////////////
  template<typename MessageT>
  bool Node::Subscribe(
      const std::string &_topic,
      std::function<void(const MessageT &_msg,
                         const MessageInfo &_info)> _cb,
      const SubscribeOptions &_opts)
  {
    return this->SubscribeImpl(_topic, _cb, _opts) != nullptr;
  }

  //////////////////////////////////////////////////
  template<typename ClassT, typename MessageT>
  bool Node::Subscribe(
      const std::string &_topic,
      void(ClassT::*_cb)(const MessageT &_msg, const MessageInfo &_info),
      ClassT *_obj,
      const SubscribeOptions &_opts)
  {
    return this->SubscribeImpl(_topic, _cb, _obj, _opts) != nullptr;
  }

  //////////////////////////////////////////////////
  template <typename ...Args>
  Node::Subscriber Node::CreateSubscriber(const std::string &_topic,
                                          Args && ...args)
  {
    auto handler = this->SubscribeImpl(_topic,
                                       std::forward<Args>(args)...);
    if (handler && !handler->HandlerUuid().empty())
    {
      return Node::Subscriber(_topic,
                              handler->NodeUuid(),
                              this->Options(),
                              handler->HandlerUuid());
    }
    return Node::Subscriber();
  }

  //////////////////////////////////////////////////
  template<typename MessageT>
  std::shared_ptr<SubscriptionHandler<MessageT>> Node::SubscribeImpl(
      const std::string &_topic,
      void(*_cb)(const MessageT &_msg),
      const SubscribeOptions &_opts)
  {
    std::function<void(const MessageT &, const MessageInfo &)> f =
      [_cb](const MessageT & _internalMsg,
            const MessageInfo &/*_internalInfo*/)
    {
      (*_cb)(_internalMsg);
    };

    return this->SubscribeImpl<MessageT>(_topic, f, _opts);
  }

  //////////////////////////////////////////////////
  template<typename MessageT>
  std::shared_ptr<SubscriptionHandler<MessageT>> Node::SubscribeImpl(
      const std::string &_topic,
      std::function<void(const MessageT &_msg)> _cb,
      const SubscribeOptions &_opts)
  {
    std::function<void(const MessageT &, const MessageInfo &)> f =
      [cb = std::move(_cb)](const MessageT & _internalMsg,
            const MessageInfo &/*_internalInfo*/)
    {
      cb(_internalMsg);
    };

    return this->SubscribeImpl<MessageT>(_topic, f, _opts);
  }

  //////////////////////////////////////////////////
  template<typename ClassT, typename MessageT>
  std::shared_ptr<SubscriptionHandler<MessageT>> Node::SubscribeImpl(
      const std::string &_topic,
      void(ClassT::*_cb)(const MessageT &_msg),
      ClassT *_obj,
      const SubscribeOptions &_opts)
  {
    std::function<void(const MessageT &, const MessageInfo &)> f =
      [_cb, _obj](const MessageT & _internalMsg,
                  const MessageInfo &/*_internalInfo*/)
    {
      auto cb = std::bind(_cb, _obj, std::placeholders::_1);
      cb(_internalMsg);
    };

    return this->SubscribeImpl<MessageT>(_topic, f, _opts);
  }

  //////////////////////////////////////////////////
  template<typename MessageT>
  std::shared_ptr<SubscriptionHandler<MessageT>> Node::SubscribeImpl(
      const std::string &_topic,
      void(*_cb)(const MessageT &_msg, const MessageInfo &_info),
      const SubscribeOptions &_opts)
  {
    std::function<void(const MessageT &, const MessageInfo &)> f =
      [_cb](const MessageT & _internalMsg,
            const MessageInfo &_internalInfo)
    {
      (*_cb)(_internalMsg, _internalInfo);
    };

    return this->SubscribeImpl<MessageT>(_topic, f, _opts);
  }

  //////////////////////////////////////////////////
  template<typename MessageT>
  std::shared_ptr<SubscriptionHandler<MessageT>> Node::SubscribeImpl(
      const std::string &_topic,
      std::function<void(const MessageT &_msg,
                         const MessageInfo &_info)> _cb,
      const SubscribeOptions &_opts)
  {
    // Topic remapping.
    std::string topic = _topic;
    this->Options().TopicRemap(_topic, topic);

    std::string fullyQualifiedTopic;
    if (!TopicUtils::FullyQualifiedName(this->Options().Partition(),
      this->Options().NameSpace(), topic, fullyQualifiedTopic))
    {
      std::cerr << "Topic [" << topic << "] is not valid." << std::endl;
      return nullptr;
    }

    // Create a new subscription handler.
    std::shared_ptr<SubscriptionHandler<MessageT>> subscrHandlerPtr(
        new SubscriptionHandler<MessageT>(this->NodeUuid(), _opts));

    // Insert the callback into the handler.
    subscrHandlerPtr->SetCallback(std::move(_cb));

    std::lock_guard<std::recursive_mutex> lk(this->Shared()->mutex);

    // Store the subscription handler. Each subscription handler is
    // associated with a topic. When the receiving thread gets new data,
    // it will recover the subscription handler associated to the topic and
    // will invoke the callback.
    this->Shared()->localSubscribers.normal.AddHandler(
      fullyQualifiedTopic, this->NodeUuid(), subscrHandlerPtr);

    if (!this->SubscribeHelper(fullyQualifiedTopic))
      return nullptr;

    return subscrHandlerPtr;
  }

  //////////////////////////////////////////////////
  template<typename ClassT, typename MessageT>
  std::shared_ptr<SubscriptionHandler<MessageT>> Node::SubscribeImpl(
      const std::string &_topic,
      void(ClassT::*_cb)(const MessageT &_msg, const MessageInfo &_info),
      ClassT *_obj,
      const SubscribeOptions &_opts)
  {
    std::function<void(const MessageT &, const MessageInfo &)> f =
      [_cb, _obj](const MessageT & _internalMsg,
                  const MessageInfo &_internalInfo)
    {
      auto cb = std::bind(_cb, _obj, std::placeholders::_1,
        std::placeholders::_2);
      cb(_internalMsg, _internalInfo);
    };

    return this->SubscribeImpl<MessageT>(_topic, f, _opts);
  }

  //////////////////////////////////////////////////
  template<typename RequestT, typename ReplyT>
  bool Node::Advertise(
    const std::string &_topic,
    bool(*_cb)(const RequestT &_request, ReplyT &_reply),
    const AdvertiseServiceOptions &_options)
  {
    // Dev Note: This overload of Advertise(~) is necessary so that the
    // compiler can correctly infer the template arguments. We cannot rely
    // on the compiler to implicitly cast the function pointer to a
    // std::function object, because the compiler cannot infer the template
    // parameters T1 and T2 from the signature of the function pointer that
    // gets passed to Advertise(~).

    // We create a std::function object so that we can explicitly call the
    // baseline overload of Advertise(~).
    std::function<bool(const RequestT&, ReplyT&)> f =
      [_cb](const RequestT &_internalReq, ReplyT &_internalRep)
    {
      return (*_cb)(_internalReq, _internalRep);
    };

    return this->Advertise(_topic, f, _options);
  }

  //////////////////////////////////////////////////
  template<typename ReplyT>
  bool Node::Advertise(
    const std::string &_topic,
    bool(*_cb)(ReplyT &_reply),
    const AdvertiseServiceOptions &_options)
  {
    std::function<bool(const msgs::Empty &, ReplyT &)> f =
      [_cb](const msgs::Empty &/*_internalReq*/, ReplyT &_internalRep)
    {
      return (*_cb)(_internalRep);
    };
    return this->Advertise(_topic, f, _options);
  }

  //////////////////////////////////////////////////
  template<typename RequestT>
  bool Node::Advertise(
    const std::string &_topic,
    void(*_cb)(const RequestT &_request),
    const AdvertiseServiceOptions &_options)
  {
    std::function<bool(const RequestT &, gz::msgs::Empty &)> f =
      [_cb](const RequestT &_internalReq,
            gz::msgs::Empty &/*_internalRep*/)
    {
      (*_cb)(_internalReq);
      return true;
    };

    return this->Advertise(_topic, f, _options);
  }

  //////////////////////////////////////////////////
  template<typename RequestT, typename ReplyT>
  bool Node::Advertise(
    const std::string &_topic,
    std::function<bool(const RequestT &, ReplyT &)> _cb,
    const AdvertiseServiceOptions &_options)
  {
    // Topic remapping.
    std::string topic = _topic;
    this->Options().TopicRemap(_topic, topic);

    std::string fullyQualifiedTopic;
    if (!TopicUtils::FullyQualifiedName(this->Options().Partition(),
      this->Options().NameSpace(), topic, fullyQualifiedTopic))
    {
      std::cerr << "Service [" << topic << "] is not valid." << std::endl;
      return false;
    }

    // Create a new service reply handler.
    std::shared_ptr<RepHandler<RequestT, ReplyT>> repHandlerPtr(
      new RepHandler<RequestT, ReplyT>());

    // Insert the callback into the handler.
    repHandlerPtr->SetCallback(_cb);

    std::lock_guard<std::recursive_mutex> lk(this->Shared()->mutex);

    // Add the topic to the list of advertised services.
    this->SrvsAdvertised().insert(fullyQualifiedTopic);

    // Store the replier handler. Each replier handler is
    // associated with a topic. When the receiving thread gets new requests,
    // it will recover the replier handler associated to the topic and
    // will invoke the service call.
    this->Shared()->repliers.AddHandler(
      fullyQualifiedTopic, this->NodeUuid(), repHandlerPtr);

    // Notify the discovery service to register and advertise my responser.
    ServicePublisher publisher(fullyQualifiedTopic,
      this->Shared()->myReplierAddress,
      this->Shared()->replierId.ToString(),
      this->Shared()->pUuid, this->NodeUuid(),
      std::string(RequestT().GetTypeName()),
      std::string(ReplyT().GetTypeName()), _options);

    if (!this->Shared()->AdvertisePublisher(publisher))
    {
      std::cerr << "Node::Advertise(): Error advertising service ["
                << topic
                << "]. Did you forget to start the discovery service?"
                << std::endl;
      return false;
    }

    return true;
  }

  //////////////////////////////////////////////////
  template<typename ReplyT>
  bool Node::Advertise(
    const std::string &_topic,
    std::function<bool(ReplyT &_reply)> &_cb,
    const AdvertiseServiceOptions &_options)
  {
    std::function<bool(const msgs::Empty &, ReplyT &)> f =
      [_cb](const msgs::Empty &/*_internalReq*/, ReplyT &_internalRep)
    {
      return (_cb)(_internalRep);
    };
    return this->Advertise(_topic, f, _options);
  }

  //////////////////////////////////////////////////
  template<typename RequestT>
  bool Node::Advertise(
    const std::string &_topic,
    std::function<void(const RequestT &_request)> &_cb,
    const AdvertiseServiceOptions &_options)
  {
    std::function<bool(const RequestT &, gz::msgs::Empty &)> f =
      [_cb](const RequestT &_internalReq,
            gz::msgs::Empty &/*_internalRep*/)
    {
      (_cb)(_internalReq);
      return true;
    };

    return this->Advertise(_topic, f, _options);
  }

  //////////////////////////////////////////////////
  template<typename ClassT, typename RequestT, typename ReplyT>
  bool Node::Advertise(
    const std::string &_topic,
    bool(ClassT::*_cb)(const RequestT &_request, ReplyT &_reply),
    ClassT *_obj,
    const AdvertiseServiceOptions &_options)
  {
    std::function<bool(const RequestT &, ReplyT &)> f =
      [_cb, _obj](const RequestT &_internalReq,
                  ReplyT &_internalRep)
    {
      return (_obj->*_cb)(_internalReq, _internalRep);
    };

    return this->Advertise(_topic, f, _options);
  }

  //////////////////////////////////////////////////
  template<typename ClassT, typename ReplyT>
  bool Node::Advertise(
    const std::string &_topic,
    bool(ClassT::*_cb)(ReplyT &_reply),
    ClassT *_obj,
    const AdvertiseServiceOptions &_options)
  {
    std::function<bool(const msgs::Empty &, ReplyT &)> f =
      [_cb, _obj](const msgs::Empty &/*_internalReq*/, ReplyT &_internalRep)
    {
      return (_obj->*_cb)(_internalRep);
    };

    return this->Advertise(_topic, f, _options);
  }

  //////////////////////////////////////////////////
  template<typename ClassT, typename RequestT>
  bool Node::Advertise(
    const std::string &_topic,
    void(ClassT::*_cb)(const RequestT &_request),
    ClassT *_obj,
    const AdvertiseServiceOptions &_options)
  {
    std::function<bool(const RequestT &, gz::msgs::Empty &)> f =
      [_cb, _obj](const RequestT &_internalReq,
         gz::msgs::Empty &/*_internalRep*/)
    {
      auto cb = std::bind(_cb, _obj, std::placeholders::_1);
      cb(_internalReq);
      return true;
    };

    return this->Advertise(_topic, f, _options);
  }

  //////////////////////////////////////////////////
  template<typename RequestT, typename ReplyT>
  bool Node::Request(
    const std::string &_topic,
    const RequestT &_request,
    void(*_cb)(const ReplyT &_reply, const bool _result))
  {
    std::function<void(const ReplyT &, const bool)> f =
      [_cb](const ReplyT &_internalRep, const bool _internalResult)
    {
      (*_cb)(_internalRep, _internalResult);
    };

    return this->Request<RequestT, ReplyT>(_topic, _request, f);
  }

  //////////////////////////////////////////////////
  template<typename ReplyT>
  bool Node::Request(
    const std::string &_topic,
    void(*_cb)(const ReplyT &_reply, const bool _result))
  {
    msgs::Empty req;
    return this->Request(_topic, req, _cb);
  }

  //////////////////////////////////////////////////
  template<typename RequestT, typename ReplyT>
  bool Node::Request(
    const std::string &_topic,
    const RequestT &_request,
    std::function<void(const ReplyT &_reply, const bool _result)> &_cb)
  {
    // Topic remapping.
    std::string topic = _topic;
    this->Options().TopicRemap(_topic, topic);

    std::string fullyQualifiedTopic;
    if (!TopicUtils::FullyQualifiedName(this->Options().Partition(),
      this->Options().NameSpace(), topic, fullyQualifiedTopic))
    {
      std::cerr << "Service [" << topic << "] is not valid." << std::endl;
      return false;
    }

    bool localResponserFound;
    IRepHandlerPtr repHandler;
    {
      std::lock_guard<std::recursive_mutex> lk(this->Shared()->mutex);
      localResponserFound = this->Shared()->repliers.FirstHandler(
            fullyQualifiedTopic,
            std::string(RequestT().GetTypeName()),
            std::string(ReplyT().GetTypeName()),
            repHandler);
    }

    // If the responser is within my process.
    if (localResponserFound)
    {
      // There is a responser in my process, let's use it.
      ReplyT rep;
      bool result = repHandler->RunLocalCallback(_request, rep);

      _cb(rep, result);
      return true;
    }

    // Create a new request handler.
    std::shared_ptr<ReqHandler<RequestT, ReplyT>> reqHandlerPtr(
      new ReqHandler<RequestT, ReplyT>(this->NodeUuid()));

    // Insert the request's parameters.
    reqHandlerPtr->SetMessage(&_request);

    // Insert the callback into the handler.
    reqHandlerPtr->SetCallback(_cb);

    {
      std::lock_guard<std::recursive_mutex> lk(this->Shared()->mutex);

      // Store the request handler.
      this->Shared()->requests.AddHandler(
        fullyQualifiedTopic, this->NodeUuid(), reqHandlerPtr);

      // If the responser's address is known, make the request.
      SrvAddresses_M addresses;
      if (this->Shared()->TopicPublishers(fullyQualifiedTopic, addresses))
      {
        this->Shared()->SendPendingRemoteReqs(fullyQualifiedTopic,
          std::string(RequestT().GetTypeName()),
          std::string(ReplyT().GetTypeName()));
      }
      else
      {
        // Discover the service responser.
        if (!this->Shared()->DiscoverService(fullyQualifiedTopic))
        {
          std::cerr << "Node::Request(): Error discovering service ["
                    << topic
                    << "]. Did you forget to start the discovery service?"
                    << std::endl;
          return false;
        }
      }
    }

    return true;
  }

  //////////////////////////////////////////////////
  template<typename ReplyT>
  bool Node::Request(
    const std::string &_topic,
    std::function<void(const ReplyT &_reply, const bool _result)> &_cb)
  {
    msgs::Empty req;
    return this->Request(_topic, req, _cb);
  }

  //////////////////////////////////////////////////
  template<typename ClassT, typename RequestT, typename ReplyT>
  bool Node::Request(
    const std::string &_topic,
    const RequestT &_request,
    void(ClassT::*_cb)(const ReplyT &_reply, const bool _result),
    ClassT *_obj)
  {
    std::function<void(const ReplyT &, const bool)> f =
      [_cb, _obj](const ReplyT &_internalRep, const bool _internalResult)
    {
      auto cb = std::bind(_cb, _obj, std::placeholders::_1,
        std::placeholders::_2);
      cb(_internalRep, _internalResult);
    };

    return this->Request<RequestT, ReplyT>(_topic, _request, f);
  }

  //////////////////////////////////////////////////
  template<typename ClassT, typename ReplyT>
  bool Node::Request(
    const std::string &_topic,
    void(ClassT::*_cb)(const ReplyT &_reply, const bool _result),
    ClassT *_obj)
  {
    msgs::Empty req;
    return this->Request(_topic, req, _cb, _obj);
  }

  //////////////////////////////////////////////////
  template<typename RequestT, typename ReplyT>
  bool Node::Request(
          const std::string &_topic,
          const RequestT &_request,
          const unsigned int &_timeout,
          ReplyT &_reply,
          bool &_result)
  {
    // Topic remapping.
    std::string topic = _topic;
    this->Options().TopicRemap(_topic, topic);

    std::string fullyQualifiedTopic;
    if (!TopicUtils::FullyQualifiedName(this->Options().Partition(),
      this->Options().NameSpace(), topic, fullyQualifiedTopic))
    {
      std::cerr << "Service [" << topic << "] is not valid." << std::endl;
      return false;
    }

    // Create a new request handler.
    std::shared_ptr<ReqHandler<RequestT, ReplyT>> reqHandlerPtr(
      new ReqHandler<RequestT, ReplyT>(this->NodeUuid()));

    // Insert the request's parameters.
    reqHandlerPtr->SetMessage(&_request);
    reqHandlerPtr->SetResponse(&_reply);

    bool localResponserFound;
    IRepHandlerPtr repHandler;
    {
      std::lock_guard<std::recursive_mutex> lk(this->Shared()->mutex);
      localResponserFound = this->Shared()->repliers.FirstHandler(
          fullyQualifiedTopic, std::string(_request.GetTypeName()),
          std::string(_reply.GetTypeName()), repHandler);
    }

    // If the responser is within my process.
    if (localResponserFound)
    {
      // There is a responser in my process, let's use it.
      _result = repHandler->RunLocalCallback(_request, _reply);
      return true;
    }

    std::unique_lock<std::recursive_mutex> lk(this->Shared()->mutex);
    // Store the request handler.
    this->Shared()->requests.AddHandler(
      fullyQualifiedTopic, this->NodeUuid(), reqHandlerPtr);

    // If the responser's address is known, make the request.
    SrvAddresses_M addresses;
    if (this->Shared()->TopicPublishers(fullyQualifiedTopic, addresses))
    {
      this->Shared()->SendPendingRemoteReqs(fullyQualifiedTopic,
        std::string(_request.GetTypeName()),
        std::string(_reply.GetTypeName()));
    }
    else
    {
      // Discover the service responser.
      if (!this->Shared()->DiscoverService(fullyQualifiedTopic))
      {
        std::cerr << "Node::Request(): Error discovering service ["
                  << topic
                  << "]. Did you forget to start the discovery service?"
                  << std::endl;
        return false;
      }
    }

    // Wait until the REP is available.
    bool executed = reqHandlerPtr->WaitUntil(lk, _timeout);

    // The request was not executed.
    if (!executed)
      return false;

    // The request was executed but did not succeed.
    if (!reqHandlerPtr->Result())
    {
      _result = false;
      return true;
    }

    // Parse the response.
    if (!_reply.ParseFromString(reqHandlerPtr->Response()))
    {
      std::cerr << "Node::Request(): Error Parsing the response"
                << std::endl;
      _result = false;
      return true;
    }

    _result = true;
    return true;
  }

  //////////////////////////////////////////////////
  template<typename ReplyT>
  bool Node::Request(
    const std::string &_topic,
    const unsigned int &_timeout,
    ReplyT &_reply,
    bool &_result)
  {
    msgs::Empty req;
    return this->Request(_topic, req, _timeout, _reply, _result);
  }

  //////////////////////////////////////////////////
  template<typename RequestT>
  bool Node::Request(
      const std::string &_topic,
      const RequestT &_request)
  {
    // This callback is here for reusing the regular Request() call with
    // input and output parameters.
    std::function<void(const gz::msgs::Empty &, const bool)> f =
      [](const gz::msgs::Empty &, const bool)
    {
    };

    return this->Request<RequestT, gz::msgs::Empty>(
          _topic, _request, f);
  }
}

#endif
