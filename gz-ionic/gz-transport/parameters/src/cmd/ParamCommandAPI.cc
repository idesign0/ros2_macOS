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

#include "ParamCommandAPI.hh"

#include <memory>
#include <string>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <gz/msgs/boolean.pb.h>
#include <gz/msgs/empty.pb.h>
#include <gz/msgs/parameter.pb.h>
#include <gz/msgs/parameter_declarations.pb.h>
#include <gz/msgs/parameter_name.pb.h>
#include <gz/msgs/parameter_value.pb.h>

#include <gz/transport/parameters/Client.hh>

using namespace gz;
using namespace transport;

//////////////////////////////////////////////////
extern "C" void cmdParametersList(const char * _ns)
{
  if (!_ns)
  {
    std::cerr << "Error: Namespace is null" << std::endl;
    return;
  }
  parameters::ParametersClient client{_ns};

  std::cout << std::endl << "Listing parameters, registry namespace [" << _ns
            << "]..." << std::endl << std::endl;
  try {
    auto res = client.ListParameters();
    if (!res.parameter_declarations_size()) {
      std::cout << "No parameters available" << std::endl;
      return;
    }
    for (const auto & decl : res.parameter_declarations()) {
      std::cout << decl.name() << "            [" << decl.type() << "]"
                << std::endl;
    }
  } catch (const std::runtime_error& e) {
    std::cerr << "Error listing parameters: " << e.what() << std::endl;
    std::cerr << "\nPossible causes:" << std::endl;
    std::cerr << "1. Parameter server not running" << std::endl;
    std::cerr << "2. Invalid namespace: '" << _ns << "'" << std::endl;
    std::cerr << "3. Network communication issue" << std::endl;
  }
}

//////////////////////////////////////////////////
extern "C" void cmdParameterGet(const char * _ns, const char *_paramName) {
  if (!_ns || !_paramName)
  {
    std::cerr << "Error: Namespace or parameter name is null" << std::endl;
    return;
  }
  parameters::ParametersClient client{_ns};

  std::cout << std::endl << "Getting parameter [" << _paramName
            << "] for registry namespace [" << _ns << "]..." << std::endl;

  std::unique_ptr<google::protobuf::Message> value;
  auto ret = client.Parameter(_paramName, value);
  if (!ret) {
    std::cerr << "Failed to get parameter: " << ret << std::endl;
    return;
  }

  std::string msgType = "gz_msgs.";
  msgType += value->GetDescriptor()->name();
  std::cout << "Parameter type [" << msgType << "]" << std::endl << std::endl
            << "------------------------------------------------"
            << std::endl;
  {
    google::protobuf::io::OstreamOutputStream fos{&std::cout};
    if (!google::protobuf::TextFormat::Print(*value, &fos)) {
      std::cerr << "failed to convert the parameter value to a string"
                << std::endl;
      return;
    }
  }
  std::cout << "------------------------------------------------"
            << std::endl;
}

extern "C" void cmdParameterSet(
    const char * _ns, const char *_paramName, const char * _paramType,
    const char *_paramValue)
{
  if (!_ns || !_paramName)
  {
    std::cerr << "Error: Namespace or parameter name is null" << std::endl;
    return;
  }
  if (!_paramType || !_paramValue)
  {
    std::cerr << "Error: Parameter type or value is null" << std::endl;
    return;
  }
  parameters::ParametersClient client{_ns};

  std::cout << std::endl << "Setting parameter [" << _paramName
            << "] for registry namespace [" << _ns << "]..." << std::endl;

  auto msg = gz::msgs::Factory::New(_paramType);
  if (!msg) {
    std::cerr << "Could not create a message of type [" << _paramType
              << "]. The message type is invalid."
              << std::endl;
    return;
  }
  if (!google::protobuf::TextFormat::ParseFromString(_paramValue, msg.get())) {
    std::cerr << "Could not create a message of type [" << _paramType
              << "]. The message string representation is invalid."
              << std::endl;
    return;
  }
  auto ret = client.SetParameter(_paramName, *msg);
  if (!ret) {
    std::cerr << "Failed to set parameter: " << ret << std::endl;
  }
  std::cout << "Parameter successfully set!" << std::endl;
}
