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

/* Note: Use PRAGMA foreign_keys = ON; prior to writing to a database using this schema */

/* Describes the schema version used in this database */
CREATE TABLE migrations (
  /* Uniquely identifies a row in this table. Sqlite3 will make it an alias of rowid. */
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  /* Previous schema version. NULL on the row inserted when the database is created. */
  from_version TEXT DEFAULT NULL,
  /* Version of the schema the database was migrated to. */
  to_version TEXT NOT NULL,
  /* Time when the migration happened (auto populates). */
  time_utc INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP
);

/* Set the initial version to 0.1.0 */
INSERT INTO migrations (to_version) VALUES ('0.1.0');

/* Contains every type of message used by a recorded topic */
CREATE TABLE message_types (
  /* Uniquely identifies a row in this table. Sqlite3 will make it an alias of rowid. */
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  /* Name of the message (e.g. .gz.msgs.LaserScan) */
  name TEXT NOT NULL,
  /* Full text of the protobuf file, or NULL if logging did not have access to it */
  proto_descriptor TEXT
);

/* Contains every topic logged */
CREATE TABLE topics (
  /* Uniquely identifies a row in this table. Sqlite3 will make it an alias of rowid. */
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  /* Name of the topic (e.g. /car/roof/scan) */
  name TEXT NOT NULL,
  /* A message type in the message_types table */
  message_type_id NOT NULL REFERENCES message_types (id) ON DELETE CASCADE
);

/* There is at most 1 row in topics for each name/message_type combo */
CREATE UNIQUE INDEX idx_topic ON topics (name, message_type_id);

/* Contains every message received on every topic recorded */
CREATE TABLE messages (
  /* Uniquely identifies a row in this table. Sqlite3 will make it an alias of rowid. */
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  /* Timestamp the message was received (utc nanoseconds) */
  time_recv INTEGER NOT NULL,
  /* Topic the message was received on */
  topic_id REFERENCES topics (id) ON DELETE CASCADE,
  /* Serialized protobuf message */
  message BLOB NOT NULL
);

/* Lots of queries are done by time received, so add an index to speed it up */
CREATE INDEX idx_time_recv ON messages (time_recv);
