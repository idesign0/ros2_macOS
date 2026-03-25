/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "gz/transport/AdvertiseOptions.hh"
#include "gz/transport/Helpers.hh"
#include "gz/transport/Publisher.hh"
#include "gz/transport/NodeShared.hh"
#include "gz/transport/SubscriptionHandler.hh"

namespace gz::transport
{
//////////////////////////////////////////////////
Publisher::Publisher(const std::string &_topic, const std::string &_addr,
  const std::string &_pUuid, const std::string &_nUuid,
  const AdvertiseOptions &_opts)
  : topic(_topic),
    addr(_addr),
    pUuid(_pUuid),
    nUuid(_nUuid),
    opts(_opts)
{
}

//////////////////////////////////////////////////
std::string Publisher::Topic() const
{
  return this->topic;
}

//////////////////////////////////////////////////
std::string Publisher::Addr() const
{
  return this->addr;
}

//////////////////////////////////////////////////
std::string Publisher::PUuid() const
{
  return this->pUuid;
}

//////////////////////////////////////////////////
std::string Publisher::NUuid() const
{
  return this->nUuid;
}

//////////////////////////////////////////////////
const AdvertiseOptions& Publisher::Options() const
{
  return this->opts;
}

//////////////////////////////////////////////////
void Publisher::SetTopic(const std::string &_topic)
{
  this->topic = _topic;
}

//////////////////////////////////////////////////
void Publisher::SetAddr(const std::string &_addr)
{
  this->addr = _addr;
}

//////////////////////////////////////////////////
void Publisher::SetPUuid(const std::string &_pUuid)
{
  this->pUuid = _pUuid;
}

//////////////////////////////////////////////////
void Publisher::SetNUuid(const std::string &_nUuid)
{
  this->nUuid = _nUuid;
}

//////////////////////////////////////////////////
void Publisher::SetOptions(const AdvertiseOptions &_opts)
{
  this->opts = _opts;
}

//////////////////////////////////////////////////
void Publisher::FillDiscovery(msgs::Discovery &_msg) const
{
  msgs::Discovery::Publisher *pub = _msg.mutable_pub();
  pub->set_topic(this->Topic());
  pub->set_address(this->Addr());
  pub->set_process_uuid(this->PUuid());
  pub->set_node_uuid(this->NUuid());

  switch (this->opts.Scope())
  {
    case Scope_t::PROCESS:
      pub->set_scope(msgs::Discovery::Publisher::PROCESS);
      break;
    case Scope_t::HOST:
      pub->set_scope(msgs::Discovery::Publisher::HOST);
      break;
    default:
    case Scope_t::ALL:
      pub->set_scope(msgs::Discovery::Publisher::ALL);
      break;
  }
}

//////////////////////////////////////////////////
void Publisher::SetFromDiscovery(const msgs::Discovery &_msg)
{
  if (_msg.has_sub())
    this->topic = _msg.sub().topic();
  else if (_msg.has_pub())
  {
    this->topic = _msg.pub().topic();
    this->addr = _msg.pub().address();
    this->pUuid = _msg.pub().process_uuid();
    this->nUuid = _msg.pub().node_uuid();

    switch (_msg.pub().scope())
    {
      case msgs::Discovery::Publisher::PROCESS:
        this->opts.SetScope(Scope_t::PROCESS);
        break;
      case msgs::Discovery::Publisher::HOST:
        this->opts.SetScope(Scope_t::HOST);
        break;
      default:
      case msgs::Discovery::Publisher::ALL:
        this->opts.SetScope(Scope_t::ALL);
        break;
    }
  }
}

//////////////////////////////////////////////////
bool Publisher::operator==(const Publisher &_pub) const
{
  return this->topic == _pub.topic && this->addr == _pub.addr &&
    this->pUuid == _pub.pUuid && this->nUuid == _pub.nUuid &&
    this->Options() == _pub.Options();
}

//////////////////////////////////////////////////
bool Publisher::operator!=(const Publisher &_pub) const
{
  return !(*this == _pub);
}

//////////////////////////////////////////////////
MessagePublisher::MessagePublisher(const std::string &_topic,
  const std::string &_addr, const std::string &_ctrl, const std::string &_pUuid,
  const std::string &_nUuid, const std::string &_msgTypeName,
  const AdvertiseMessageOptions &_opts)
  : Publisher(_topic, _addr, _pUuid, _nUuid, _opts),
    ctrl(_ctrl),
    msgTypeName(_msgTypeName),
    msgOpts(_opts)
{
}

//////////////////////////////////////////////////
std::string MessagePublisher::Ctrl() const
{
  return this->ctrl;
}

//////////////////////////////////////////////////
void MessagePublisher::SetCtrl(const std::string &_ctrl)
{
  this->ctrl = _ctrl;
}

//////////////////////////////////////////////////
std::string MessagePublisher::MsgTypeName() const
{
  return this->msgTypeName;
}

//////////////////////////////////////////////////
void MessagePublisher::SetMsgTypeName(const std::string &_msgTypeName)
{
  this->msgTypeName = _msgTypeName;
}

//////////////////////////////////////////////////
const AdvertiseMessageOptions& MessagePublisher::Options() const
{
  return this->msgOpts;
}

//////////////////////////////////////////////////
void MessagePublisher::SetOptions(const AdvertiseMessageOptions &_opts)
{
  this->msgOpts = _opts;
}

//////////////////////////////////////////////////
void MessagePublisher::FillDiscovery(msgs::Discovery &_msg) const
{
  Publisher::FillDiscovery(_msg);
  msgs::Discovery::Publisher *pub = _msg.mutable_pub();

  // Message options
  pub->mutable_msg_pub()->set_ctrl(this->Ctrl());
  pub->mutable_msg_pub()->set_msg_type(this->MsgTypeName());
  pub->mutable_msg_pub()->set_throttled(this->msgOpts.Throttled());
  pub->mutable_msg_pub()->set_msgs_per_sec(this->msgOpts.MsgsPerSec());
}

//////////////////////////////////////////////////
void MessagePublisher::SetFromDiscovery(const msgs::Discovery &_msg)
{
  Publisher::SetFromDiscovery(_msg);
  this->ctrl = _msg.pub().msg_pub().ctrl();
  this->msgTypeName = _msg.pub().msg_pub().msg_type();
  this->msgOpts.SetScope(Publisher::Options().Scope());
  if (!_msg.pub().msg_pub().throttled())
    this->msgOpts.SetMsgsPerSec(kUnthrottled);
  else
    this->msgOpts.SetMsgsPerSec(_msg.pub().msg_pub().msgs_per_sec());
}

//////////////////////////////////////////////////
bool MessagePublisher::operator==(const MessagePublisher &_pub) const
{
  return Publisher::operator==(_pub)      &&
    this->ctrl == _pub.ctrl               &&
    this->msgTypeName == _pub.msgTypeName;
}

//////////////////////////////////////////////////
bool MessagePublisher::operator!=(const MessagePublisher &_pub) const
{
  return !(*this == _pub);
}

//////////////////////////////////////////////////
ServicePublisher::ServicePublisher(const std::string &_topic,
  const std::string &_addr, const std::string &_socketId,
  const std::string &_pUuid, const std::string &_nUuid,
  const std::string &_reqType, const std::string &_repType,
  const AdvertiseServiceOptions &_opts)
  : Publisher(_topic, _addr, _pUuid, _nUuid, _opts),
    socketId(_socketId),
    reqTypeName(_reqType),
    repTypeName(_repType),
    srvOpts(_opts)
{
}


//////////////////////////////////////////////////
std::string ServicePublisher::SocketId() const
{
  return this->socketId;
}

//////////////////////////////////////////////////
void ServicePublisher::SetSocketId(const std::string &_socketId)
{
  this->socketId = _socketId;
}

//////////////////////////////////////////////////
std::string ServicePublisher::ReqTypeName() const
{
  return this->reqTypeName;
}

//////////////////////////////////////////////////
std::string ServicePublisher::RepTypeName() const
{
  return this->repTypeName;
}

//////////////////////////////////////////////////
void ServicePublisher::SetReqTypeName(const std::string &_reqTypeName)
{
  this->reqTypeName = _reqTypeName;
}

//////////////////////////////////////////////////
void ServicePublisher::SetRepTypeName(const std::string &_repTypeName)
{
  this->repTypeName = _repTypeName;
}

//////////////////////////////////////////////////
const AdvertiseServiceOptions& ServicePublisher::Options() const
{
  return this->srvOpts;
}

//////////////////////////////////////////////////
void ServicePublisher::SetOptions(const AdvertiseServiceOptions &_opts)
{
  this->srvOpts = _opts;
}

//////////////////////////////////////////////////
void ServicePublisher::FillDiscovery(msgs::Discovery &_msg) const
{
  Publisher::FillDiscovery(_msg);
  msgs::Discovery::Publisher *pub = _msg.mutable_pub();

  // Service publisher info
  pub->mutable_srv_pub()->set_socket_id(this->SocketId());
  pub->mutable_srv_pub()->set_request_type(this->ReqTypeName());
  pub->mutable_srv_pub()->set_response_type(this->RepTypeName());
}

//////////////////////////////////////////////////
void ServicePublisher::SetFromDiscovery(const msgs::Discovery &_msg)
{
  Publisher::SetFromDiscovery(_msg);
  this->srvOpts.SetScope(Publisher::Options().Scope());
  this->socketId = _msg.pub().srv_pub().socket_id();
  this->reqTypeName = _msg.pub().srv_pub().request_type();
  this->repTypeName = _msg.pub().srv_pub().response_type();
}

//////////////////////////////////////////////////
bool ServicePublisher::operator==(const ServicePublisher &_srv) const
{
  return Publisher::operator==(_srv)      &&
    this->socketId == _srv.socketId       &&
    this->reqTypeName == _srv.reqTypeName &&
    this->repTypeName == _srv.repTypeName;
}

//////////////////////////////////////////////////
bool ServicePublisher::operator!=(const ServicePublisher &_srv) const
{
  return !(*this == _srv);
}
}  // namespace gz::transport
