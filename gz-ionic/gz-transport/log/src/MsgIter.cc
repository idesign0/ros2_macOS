/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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

#include <sqlite3.h>

#include <memory>
#include <vector>

#include "Console.hh"
#include "gz/transport/log/MsgIter.hh"
#include "MsgIterPrivate.hh"
#include "raii-sqlite3.hh"

using namespace gz::transport;
using namespace gz::transport::log;

//////////////////////////////////////////////////
MsgIterPrivate::MsgIterPrivate()
{
}

//////////////////////////////////////////////////
MsgIterPrivate::MsgIterPrivate(
    const std::shared_ptr<raii_sqlite3::Database> &_db,
    const std::shared_ptr<std::vector<SqlStatement>> &_statements)
  : db(_db), statements(_statements)
{
  PrepareNextStatement();
}

//////////////////////////////////////////////////
MsgIterPrivate::~MsgIterPrivate()
{
}

//////////////////////////////////////////////////
bool MsgIterPrivate::PrepareNextStatement()
{
  if (this->statements != nullptr
      && this->statementIndex >= this->statements->size())
  {
    // No more statements
    return false;
  }
  // Get next statement in list
  const SqlStatement & query = this->statements->at(this->statementIndex);

  // Compile the statement
  std::unique_ptr<raii_sqlite3::Statement> nextStatement(
      new raii_sqlite3::Statement(*(this->db), query.statement));
  if (!*nextStatement)
  {
    LERR("Failed to prepare query: "<< sqlite3_errmsg(
        this->db->Handle()) << "\n");
    return false;
  }

  // Bind the parameters supplied with the statement
  int i = 1;
  int returnCode;
  for (const SqlParameter &param : query.parameters)
  {
    switch (param.Type())
    {
      case SqlParameter::ParamType::TEXT:
        returnCode = sqlite3_bind_text(nextStatement->Handle(), i,
          param.QueryText()->c_str(), param.QueryText()->size(),
          SQLITE_STATIC);
        break;
      case SqlParameter::ParamType::INTEGER:
        returnCode = sqlite3_bind_int64(nextStatement->Handle(), i,
          *param.QueryInteger());
        break;
      case SqlParameter::ParamType::REAL:
        returnCode = sqlite3_bind_double(nextStatement->Handle(), i,
          *param.QueryReal());
        break;
      default:
        return false;
    }
    if (returnCode != SQLITE_OK)
    {
      LERR("Failed to query messages: "<< sqlite3_errmsg(
        this->db->Handle()) << "\n");
      return false;
    }
    ++i;
  }

  this->statement = std::move(nextStatement);
  return true;
}

//////////////////////////////////////////////////
void MsgIterPrivate::StepStatement()
{
  if (this->statement)
  {
    // Get the results from the statement
    int returnCode = sqlite3_step(this->statement->Handle());

    if (returnCode == SQLITE_ROW)
    {
      // TODO(anyone) get data and create message in the dereference operators
      // Assumes statement has column order:
      // messages id (0), timeRecv(1), topics name(2),
      // message_type name(3), message data(4)
      std::chrono::nanoseconds timeRecv;

      // Time received
      sqlite_int64 timeRecvInt = sqlite3_column_int64(
          this->statement->Handle(), 1);
      timeRecv = std::chrono::nanoseconds(timeRecvInt);

      // Topic name
      const unsigned char *topic = sqlite3_column_text(
          this->statement->Handle(), 2);
      std::size_t numTopic = sqlite3_column_bytes(
          this->statement->Handle(), 2);

      // Message type name
      const unsigned char *type = sqlite3_column_text(
          this->statement->Handle(), 3);
      std::size_t numType = sqlite3_column_bytes(this->statement->Handle(), 3);

      // Message data
      const void *data = sqlite3_column_blob(this->statement->Handle(), 4);
      std::size_t numData = sqlite3_column_bytes(this->statement->Handle(), 4);

      this->message.reset(new Message(
            timeRecv,
            data, numData,
            reinterpret_cast<const char*>(type), numType,
            reinterpret_cast<const char*>(topic), numTopic));
    }
    else
    {
      if (returnCode != SQLITE_DONE)
      {
        LERR("Failed to get message [" << returnCode << "]\n");
      }
      // Out of data
      this->statement.reset();
      ++this->statementIndex;
      this->PrepareNextStatement();
    }
  }
}

//////////////////////////////////////////////////
MsgIter::MsgIter()
  : dataPtr(new MsgIterPrivate)
{
}

//////////////////////////////////////////////////
// TODO(anyone)
// MsgIter::MsgIter(const MsgIter &_orig)
//   : dataPtr(new MsgIterPrivate)
// {
//   // TODO(anyone) this copy constructor needs to create a new statement
//   // starting
//   // at the current row id
// }

MsgIter::MsgIter(MsgIter &&_orig)  // NOLINT(build/c++11)
  : dataPtr(std::move(_orig.dataPtr))
{
}

//////////////////////////////////////////////////
MsgIter::MsgIter(
    std::unique_ptr<MsgIterPrivate> &&_pimpl)  // NOLINT(build/c++11)
  : dataPtr(std::move(_pimpl))
{
  // Execute statement so iter points to first result
  this->dataPtr->StepStatement();
}

//////////////////////////////////////////////////
MsgIter::~MsgIter()
{
}

//////////////////////////////////////////////////
MsgIter &MsgIter::operator=(MsgIter &&_other) // NOLINT
{
  if (this != &_other)
  {
    this->dataPtr = std::move(_other.dataPtr);
  }
  return *this;
}

//////////////////////////////////////////////////
MsgIter &MsgIter::operator++()
{
  this->dataPtr->StepStatement();
  return *this;
}

//////////////////////////////////////////////////
// TODO(anyone)
// MsgIter MsgIter::operator++(int)
// {
// }

//////////////////////////////////////////////////
// TODO(anyone)
bool MsgIter::operator==(const MsgIter &_other) const
{
  // TODO(anyone) this won't work once this class has a proper copy constructor
  // It's only good enough to compare this with an empty iterator
  return this->dataPtr->statement.get() == _other.dataPtr->statement.get();
}

//////////////////////////////////////////////////
bool MsgIter::operator!=(const MsgIter &_other) const
{
  return !this->operator==(_other);
}

//////////////////////////////////////////////////
const Message &MsgIter::operator*() const
{
  return *this->dataPtr->message;
}

//////////////////////////////////////////////////
const Message *MsgIter::operator->() const
{
  return this->dataPtr->message.get();
}
