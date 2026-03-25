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

/// \brief Example of creating custom query options for the log file.
/// Custom query options can filter or re-order the output of a query for
/// messages.
/// This example will query all messages in a log, but output them in reverse.

#include <iostream>
#include <vector>
#include <gz/transport/log/Log.hh>
#include <gz/transport/log/QueryOptions.hh>

//////////////////////////////////////////////////
/// \brief Query for all the topics in reverse
class AllTopicsReverse final
    : public virtual gz::transport::log::QueryOptions
{
  // See documentation on QueryOptions
  public: std::vector<gz::transport::log::SqlStatement>
    GenerateStatements(
      const gz::transport::log::Descriptor &/*_descriptor*/)
      const override
  {
    // The preamble has all the necessary joins and the correct column order
    // for a SELECT statement that returns messages.
    gz::transport::log::SqlStatement statement =
      this->StandardMessageQueryPreamble();
    // Reverse the order of the messages
    statement.statement += "ORDER BY messages.time_recv DESC;";
    return {statement};
  }
};

//////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " INPUT.tlog\n";
    return -1;
  }

  gz::transport::log::Log log;
  if (!log.Open(argv[1]))
  {
    std::cerr << "Failed to open log\n";
    return -1;
  }

  // Output all the messages using the custom query
  auto batch = log.QueryMessages(AllTopicsReverse());
  for (const auto &message : batch)
  {
    std::cout << message.TimeReceived().count()
              << ": '" << message.Data() << "'\n";
  }

  return 0;
}
