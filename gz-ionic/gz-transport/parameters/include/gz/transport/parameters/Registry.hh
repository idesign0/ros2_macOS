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

#ifndef GZ_TRANSPORT_PARAMETERS_REGISTRY_HH_
#define GZ_TRANSPORT_PARAMETERS_REGISTRY_HH_

#include <functional>
#include <memory>
#include <string>

#include <google/protobuf/message.h>

#include <gz/msgs/parameter_declarations.pb.h>

#include "gz/transport/config.hh"
#include "gz/transport/parameters/result.hh"
#include "gz/transport/parameters/Export.hh"
#include "gz/transport/parameters/Interface.hh"

namespace gz::transport::parameters
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {

  struct ParametersRegistryPrivate;

  /// \brief Provides a parameter registry.
  /// Parameters can be declared, get or set in the registry.
  /// It also provides services, so the parameters can be get, set or
  /// listed from other processes.
  ///
  /// Provided services:
  /// * /${_parametersServicesNamespace}/get_parameter
  /// * /${_parametersServicesNamespace}/list_parameters
  /// * /${_parametersServicesNamespace}/set_parameter
  /// * /${_parametersServicesNamespace}/declare_parameter
  class GZ_TRANSPORT_PARAMETERS_VISIBLE ParametersRegistry
  : public ParametersInterface
  {
    /// \brief Constructor.
    /// \param[in] _parametersServicesNamespace Namespace that will be used
    ///   in all the created services names.
    public: explicit ParametersRegistry(
      const std::string & _parametersServicesNamespace);

    /// \brief Destructor.
    public: ~ParametersRegistry();

    /// \brief No copy constructor.
    public: ParametersRegistry(const ParametersRegistry &) = delete;
    /// \brief No copy assignment.
    public: ParametersRegistry & operator=(const ParametersRegistry &) = delete;
    /// \brief Default move constructor.
    public: ParametersRegistry(ParametersRegistry &&);
    /// \brief Default move assignment.
    public: ParametersRegistry & operator=(ParametersRegistry &&);

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

    /// \brief Declare a new parameter.
    /// \param[in] _parameterName Name of the parameter.
    /// \param[in] _initialValue The initial value of the parameter.
    ///   The parameter type will be deduced from the type of the message.
    /// \throw std::invalid_argument if `_initialValue` is `nullptr`.
    /// \throw ParameterAlreadyDeclaredException if a parameter with the
    ///   same name was declared before.
    public: ParameterResult DeclareParameter(
      const std::string & _parameterName,
      std::unique_ptr<google::protobuf::Message> _initialValue);

    /// \brief Set the value of a parameter.
    /// \param[in] _parameterName Name of the parameter to set.
    /// \param[in] _value The value of the parameter.
    /// \throw ParameterNotDeclaredException if a parameter of that name
    ///   was not declared before.
    /// \throw ParameterInvalidTypeException if the type does not match
    ///   the type of the parameter when it was declared.
    public: ParameterResult SetParameter(
      const std::string & _parameterName,
      std::unique_ptr<google::protobuf::Message> _value);

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::unique_ptr
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \brief Pointer to implementation.
    private: std::unique_ptr<ParametersRegistryPrivate> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}

#endif
