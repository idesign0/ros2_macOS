/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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

#include "gz/transport/parameters/Registry.hh"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "google/protobuf/message.h"
#include "google/protobuf/any.h"

#include "gz/transport/Node.hh"

#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/parameter.pb.h>
#include <gz/msgs/parameter_declarations.pb.h>
#include <gz/msgs/parameter_error.pb.h>
#include <gz/msgs/parameter_name.pb.h>
#include <gz/msgs/parameter_value.pb.h>

#include <gz/transport/parameters/result.hh>

#include "Utils.hh"

namespace gz::transport::parameters
{
// Inline bracket to help doxygen filtering.
inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
struct ParametersRegistryPrivate
{
  using ParametersMapT = std::unordered_map<
    std::string, std::unique_ptr<google::protobuf::Message>>;

  /// \brief Get parameter service callback.
  /// \param[in] _req Request specifying the parameter name.
  /// \param[out] _res The value of the parameter.
  /// \return True if successful.
  bool GetParameter(const msgs::ParameterName &_req,
    msgs::ParameterValue &_res);

  /// \brief List parameter service callback.
  /// \param[in] _req unused.
  /// \param[out] _res List of available parameters.
  /// \return True if successful.
  bool ListParameters(const msgs::Empty &_req,
    msgs::ParameterDeclarations &_res);

  /// \brief Set parameter service callback.
  /// \param[in] _req Request specifying which parameter to set and its value.
  /// \param[out] _res Unused.
  /// \return True if successful.
  bool SetParameter(const msgs::Parameter &_req, msgs::ParameterError &_res);

  /// \brief Declare parameter service callback.
  /// \param[in] _req Request specifying which parameter to be declared.
  /// \param[out] _res Unused.
  /// \return True if successful.
  bool DeclareParameter(
    const msgs::Parameter &_req, msgs::ParameterError &_res);

  transport::Node node;
  std::mutex parametersMapMutex;
  ParametersMapT parametersMap;
};
}  // namespace GZ_TRANSPORT_VERSION_NAMESPACE

//////////////////////////////////////////////////
ParametersRegistry::ParametersRegistry(
  const std::string &  _parametersServicesNamespace)
  : dataPtr{std::make_unique<ParametersRegistryPrivate>()}
{
  std::string getParameterSrvName{
    _parametersServicesNamespace + "/get_parameter"};
  this->dataPtr->node.Advertise(getParameterSrvName,
    &ParametersRegistryPrivate::GetParameter, this->dataPtr.get());

  std::string listParametersSrvName{
    _parametersServicesNamespace + "/list_parameters"};
  this->dataPtr->node.Advertise(listParametersSrvName,
    &ParametersRegistryPrivate::ListParameters, this->dataPtr.get());

  std::string setParameterSrvName{
    _parametersServicesNamespace + "/set_parameter"};
  this->dataPtr->node.Advertise(setParameterSrvName,
    &ParametersRegistryPrivate::SetParameter, this->dataPtr.get());

  std::string declareParameterSrvName{
    _parametersServicesNamespace + "/declare_parameter"};
  this->dataPtr->node.Advertise(declareParameterSrvName,
    &ParametersRegistryPrivate::DeclareParameter, this->dataPtr.get());
}

//////////////////////////////////////////////////
ParametersRegistry::~ParametersRegistry() = default;

//////////////////////////////////////////////////
ParametersRegistry::ParametersRegistry(ParametersRegistry &&) = default;

//////////////////////////////////////////////////
ParametersRegistry & ParametersRegistry::operator=(
  ParametersRegistry &&) = default;

//////////////////////////////////////////////////
bool ParametersRegistryPrivate::GetParameter(const msgs::ParameterName &_req,
  msgs::ParameterValue &_res)
{
  std::lock_guard guard{this->parametersMapMutex};
  auto it = this->parametersMap.find(_req.name());
  if (it == this->parametersMap.end()) {
    return false;
  }
#if PROTOBUF_VERSION < 3015000
  _res.mutable_data()->PackFrom(*it->second, "gz_msgs");
  return true;
#else
  return _res.mutable_data()->PackFrom(*it->second, "gz_msgs");
#endif
}

//////////////////////////////////////////////////
bool ParametersRegistryPrivate::ListParameters(const msgs::Empty &,
  msgs::ParameterDeclarations &_res)
{
  // TODO(ivanpauno): Maybe the response should only include parameter names,
  // maybe only names and types (?)
  // Including the component key doesn't seem to matter much,
  // though it's also not wrong.
  {
    std::lock_guard guard{this->parametersMapMutex};
    for (const auto & paramPair : this->parametersMap) {
      auto * decl = _res.add_parameter_declarations();
      decl->set_name(paramPair.first);
      decl->set_type(addGzMsgsPrefix(
        std::string(paramPair.second->GetDescriptor()->name())));
    }
  }
  return true;
}

//////////////////////////////////////////////////
bool ParametersRegistryPrivate::SetParameter(
  const msgs::Parameter &_req, msgs::ParameterError &_res)
{
  (void)_res;
  const auto & paramName = _req.name();
  {
    std::lock_guard guard{this->parametersMapMutex};
    auto it = this->parametersMap.find(paramName);
    if (it == this->parametersMap.end()) {
      _res.set_data(msgs::ParameterError::NOT_DECLARED);
      return true;
    }
    auto requestedGzTypeOpt = getGzTypeFromAnyProto(
      _req.value());
    if (!requestedGzTypeOpt) {
      _res.set_data(msgs::ParameterError::INVALID_TYPE);
      return true;
    }
    auto requestedGzType = *requestedGzTypeOpt;
    if (it->second->GetDescriptor()->name() != requestedGzType) {
      _res.set_data(msgs::ParameterError::INVALID_TYPE);
      return true;
    }
    if (!_req.value().UnpackTo(it->second.get())) {
      // unexpected error
      return false;
    }
  }
  return true;
}

//////////////////////////////////////////////////
bool ParametersRegistryPrivate::DeclareParameter(
  const msgs::Parameter &_req, msgs::ParameterError &_res)
{
  (void)_res;
  const auto & gzTypeOpt = getGzTypeFromAnyProto(_req.value());
  if (!gzTypeOpt) {
    _res.set_data(msgs::ParameterError::INVALID_TYPE);
    return true;
  }
  auto gzType = addGzMsgsPrefix(*gzTypeOpt);
  auto paramValue = gz::msgs::Factory::New(gzType);
  if (!paramValue) {
    _res.set_data(msgs::ParameterError::INVALID_TYPE);
    return true;
  }
  if (!_req.value().UnpackTo(paramValue.get())) {
    // unexpected error
    return false;
  }
  std::lock_guard guard{this->parametersMapMutex};
  auto it_emplaced_pair = this->parametersMap.emplace(
    std::make_pair(_req.name(), std::move(paramValue)));
  if (!it_emplaced_pair.second) {
    _res.set_data(msgs::ParameterError::ALREADY_DECLARED);
  }
  return true;
}

//////////////////////////////////////////////////
ParameterResult
ParametersRegistry::DeclareParameter(
  const std::string & _parameterName,
  std::unique_ptr<google::protobuf::Message> _initialValue)
{
  if (!_initialValue) {
    throw std::invalid_argument{
      "ParametersRegistry::DeclareParameter(): `_parameterName` is nullptr"};
  }
  std::lock_guard guard{this->dataPtr->parametersMapMutex};
  auto it_emplaced_pair = this->dataPtr->parametersMap.emplace(
    std::make_pair(_parameterName, std::move(_initialValue)));
  if (!it_emplaced_pair.second) {
    return ParameterResult{
      ParameterResultType::AlreadyDeclared,
      _parameterName};
  }
  return ParameterResult{ParameterResultType::Success};
}

//////////////////////////////////////////////////
ParameterResult
ParametersRegistry::DeclareParameter(
  const std::string & _parameterName,
  const google::protobuf::Message & _msg)
{
  auto protoType = addGzMsgsPrefix(std::string(_msg.GetDescriptor()->name()));
  auto newParam = gz::msgs::Factory::New(protoType);
  if (!newParam) {
    return ParameterResult{
      ParameterResultType::Unexpected,
      _parameterName,
      protoType};
  }
  newParam->CopyFrom(_msg);
  this->DeclareParameter(_parameterName, std::move(newParam));
  return ParameterResult{ParameterResultType::Success};
}

//////////////////////////////////////////////////
ParameterResult
ParametersRegistry::Parameter(
  const std::string & _parameterName,
  google::protobuf::Message & _parameter) const
{
  std::lock_guard guard{this->dataPtr->parametersMapMutex};
  auto it = dataPtr->parametersMap.find(_parameterName);
  if (it == dataPtr->parametersMap.end()) {
    return ParameterResult{
      ParameterResultType::NotDeclared,
      _parameterName};
  }
  const auto & newProtoType = _parameter.GetDescriptor()->name();
  const auto & protoType = it->second->GetDescriptor()->name();
  if (newProtoType != protoType) {
    return ParameterResult{
      ParameterResultType::InvalidType,
      _parameterName,
      addGzMsgsPrefix(std::string(protoType))};
  }
  _parameter.CopyFrom(*it->second);
  return ParameterResult{ParameterResultType::Success};
}

//////////////////////////////////////////////////
ParameterResult
ParametersRegistry::Parameter(
  const std::string & _parameterName,
  std::unique_ptr<google::protobuf::Message> & _parameter) const
{
  std::lock_guard guard{this->dataPtr->parametersMapMutex};
  auto it = dataPtr->parametersMap.find(_parameterName);
  if (it == dataPtr->parametersMap.end()) {
    return ParameterResult{
      ParameterResultType::NotDeclared,
      _parameterName};
  }
  const auto & protoType = it->second->GetDescriptor()->name();
  _parameter = gz::msgs::Factory::New(std::string(protoType));
  if (!_parameter) {
    return ParameterResult{
      ParameterResultType::InvalidType,
      _parameterName,
      addGzMsgsPrefix(std::string(protoType))};

  }
  _parameter->CopyFrom(*it->second);
  return ParameterResult{ParameterResultType::Success};
}

//////////////////////////////////////////////////
ParameterResult
ParametersRegistry::SetParameter(
  const std::string & _parameterName,
  std::unique_ptr<google::protobuf::Message> _value)
{
  std::lock_guard guard{this->dataPtr->parametersMapMutex};
  auto it = this->dataPtr->parametersMap.find(_parameterName);
  if (it == this->dataPtr->parametersMap.end()) {
    return ParameterResult{
      ParameterResultType::NotDeclared,
      _parameterName};
  }
  // Validate the type matches before copying.
  if (it->second->GetDescriptor() != _value->GetDescriptor()) {
    return ParameterResult{
      ParameterResultType::InvalidType,
      _parameterName,
      std::string(addGzMsgsPrefix(
            std::string(it->second->GetDescriptor()->name())))};
  }
  it->second = std::move(_value);
  return ParameterResult{ParameterResultType::Success};
}

//////////////////////////////////////////////////
ParameterResult
ParametersRegistry::SetParameter(
  const std::string & _parameterName,
  const google::protobuf::Message & _value)
{
  std::lock_guard guard{this->dataPtr->parametersMapMutex};
  auto it = this->dataPtr->parametersMap.find(_parameterName);
  if (it == this->dataPtr->parametersMap.end()) {
    return ParameterResult{
      ParameterResultType::NotDeclared,
      _parameterName};
  }
  // Validate the type matches before copying.
  if (it->second->GetDescriptor() != _value.GetDescriptor()) {
    return ParameterResult{
      ParameterResultType::InvalidType,
      _parameterName};
  }
  it->second->CopyFrom(_value);
  return ParameterResult{ParameterResultType::Success};
}

//////////////////////////////////////////////////
gz::msgs::ParameterDeclarations
ParametersRegistry::ListParameters() const
{
  gz::msgs::ParameterDeclarations ret;
  gz::msgs::Empty unused;

  dataPtr->ListParameters(unused, ret);
  return ret;
}
}  // namespace gz::transport::parameters
