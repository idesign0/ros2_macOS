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

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "DynamicFactory.hh"
#include "gz/utils/Environment.hh"

#include <gz/msgs/InstallationDirectories.hh>

namespace {
constexpr const char * kDescriptorEnv = "GZ_DESCRIPTOR_PATH";
#ifdef _WIN32
constexpr char kEnvironmentVariableSeparator = ';';
#else
constexpr char kEnvironmentVariableSeparator = ':';
#endif

//////////////////////////////////////////////////
/// \brief split at a one character delimiter to get a vector of something
/// \param[in] _orig The string to split
/// \param[in] _delim a character to split the string at
/// \returns vector of split pieces of the string excluding the delimiter
std::vector<std::string> split(const std::string &_orig, char _delim)
{
  std::vector<std::string> pieces;
  size_t pos1 = 0;
  size_t pos2 = _orig.find(_delim);
  while (pos2 != std::string::npos)
  {
    pieces.push_back(_orig.substr(pos1, pos2-pos1));
    pos1 = pos2+1;
    pos2 = _orig.find(_delim, pos2+1);
  }
  pieces.push_back(_orig.substr(pos1, _orig.size()-pos1));
  return pieces;
}
}  // namespace

namespace gz::msgs {

//////////////////////////////////////////////////
DynamicFactory::DynamicFactory()
{
  // Try to get the list of paths from an environment variable.
  std::string descPaths;
  if (gz::utils::env(kDescriptorEnv, descPaths)) {
    // Load all the descriptors found in the paths set with GZ_DESCRIPTOR_PATH.
    this->LoadDescriptors(descPaths);
  }

  auto globalPath =
    std::filesystem::path(gz::msgs::getInstallPrefix()) /
    "share" / "gz" / "protos";

  if (std::filesystem::exists(globalPath))
  {
    // Load descriptors from the global share path
    this->LoadDescriptors(globalPath.string());
  }
}

//////////////////////////////////////////////////
void DynamicFactory::LoadDescriptors(const std::string &_paths)
{
  if (_paths.empty())
    return;

  // Split all the directories containing .desc files.
  std::vector<std::string> descDirs =
    split(_paths, kEnvironmentVariableSeparator);

  auto loadDescFile = [this](const std::string &descFile)
  {
    // Ignore files without the .desc extension.
    if (descFile.rfind(".desc") == std::string::npos &&
        descFile.rfind(".gz_desc") == std::string::npos &&
        descFile.rfind(".proto") == std::string::npos &&
        descFile.rfind(".proto.bin") == std::string::npos)
      return;

    // Parse the .desc file.
    std::ifstream ifs(descFile);
    if (!ifs.is_open())
    {
      std::cerr << "DynamicFactory(): Unable to open [" << descFile << "]"
                << std::endl;
      return;
    }

    google::protobuf::FileDescriptorSet fileDescriptorSet;
    if (!fileDescriptorSet.ParseFromIstream(&ifs))
    {
      std::cerr << "DynamicFactory(): Unable to parse descriptor set from ["
                << descFile << "]" << std::endl;
      return;
    }

    // Place the real descriptors in the descriptor pool.
    for (const google::protobuf::FileDescriptorProto &fileDescriptorProto :
         fileDescriptorSet.file())
    {
      if (!static_cast<bool>(this->pool.BuildFile(fileDescriptorProto)))
      {
        std::cerr << "DynamicFactory(). Unable to place descriptors from ["
                  << descFile << "] in the descriptor pool" << std::endl;
      }
      else
      {
        this->db.Add(fileDescriptorProto);
      }
    }
  };

  for (const std::string &descDir : descDirs)
  {
    if (!std::filesystem::is_directory(descDir))
    {
      loadDescFile(descDir);
    }
    else
    {
      for (auto const &dirIter : std::filesystem::directory_iterator{descDir})
      {
        loadDescFile(dirIter.path().string());
      }
    }
  }
}

//////////////////////////////////////////////////
void DynamicFactory::Types(std::vector<std::string> &_types)
{
  std::vector<std::string> messages;
  if (this->db.FindAllMessageNames(&messages))
  {
    std::copy(messages.begin(), messages.end(), std::back_inserter(_types));
  }
  else
  {
    std::cerr << "DynamicFactory(). Unable to find all message names.\n";
  }
}

//////////////////////////////////////////////////
DynamicFactory::MessagePtr DynamicFactory::New(const std::string &_msgType)
{
  // Shortcut if the type has been already registered.
  auto messageIt = dynamicMsgMap.find(_msgType);
  if (messageIt != dynamicMsgMap.end())
    return messageIt ->second();

  // Nothing to do if we don't know about this type in the descriptor map.
  const auto *descriptor = pool.FindMessageTypeByName(_msgType);
  if (!static_cast<bool>(descriptor))
    return nullptr;

  google::protobuf::Message *msgPtr(
      dynamicMessageFactory.GetPrototype(descriptor)->New());

  // Create the lambda for registration purposes.
  auto f = [msgPtr]() -> MessagePtr
  {
    MessagePtr ptr(msgPtr->New());
    return ptr;
  };

  // Register the new type for the future.
  dynamicMsgMap[_msgType] = f;

  return f();
}
}  // namespace gz::msgs
