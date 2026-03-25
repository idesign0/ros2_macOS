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

#include <sqlite3.h>

#include <string>

#include "Console.hh"
#include "raii-sqlite3.hh"

using namespace raii_sqlite3;

//////////////////////////////////////////////////
Database::Database(const std::string &_path, int _flags)
{
  // Open the database;
  int return_code = sqlite3_open_v2(
      _path.c_str(), &(this->handle), _flags, nullptr);

  if (return_code != SQLITE_OK)
  {
    LERR("Failed to open the requested sqlite3 database\n");
    sqlite3_close(this->handle);
    this->handle = nullptr;
    return;
  }

  // Turn on extended error codes
  return_code = sqlite3_extended_result_codes(this->handle, 1);
  if (return_code != SQLITE_OK)
  {
    LERR("Failed to turn on extended result codes"
        << sqlite3_errmsg(this->handle) << "\n");
    sqlite3_close(this->handle);
    this->handle = nullptr;
    return;
  }

  // Turn on foreign key support
  const char *sql = "PRAGMA foreign_keys = ON;";
  return_code = sqlite3_exec(this->handle, sql, NULL, 0, NULL);
  if (return_code != SQLITE_OK)
  {
    LERR("Failed to turn on foreign_key support"
        << sqlite3_errmsg(this->handle) << "\n");
    sqlite3_close(this->handle);
    this->handle = nullptr;
    return;
  }
}

//////////////////////////////////////////////////
Database::~Database()
{
  if (this->handle)
  {
    sqlite3_close(this->handle);
    this->handle = nullptr;
  }
}

//////////////////////////////////////////////////
sqlite3 *Database::Handle()
{
  return this->handle;
}

//////////////////////////////////////////////////
Database::operator bool() const
{
  return this->handle != nullptr;
}

//////////////////////////////////////////////////
Statement::Statement(Database &_db, const std::string &_sql)
{
  int return_code = sqlite3_prepare_v2(
      _db.Handle(), _sql.c_str(), _sql.size(), &(this->handle), NULL);

  if (return_code != SQLITE_OK)
  {
    if (this->handle)
    {
      LERR("Failed to prepare statement"
          << sqlite3_errmsg(_db.Handle()) << "\n");
      sqlite3_finalize(this->handle);
      this->handle = nullptr;
    }
    return;
  }
}

//////////////////////////////////////////////////
Statement::~Statement()
{
  if (this->handle)
  {
    sqlite3_finalize(this->handle);
    this->handle = nullptr;
  }
}

//////////////////////////////////////////////////
sqlite3_stmt *Statement::Handle()
{
  return this->handle;
}

//////////////////////////////////////////////////
Statement::operator bool() const
{
  return this->handle != nullptr;
}
