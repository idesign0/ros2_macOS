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

#include "gz/common/CSVStreams.hh"

#include <sstream>

namespace gz
{
namespace common
{

/////////////////////////////////////////////////
const CSVDialect CSVDialect::Unix = {',', '\n', '"'};

/////////////////////////////////////////////////
bool operator==(const CSVDialect &_lhs, const CSVDialect &_rhs)
{
  return (_lhs.delimiter == _rhs.delimiter &&
          _lhs.terminator == _rhs.terminator &&
          _lhs.quote == _rhs.quote);
}

/////////////////////////////////////////////////
std::istream &ExtractCSVToken(
    std::istream &_stream, CSVToken &_token,
    const CSVDialect &_dialect)
{
  char character;
  if (_stream.peek(), !_stream.fail() && _stream.eof())
  {
    _token = {CSVToken::TERMINATOR, static_cast<char>(EOF)};
  }
  else if (_stream.get(character))
  {
    if (character == _dialect.terminator)
    {
      _token = {CSVToken::TERMINATOR, character};
    }
    else if (character == _dialect.delimiter)
    {
      _token = {CSVToken::DELIMITER, character};
    }
    else if (character == _dialect.quote)
    {
      if (_stream.peek() == _dialect.quote)
      {
        _token = {CSVToken::TEXT, character};
        _stream.ignore();
      }
      else
      {
        _token = {CSVToken::QUOTE, character};
      }
    }
    else
    {
      _token = {CSVToken::TEXT, character};
    }
  }
  return _stream;
}

/////////////////////////////////////////////////
std::istream &ParseCSVRow(
    std::istream &_stream,
    std::vector<std::string> &_row,
    const CSVDialect &_dialect)
{
  std::stringstream text;
  enum {
    FIELD_START = 0,
    ESCAPED_FIELD,
    NONESCAPED_FIELD,
    FIELD_END,
    RECORD_END
  } state = FIELD_START;

  _row.clear();

  CSVToken token;
  while (state != RECORD_END && ExtractCSVToken(_stream, token, _dialect))
  {
    switch (state)
    {
      case FIELD_START:
        if (token.type == CSVToken::QUOTE)
        {
          state = ESCAPED_FIELD;
          break;
        }
        state = NONESCAPED_FIELD;
        [[fallthrough]];
      case NONESCAPED_FIELD:
        if (token.type == CSVToken::TEXT)
        {
          text << token.character;
          break;
        }
        state = FIELD_END;
        [[fallthrough]];
      case FIELD_END:
        switch (token.type)
        {
          case CSVToken::DELIMITER:
            _row.push_back(text.str());
            state = FIELD_START;
            break;
          case CSVToken::TERMINATOR:
            if (token.character != static_cast<char>(EOF) || !_row.empty() ||
                text.tellp() > 0)
            {
              _row.push_back(text.str());
              state = RECORD_END;
              break;
            }
            [[fallthrough]];
          default:
            _stream.setstate(std::istream::failbit);
            break;
        }
        text.str(""), text.clear();
        break;
      case ESCAPED_FIELD:
        if (token.type == CSVToken::QUOTE)
        {
          state = FIELD_END;
          break;
        }
        if (token.type != CSVToken::TERMINATOR ||
            token.character != static_cast<char>(EOF))
        {
          text << token.character;
          break;
        }
        [[fallthrough]];
      default:
        _stream.setstate(std::istream::failbit);
        break;
    }
  }
  return _stream;
}

/// \brief Private data for the CSVIStreamIterator class
class CSVIStreamIterator::Implementation
{
  /// \brief Default constructor for end iterator.
  public: Implementation() = default;

  /// \brief Constructor for begin iterator.
  public: Implementation(std::istream &_stream, const CSVDialect &_dialect)
    : stream(&_stream), dialect(_dialect)
  {
  }

  /// \brief Copy constructor.
  public: Implementation(const Implementation &_other)
    : stream(_other.stream), dialect(_other.dialect), row(_other.row)
  {
  }

  /// \brief Copy assignment.
  public: Implementation& operator=(Implementation other)
  {
    if (this != &other)
    {
      std::swap(this->stream, other.stream);
      std::swap(this->dialect, other.dialect);
      std::swap(this->row, other.row);
    }
    return *this;
  }
  /// \brief Advance iterator to next row if possible.
  public: void Next()
  {
    if (this->stream != nullptr)
    {
      try
      {
        if (!ParseCSVRow(*this->stream, this->row, this->dialect))
        {
          this->stream = nullptr;
        }
      }
      catch (...)
      {
        this->stream = nullptr;
        throw;
      }
    }
  }

  /// \brief CSV data stream to iterate, if any.
  public: std::istream *stream{nullptr};

  /// \brief CSV dialect for data parsing.
  public: CSVDialect dialect{};

  /// \brief Current CSV data row.
  public: std::vector<std::string> row;
};

/////////////////////////////////////////////////
CSVIStreamIterator::CSVIStreamIterator()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
}

/////////////////////////////////////////////////
CSVIStreamIterator::CSVIStreamIterator(std::istream &_stream,
                                       const CSVDialect &_dialect)
  : dataPtr(gz::utils::MakeImpl<Implementation>(_stream, _dialect))
{
  this->dataPtr->Next();
}

/////////////////////////////////////////////////
bool CSVIStreamIterator::operator==(const CSVIStreamIterator &_other) const
{
  return this->dataPtr->stream == _other.dataPtr->stream && (
      this->dataPtr->stream == nullptr ||
      this->dataPtr->dialect == _other.dataPtr->dialect);
}

/////////////////////////////////////////////////
bool CSVIStreamIterator::operator!=(const CSVIStreamIterator &_other) const
{
  return !(*this == _other);
}

/////////////////////////////////////////////////
CSVIStreamIterator &CSVIStreamIterator::operator++()
{
  this->dataPtr->Next();
  return *this;
}

/////////////////////////////////////////////////
// NOLINTNEXTLINE(readability/casting)
CSVIStreamIterator CSVIStreamIterator::operator++(int)
{
  CSVIStreamIterator it(*this);
  this->dataPtr->Next();
  return it;
}

/////////////////////////////////////////////////
const std::vector<std::string> &CSVIStreamIterator::operator*() const
{
  return this->dataPtr->row;
}

/////////////////////////////////////////////////
const std::vector<std::string> *CSVIStreamIterator::operator->() const
{
  return &this->dataPtr->row;
}

}
}
