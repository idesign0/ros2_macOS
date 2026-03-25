/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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
#ifndef DYNAMIC_FACTORY_HH_
#define DYNAMIC_FACTORY_HH_

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4146 4251)
#endif

#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor_database.h>
#include <google/protobuf/dynamic_message.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace gz::msgs {

/////////////////////////////////////////////////
/// \brief A factory class to generate protobuf messages at runtime based on
/// their message descriptors. The location of the .desc files is expected
/// via the GZ_DESCRIPTOR_PATH environment variable. This environment
/// variable expects paths to directories containing .desc files.
/// Any file without the .desc or .gz_desc extension will be ignored.
class DynamicFactory
{
  public: using Message = google::protobuf::Message;
  public: using MessagePtr = std::unique_ptr<Message>;

  //////////////////////////////////////////////////
  /// \brief Constructor.
  /// The constructor will try to load all descriptors specified in the
  /// GZ_DESCRIPTOR_PATH environment variable.
  public: DynamicFactory();

  //////////////////////////////////////////////////
  /// \brief Load descriptors into the descriptor pool.
  /// \param[in] _paths A set of directories containing .desc descriptor files.
  /// Each directory should be separated by ":".
  public: void LoadDescriptors(const std::string &_paths);

  //////////////////////////////////////////////////
  /// \brief Create a new instance of a message.
  /// \param[in] _msgType Type of message to create.
  /// \return Pointer to a google protobuf message. Null if the message
  /// type could not be handled.
  public: MessagePtr New(const std::string &_msgType);

  //////////////////////////////////////////////////
    /// \brief Get all the message types
    /// \param[out] _types Vector of strings of the message types.
  public: void Types(std::vector<std::string> &_types);

  /// \typedef FactoryFn
  /// \brief Prototype for message factory generation
  private: using FactoryFn = std::function<MessagePtr(void)>;

  /// \brief A list of registered message types built at runtime.
  /// The key is the message type. The value is a function that returns a
  /// std::unique_ptr to a new empty instance of the message or nullptr if
  /// the message is not registered.
  private: std::map<std::string, FactoryFn> dynamicMsgMap;

  /// \brief We store the descriptors here.
  private: google::protobuf::DescriptorPool pool;

  /// \brief Descriptor database to allow us to retrieve all
  /// message types from the descriptor files
  private: google::protobuf::SimpleDescriptorDatabase db;

  /// \brief Used to create a message from a descriptor.
  private: google::protobuf::DynamicMessageFactory dynamicMessageFactory;
};

}  // namespace gz::msgs
#endif  // DYNAMIC_FACTORY_HH_
