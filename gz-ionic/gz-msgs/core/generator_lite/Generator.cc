/*
 * Copyright 2016 Open Source Robotics Foundation
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
#include <filesystem>
#include <iostream>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100 4512 4127 4068 4244 4267 4251 4146)
#endif

#include <google/protobuf/descriptor.h>
#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/descriptor.pb.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "Generator.hh"

namespace google::protobuf::compiler::cpp {

/////////////////////////////////////////////////
Generator::Generator(const std::string &/*_name*/)
{
}

/////////////////////////////////////////////////
Generator::~Generator() = default;

/////////////////////////////////////////////////
bool Generator::Generate(const FileDescriptor *_file,
                               const std::string &/*_parameter*/,
                               OutputDirectory *_generatorContext,
                               std::string * /*_error*/) const
{
  std::string delim = ".proto";
  std::string headerFilename(_file->name());
  std::string sourceFilename(_file->name());

  {
    auto pos = headerFilename.rfind(delim);
    headerFilename.replace(pos, delim.size(), ".pb.h");
  }
  {
    auto pos = sourceFilename.rfind(delim);
    sourceFilename.replace(pos, delim.size(), ".pb.h");
  }

  {
    auto *output = _generatorContext->OpenForInsert(headerFilename, "includes");
    auto printer = io::Printer(output, '$');
    printer.Print("#include <memory>\n", "name", "includes");
  }

  {
    auto *output = _generatorContext->OpenForInsert(
      headerFilename, "namespace_scope");
    auto printer = io::Printer(output, '$');

    for (auto i = 0; i < _file->message_type_count(); ++i)
    {
      const auto *desc = _file->message_type(i);
      std::string ptrTypes;
      std::string descName(desc->name());

      // Define std::unique_ptr types for our messages
      ptrTypes += "typedef std::unique_ptr<"
        + descName + "> "
        + descName + "UniquePtr;\n";

      // Define const std::unique_ptr types for our messages
      ptrTypes += "typedef std::unique_ptr<const "
        + descName + "> Const"
        + descName + "UniquePtr;\n";

      // Define std::shared_ptr types for our messages
      ptrTypes += "typedef std::shared_ptr<"
        + descName + "> "
        + descName + "SharedPtr;\n";

      // Define const std::shared_ptr types for our messages
      ptrTypes += "typedef std::shared_ptr<const "
        + descName + "> Const"
        + descName + "SharedPtr;\n";

      printer.PrintRaw(ptrTypes.c_str());
    }
  }
  return true;
}
}  // namespace google::protobuf::compiler::cpp
