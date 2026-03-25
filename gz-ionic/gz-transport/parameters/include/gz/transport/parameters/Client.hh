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

#ifndef GZ_TRANSPORT_PARAMETERS_CLIENT_HH_
#define GZ_TRANSPORT_PARAMETERS_CLIENT_HH_

#include <memory>
#include <string>

#include "google/protobuf/message.h"

#include "gz/msgs/parameter_declarations.pb.h"

#include "gz/transport/config.hh"
#include "gz/transport/Node.hh"
#include "gz/transport/parameters/Export.hh"
#include "gz/transport/parameters/Interface.hh"

namespace gz::transport::parameters
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {

  struct ParametersClientPrivate;

  /// \brief Allow to get, set, declare or list parameters
  /// \brief in a remote registry.
  class GZ_TRANSPORT_PARAMETERS_VISIBLE ParametersClient
  : public ParametersInterface
  {
    /// \brief Constructor.
    /// \param[in] _serverNamespace Namespace of the parameters registry
    ///    services. The client will send requests to:
    ///   * /${_serverNamespace}/get_parameter
    ///   * /${_serverNamespace}/list_parameters
    ///   * /${_serverNamespace}/set_parameter
    ///   * /${_serverNamespace}/declare_parameter
    /// \param[in] _timeoutMs Time to wait for the server to respond.
    public: ParametersClient(
      const std::string & _serverNamespace = "",
      unsigned int _timeoutMs = kDefaultTimeoutMs);

    /// \brief Destructor.
    public: ~ParametersClient();

    /// \brief No copy constructor.
    public: ParametersClient(const ParametersClient &) = delete;
    /// \brief No copy assignment.
    public: ParametersClient & operator=(const ParametersClient &) = delete;
    /// \brief Default move constructor.
    public: ParametersClient(ParametersClient &&);
    /// \brief Default move assignment.
    public: ParametersClient & operator=(ParametersClient &&);

    /// \brief Declare a new parameter.
    /// See ParametersInterface::DeclareParameter().
    public: ParameterResult DeclareParameter(
      const std::string & _parameterName,
      const google::protobuf::Message & _msg) final;

    /// \brief Request the value of a parameter.
    /// See ParametersInterface::Parameter().
    public: ParameterResult Parameter(
      const std::string & _parameterName,
      google::protobuf::Message & _parameter) const final;

    public: ParameterResult Parameter(
      const std::string & _parameterName,
      std::unique_ptr<google::protobuf::Message> & _parameter) const final;

    /// \brief Set the value of a parameter.
    /// See ParametersInterface::SetParameter().
    public: ParameterResult SetParameter(
      const std::string & _parameterName,
      const google::protobuf::Message & _msg) final;

    /// \brief List all parameters.
    /// \return Protobuf message with a list of all declared parameter
    ///   names and their types.
    public: gz::msgs::ParameterDeclarations ListParameters() const final;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Pointer to implementation.
    private: std::unique_ptr<ParametersClientPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif

    private: constexpr static inline unsigned int kDefaultTimeoutMs = 5000;
  };
  }
}

#endif
