/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <vector>

#include "gz/transport/log/Batch.hh"
#include "BatchPrivate.hh"
#include "build_config.hh"
#include "MsgIterPrivate.hh"
#include "raii-sqlite3.hh"

using namespace gz::transport;
using namespace gz::transport::log;

//////////////////////////////////////////////////
BatchPrivate::BatchPrivate(const std::shared_ptr<raii_sqlite3::Database> &_db,
      std::vector<SqlStatement> &&_statements)  // NOLINT(build/c++11)
  : statements(new std::vector<SqlStatement>(std::move(_statements))), db(_db)
{
}

//////////////////////////////////////////////////
BatchPrivate::~BatchPrivate()
{
}

//////////////////////////////////////////////////
Batch::Batch()
  : dataPtr(nullptr)
{
}

//////////////////////////////////////////////////
Batch::Batch(Batch &&_old)  // NOLINT
  : dataPtr(std::move(_old.dataPtr))
{
}

//////////////////////////////////////////////////
Batch &Batch::operator=(Batch &&_other) // NOLINT
{
  if (this != &_other)
  {
    this->dataPtr = std::move(_other.dataPtr);
  }
  return *this;
}

//////////////////////////////////////////////////
Batch::~Batch()
{
}

//////////////////////////////////////////////////
Batch::iterator Batch::begin()
{
  if (!this->dataPtr)
  {
    return Batch::iterator();
  }

  std::unique_ptr<MsgIterPrivate> msgPriv(new MsgIterPrivate(
        this->dataPtr->db, this->dataPtr->statements));
  return Batch::iterator(std::move(msgPriv));
}

//////////////////////////////////////////////////
Batch::iterator Batch::end()
{
  return Batch::iterator();
}

//////////////////////////////////////////////////
Batch::Batch(std::unique_ptr<BatchPrivate> &&_pimpl)  // NOLINT(build/c++11)
  : dataPtr(std::move(_pimpl))
{
}
