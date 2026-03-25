/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#include <filesystem>

#include "gz/common/Filesystem.hh"

namespace fs = std::filesystem;

namespace gz
{
namespace common
{

class DirIter::Implementation
{
  /// \brief Filesystem iterator that this class is wrapping
  public: fs::directory_iterator it;
};

//////////////////////////////////////////////////
DirIter::DirIter():
  dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->it = fs::directory_iterator();
}

//////////////////////////////////////////////////
DirIter::DirIter(const std::string &_in):
  DirIter()
{
  try
  {
    this->dataPtr->it = fs::directory_iterator(_in);
  }
  catch (const fs::filesystem_error &ex)
  {
  }
}

//////////////////////////////////////////////////
std::string DirIter::operator*() const
{
  return this->dataPtr->it->path().string();
}

//////////////////////////////////////////////////
const DirIter &DirIter::operator++()
{
  this->dataPtr->it++;
  return *this;
}

//////////////////////////////////////////////////
bool DirIter::operator!=(const DirIter &_other) const
{
  return (this->dataPtr->it != _other.dataPtr->it);
}

}  // namespace common
}  // namespace gz
