/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

#include "gz/common/CSVStreams.hh"

using namespace gz;
using namespace common;

/////////////////////////////////////////////////
TEST(CSVStreams, CanExtractCSVTokens)
{
  std::stringstream sstream;
  sstream << "\"a,\n\"\"";

  CSVToken token;
  EXPECT_TRUE(ExtractCSVToken(sstream, token, CSVDialect::Unix));
  EXPECT_EQ(token.type, CSVToken::QUOTE);
  EXPECT_EQ(token.character, '"');

  EXPECT_TRUE(ExtractCSVToken(sstream, token, CSVDialect::Unix));
  EXPECT_EQ(token.type, CSVToken::TEXT);
  EXPECT_EQ(token.character, 'a');

  EXPECT_TRUE(ExtractCSVToken(sstream, token, CSVDialect::Unix));
  EXPECT_EQ(token.type, CSVToken::DELIMITER);
  EXPECT_EQ(token.character, ',');

  EXPECT_TRUE(ExtractCSVToken(sstream, token, CSVDialect::Unix));
  EXPECT_EQ(token.type, CSVToken::TERMINATOR);
  EXPECT_EQ(token.character, '\n');

  EXPECT_TRUE(ExtractCSVToken(sstream, token, CSVDialect::Unix));
  EXPECT_EQ(token.type, CSVToken::TEXT);
  EXPECT_EQ(token.character, '"');

  EXPECT_TRUE(ExtractCSVToken(sstream, token, CSVDialect::Unix));
  EXPECT_EQ(token.type, CSVToken::TERMINATOR);
  EXPECT_EQ(token.character, static_cast<char>(EOF));
}

/////////////////////////////////////////////////
TEST(CSVStreams, CanParseCSVRows)
{
  {
    std::stringstream sstream;
    sstream << ",";
    std::vector<std::string> row;
    EXPECT_TRUE(ParseCSVRow(sstream, row, CSVDialect::Unix));
    const std::vector<std::string> expectedRow{"", ""};
    EXPECT_EQ(row, expectedRow);
  }

  {
    std::stringstream sstream;
    sstream << "foo";
    std::vector<std::string> row;
    EXPECT_TRUE(ParseCSVRow(sstream, row, CSVDialect::Unix));
    const std::vector<std::string> expectedRow{"foo"};
    EXPECT_EQ(row, expectedRow);
  }

  {
    std::stringstream sstream;
    sstream << "foo" << std::endl;
    std::vector<std::string> row;
    EXPECT_TRUE(ParseCSVRow(sstream, row, CSVDialect::Unix));
    const std::vector<std::string> expectedRow{"foo"};
    EXPECT_EQ(row, expectedRow);
  }

  {
    std::stringstream sstream;
    sstream << ",foo";
    std::vector<std::string> row;
    EXPECT_TRUE(ParseCSVRow(sstream, row, CSVDialect::Unix));
    const std::vector<std::string> expectedRow{"", "foo"};
    EXPECT_EQ(row, expectedRow);
  }

  {
    std::stringstream sstream;
    sstream << ",\"foo,bar\nbaz\",";
    std::vector<std::string> row;
    EXPECT_TRUE(ParseCSVRow(sstream, row, CSVDialect::Unix));
    const std::vector<std::string> expectedRow{"", "foo,bar\nbaz", ""};
    EXPECT_EQ(row, expectedRow);
  }
}

/////////////////////////////////////////////////
TEST(CSVStreams, CanHandleInvalidCSVRows)
{
  {
    std::stringstream sstream;
    std::vector<std::string> row;
    EXPECT_FALSE(ParseCSVRow(sstream, row, CSVDialect::Unix));
  }

  {
    std::stringstream sstream;
    sstream << "\"";
    std::vector<std::string> row;
    EXPECT_FALSE(ParseCSVRow(sstream, row, CSVDialect::Unix));
  }

  {
    std::stringstream sstream;
    sstream << "\"foo\"?";
    std::vector<std::string> row;
    EXPECT_FALSE(ParseCSVRow(sstream, row, CSVDialect::Unix));
  }

  {
    std::stringstream sstream;
    sstream << "foo\"bar\"";
    std::vector<std::string> row;
    EXPECT_FALSE(ParseCSVRow(sstream, row, CSVDialect::Unix));
  }
}

/////////////////////////////////////////////////
TEST(CSVStreams, CanIterateValidCSV)
{
  {
    std::stringstream sstream;
    EXPECT_EQ(CSVIStreamIterator(sstream),
              CSVIStreamIterator());
  }

  {
    std::stringstream ss;
    ss << std::endl;
    const std::vector<std::vector<std::string>> expectedRows{{""}};
    const auto rows = std::vector<std::vector<std::string>>(
        CSVIStreamIterator(ss), CSVIStreamIterator());
    EXPECT_EQ(expectedRows, rows);
    EXPECT_TRUE(ss.eof());
  }

  {
    std::stringstream ss;
    ss << "foo,bar" << std::endl
       << "bar," << std::endl
       << ",foo" << std::endl
       << "," << std::endl
       << "baz,baz";
    const std::vector<std::vector<std::string>> expectedRows{
      {"foo", "bar"}, {"bar", ""}, {"", "foo"}, {"", ""}, {"baz", "baz"}};
    const auto rows = std::vector<std::vector<std::string>>(
        CSVIStreamIterator(ss), CSVIStreamIterator());
    EXPECT_EQ(expectedRows, rows);
  }

  {
    std::stringstream ss;
    ss << "foo,bar" << std::endl
       << "bar," << std::endl
       << ",foo" << std::endl
       << "," << std::endl
       << "baz,baz";
    auto it = CSVIStreamIterator(ss);
    // Copy constructor for coverage
    auto it_copy = it;

    auto row0 = *it_copy;
    EXPECT_EQ(row0[0], "foo");
    EXPECT_EQ(row0[1], "bar");
    it_copy++;

    auto row1 = *it_copy;
    EXPECT_EQ(row1[0], "bar");
    EXPECT_EQ(row1[1], "");
  }
}

/////////////////////////////////////////////////
TEST(CSVStreams, CanIterateInvalidCSVSafely)
{
  {
    std::stringstream sstream;
    sstream << "\"" << std::endl;
    auto it = CSVIStreamIterator(sstream, CSVDialect::Unix);
    EXPECT_EQ(it, CSVIStreamIterator());
  }

  {
    std::stringstream sstream;
    sstream.exceptions(std::stringstream::failbit);
    sstream << "foo" << std::endl
            << "\"bar" << std::endl;
    auto it = CSVIStreamIterator(sstream, CSVDialect::Unix);
    EXPECT_THROW({ ++it; }, std::stringstream::failure);
    EXPECT_EQ(it, CSVIStreamIterator());
  }
}
