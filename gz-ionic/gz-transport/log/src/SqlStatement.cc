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

#include <cstdint>
#include <string>

#include <gz/transport/log/SqlStatement.hh>

using namespace gz::transport::log;

//////////////////////////////////////////////////
class SqlParameter::Implementation
{
  public: Implementation()
    : type(ParamType::NULL_TYPE)
  {
    // Do nothing
  }

  /// \internal \sa SqlParameter::Set(std::nullptr_t)
  public: void Set(std::nullptr_t)
  {
    this->type = ParamType::NULL_TYPE;
  }

  /// \internal \sa SqlParameter::Set(int64_t)
  public: void Set(const int64_t _integer)
  {
    this->type = ParamType::INTEGER;
    this->integer = _integer;
  }

  /// \internal \sa SqlParameter::Set(double)
  public: void Set(const double _real)
  {
    this->type = ParamType::REAL;
    this->real = _real;
  }

  /// \internal \sa SqlParameter::Set(const std::string &)
  public: void Set(const std::string &_text)
  {
    this->type = ParamType::TEXT;
    this->text = _text;
  }

  /// \internal \sa SqlParameter::QueryInteger()
  public: const int64_t *QueryInteger() const
  {
    if (ParamType::INTEGER == this->type)
      return &this->integer;

    return nullptr;
  }

  /// \internal \sa SqlParameter::QueryReal()
  public: const double *QueryReal() const
  {
    if (ParamType::REAL == this->type)
      return &this->real;

    return nullptr;
  }

  /// \internal \sa SqlParameter::QueryText()
  public: const std::string *QueryText() const
  {
    if (ParamType::TEXT == this->type)
      return &this->text;

    return nullptr;
  }

  /// \brief Type of value that this parameter contains
  public: ParamType type;

  // We store the parameter as a union to save space
  public: union
  {
    /// \brief Integer value for this parameter
    int64_t integer;

    /// \brief Real (floating-point) value for this parameter
    double real;
  };

  /// \brief Text value for this parameter. This cannot be part of the union
  /// because it has a non-trivial destructor.
  public: std::string text;
};

//////////////////////////////////////////////////
SqlParameter::SqlParameter(const SqlParameter &_other)
  : dataPtr(new Implementation)
{
  *this->dataPtr = *_other.dataPtr;
}

//////////////////////////////////////////////////
SqlParameter::SqlParameter(SqlParameter &&_other)  // NOLINT(build/c++11)
  : dataPtr(std::move(_other.dataPtr))
{
  // Do nothing
}

//////////////////////////////////////////////////
SqlParameter &SqlParameter::operator=(const SqlParameter &_other)
{
  *this->dataPtr = *_other.dataPtr;
  return *this;
}

//////////////////////////////////////////////////
SqlParameter &SqlParameter::operator=(SqlParameter &&_other) // NOLINT
{
  this->dataPtr = std::move(_other.dataPtr);
  return *this;
}

//////////////////////////////////////////////////
SqlParameter::SqlParameter()
  : SqlParameter(nullptr)
{
  // Do nothing
}

//////////////////////////////////////////////////
SqlParameter::SqlParameter(std::nullptr_t)
  : dataPtr(new Implementation)
{
  this->dataPtr->Set(nullptr);
}

//////////////////////////////////////////////////
SqlParameter::SqlParameter(const int64_t _integer)
  : dataPtr(new Implementation)
{
  this->dataPtr->Set(_integer);
}

//////////////////////////////////////////////////
SqlParameter::SqlParameter(const double _real)
  : dataPtr(new Implementation)
{
  this->dataPtr->Set(_real);
}

//////////////////////////////////////////////////
SqlParameter::SqlParameter(const std::string &_text)
  : dataPtr(new Implementation)
{
  this->dataPtr->Set(_text);
}

//////////////////////////////////////////////////
void SqlParameter::Set(std::nullptr_t)
{
  this->dataPtr->Set(nullptr);
}

//////////////////////////////////////////////////
void SqlParameter::Set(const int64_t _integer)
{
  this->dataPtr->Set(_integer);
}

//////////////////////////////////////////////////
void SqlParameter::Set(const double _real)
{
  this->dataPtr->Set(_real);
}

//////////////////////////////////////////////////
void SqlParameter::Set(const std::string &_text)
{
  this->dataPtr->Set(_text);
}

//////////////////////////////////////////////////
SqlParameter::ParamType SqlParameter::Type() const
{
  return this->dataPtr->type;
}

//////////////////////////////////////////////////
const int64_t *SqlParameter::QueryInteger() const
{
  return this->dataPtr->QueryInteger();
}

//////////////////////////////////////////////////
const double *SqlParameter::QueryReal() const
{
  return this->dataPtr->QueryReal();
}

//////////////////////////////////////////////////
const std::string *SqlParameter::QueryText() const
{
  return this->dataPtr->QueryText();
}

//////////////////////////////////////////////////
SqlParameter::~SqlParameter()
{
  // Destroy pimpl
}

//////////////////////////////////////////////////
void SqlStatement::Append(const SqlStatement &_other)
{
  this->statement += _other.statement;
  for (const SqlParameter &p : _other.parameters)
    this->parameters.push_back(p);
}
