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
#ifndef GZ_TRANSPORT_LOG_RAIISQLITE3_HH_
#define GZ_TRANSPORT_LOG_RAIISQLITE3_HH_

#include <string>

// Forward declarations.
struct sqlite3;
struct sqlite3_stmt;

/// \internal
/// \remarks Not using PIMPL because these classes are for internal use only
namespace raii_sqlite3
{
  /// \brief Thin RAII wrapper for a (sqlite3 *)
  /// Automatically calls sqlite_close()
  /// \internal
  class Database
  {
    /// \brief Constructor
    /// \param[in] _path UTF-8 path to db file to open
    /// \param[in] _flags Flags to use when opening the database
    /// See https://www.sqlite.org/c3ref/open.html for flags
    public: Database(const std::string &_path, int _flags);

    /// \brief Destructor
    public: ~Database();

    /// \brief Handle
    public: sqlite3 *Handle();

    /// \brief Return true if the database is valid.
    operator bool() const;

    /// \brief the pointer this is wrapping
    protected: sqlite3 *handle = nullptr;
  };

  /// \brief Thin RAII wrapper for a (sqlite3_stmt *)
  /// Automatically calls sqlite_finalize()
  /// \internal
  class Statement
  {
    /// \brief Constructor
    /// \param[in] _db The database the statement is being used on
    /// \param[in] _sql A single SQL statement to compile
    public: Statement(Database &_db, const std::string &_sql);

    /// \brief Destructor
    public: ~Statement();

    /// \brief Handle
    public: sqlite3_stmt *Handle();

    /// \brief Return true if the statement is valid is valid.
    operator bool() const;

    /// \brief the pointer this is wrapping
    protected: sqlite3_stmt *handle = nullptr;
  };
}

#endif
