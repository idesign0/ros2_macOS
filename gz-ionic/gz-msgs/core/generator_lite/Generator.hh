/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#ifndef GZ_MSGS_GENERATOR_HH_
#define GZ_MSGS_GENERATOR_HH_

#include <google/protobuf/compiler/code_generator.h>
#include <string>

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {
/// \cond
/// \brief Google protobuf message generator for gz::msgs
class Generator : public CodeGenerator
{
  /// \brief Constructor
  /// \param[in] _name Name value (currently unused)
  public: explicit Generator(const std::string &_name);

  /// \brief Destructor
  public: virtual ~Generator();

  /// \brief Generate a message.
  /// \param[in] _file File descriptor of the message.
  /// \param[in] _parameter Unused string value
  /// \param[in] _generatorContext Output directory.
  /// \param[in] _error Unused string value
  /// \return true if successful, otherwise sets _error and returns false
  public: virtual bool Generate(const FileDescriptor *_file,
              const std::string &_parameter,
              OutputDirectory *_generatorContext,
              std::string *_error) const;

};
/// \endcond
}
}
}
}
#endif
