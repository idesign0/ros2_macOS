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

#include <chrono>

#include <gz/transport/log/QualifiedTime.hh>

using namespace gz::transport::log;

//////////////////////////////////////////////////
/// \internal Implementation for QualifiedTime
class gz::transport::log::QualifiedTime::Implementation
{
  /// \internal \sa QualifiedTime(const std::chrono::nanoseconds&, Qualifier)
  public: Implementation(const Time &_time,
                         Qualifier _qualifier)
    : indeterminate(false),
      qualifier(_qualifier),
      time(_time)
  {
    // Do nothing
  }

  /// \internal \sa QualifiedTime()
  public: Implementation()
    : indeterminate(true)
  {
    // Do nothing
  }

  /// \internal \sa QualifiedTime::IsIndeterminate()
  public: bool IsIndeterminate() const
  {
    return this->indeterminate;
  }

  /// \internal \sa QualifiedTime::GetTime()
  public: const std::chrono::nanoseconds *GetTime() const
  {
    if (this->indeterminate)
      return nullptr;

    return &this->time;
  }

  /// \internal \sa QualifiedTime::GetQualifier()
  public: const Qualifier *GetQualifier() const
  {
    if (this->indeterminate)
      return nullptr;

    return &this->qualifier;
  }

  /// \internal \sa QualifiedTime::SetTime()
  public: void SetTime(const std::chrono::nanoseconds &_time,
                       Qualifier _qualifier)
  {
    this->indeterminate = false;
    this->time = _time;
    this->qualifier = _qualifier;
  }

  /// \internal \sa QualifiedTime::operator==()
  public: bool operator==(const Implementation &_other) const
  {
    // We mimic the behavior of NaN equality comparisons, which always return
    // false during an equality comparison.
    if (_other.IsIndeterminate() || this->IsIndeterminate())
      return false;

    return this->qualifier == _other.qualifier && this->time == _other.time;
  }

  /// \internal \sa QualifiedTime::operator!=()
  public: bool operator!=(const Implementation &_other) const
  {
    return !(_other == *this);
  }

  /// \internal \sa QualifiedTime::Clear()
  public: void Clear()
  {
    this->indeterminate = true;
  }

  /// \internal Flag to keep track of whether this QualifiedTime is
  /// indeterminate
  private: bool indeterminate;

  /// \internal Qualifier for the QualifiedTime, if it is not indeterminate
  private: Qualifier qualifier;

  /// \internal Time value for the QualifiedTime, if it is not indeterminate
  private: std::chrono::nanoseconds time;
};

//////////////////////////////////////////////////
QualifiedTime::QualifiedTime(const Time &_time,
                             Qualifier _qualifier)
  : dataPtr(new Implementation(_time, _qualifier),
      [](Implementation *_impl) { delete _impl; })
{
  // Do nothing
}

//////////////////////////////////////////////////
QualifiedTime::QualifiedTime()
  : dataPtr(new Implementation,
      [](Implementation *_impl) { delete _impl; })
{
  // Do nothing
}

//////////////////////////////////////////////////
QualifiedTime::QualifiedTime(const QualifiedTime &_other)
  : dataPtr(new Implementation(*_other.dataPtr),
      [](Implementation *_impl) { delete _impl; })
{
  // Do nothing
}

//////////////////////////////////////////////////
QualifiedTime & QualifiedTime::operator=(const QualifiedTime &_other)
{
  *this->dataPtr = *_other.dataPtr;
  return *this;
}

//////////////////////////////////////////////////
bool QualifiedTime::operator==(const QualifiedTime &_other) const
{
  return this->dataPtr->operator==(*(_other.dataPtr));
}

//////////////////////////////////////////////////
bool QualifiedTime::operator!=(const QualifiedTime &_other) const
{
  return this->dataPtr->operator!=(*(_other.dataPtr));
}

//////////////////////////////////////////////////
bool QualifiedTime::IsIndeterminate() const
{
  return this->dataPtr->IsIndeterminate();
}

//////////////////////////////////////////////////
const QualifiedTime::Time *QualifiedTime::GetTime() const
{
  return this->dataPtr->GetTime();
}

//////////////////////////////////////////////////
const QualifiedTime::Qualifier *QualifiedTime::GetQualifier() const
{
  return this->dataPtr->GetQualifier();
}

//////////////////////////////////////////////////
void QualifiedTime::SetTime(const Time &_time,
                            Qualifier _qualifier)
{
  this->dataPtr->SetTime(_time, _qualifier);
}

//////////////////////////////////////////////////
void QualifiedTime::Clear()
{
  this->dataPtr->Clear();
}

//////////////////////////////////////////////////
QualifiedTime::~QualifiedTime()
{
  // Destroy pimpl
}

//////////////////////////////////////////////////
/// \brief This function evaluates whether two QualifiedTimes, when used as the
/// same type of endpoint in a range, would be equivalent to each other. This
/// gracefully handles the possibility of the endpoints being indeterminate.
/// \param _t1 Either a start or finish endpoint time
/// \param _t2 A time that is being used in the same context as _t1. If _t1 is
/// being used as a start time, then _t2 must also be a start time. If _t1 is
/// being used as a finish time, then _t2 must also be a finish time.
/// \return Returns true if replacing _t1 with _t2 in a QualifiedTimeRange will
/// result in the same range, otherwise returns false.
static bool EndPointEqual(const QualifiedTime &_t1, const QualifiedTime &_t2)
{
  if (_t1.IsIndeterminate() && _t2.IsIndeterminate())
    return true;

  return (_t1 == _t2);
}

//////////////////////////////////////////////////
class gz::transport::log::QualifiedTimeRange::Implementation
{
  /// \internal \sa QualifiedTimeRange()
  public: Implementation(const QualifiedTime &_begin,
                         const QualifiedTime &_end)
    : start(_begin),
      finish(_end)
  {
    // Do nothing
  }

  /// \internal \sa QualifiedTimeRange::Start()
  const QualifiedTime &Start() const
  {
    return this->start;
  }

  /// \internal \sa QualifiedTimeRange::Finish()
  const QualifiedTime &Finish() const
  {
    return this->finish;
  }

  /// \internal \sa QualifiedTimeRange::SetBeginning()
  bool SetBeginning(const QualifiedTime &_begin)
  {
    this->start = _begin;
    return this->Valid();
  }

  /// \internal \sa QualifiedTimeRange::SetEnding()
  bool SetEnding(const QualifiedTime &_end)
  {
    this->finish = _end;
    return this->Valid();
  }

  /// \internal \sa QualifiedTimeRange::SetRange()
  bool SetRange(const QualifiedTime &_begin,
                const QualifiedTime &_end)
  {
    this->start = _begin;
    this->finish = _end;
    return this->Valid();
  }

  /// \internal \sa QualifiedTimeRange::Valid()
  bool Valid() const
  {
    // If the start is indeterminate, then the range is certainly valid.
    const QualifiedTime::Time *ts = this->start.GetTime();
    if (nullptr == ts)
      return true;

    // If the finish is indeterminate, then the range is certainly valid.
    const QualifiedTime::Time *tf = this->finish.GetTime();
    if (nullptr == tf)
      return true;

    // If the start is less than or equal to the finish, the range is valid.
    return (*ts <= *tf);
  }

  /// \internal \sa QualifiedTimeRange::operator==
  bool operator==(const QualifiedTimeRange::Implementation &_other) const
  {
    return EndPointEqual(this->start, _other.start)
        && EndPointEqual(this->finish, _other.finish);
  }

  /// \brief The time where the range starts
  public: QualifiedTime start;

  /// \brief The time where the range finishes
  public: QualifiedTime finish;
};

//////////////////////////////////////////////////
QualifiedTimeRange::QualifiedTimeRange(
    const QualifiedTime &_begin,
    const QualifiedTime &_end)
  : dataPtr(new Implementation(_begin, _end),
      [](Implementation *_impl) { delete _impl; })
{
  // Do nothing
}

//////////////////////////////////////////////////
QualifiedTimeRange::QualifiedTimeRange(const QualifiedTimeRange &_other)
  : dataPtr(new Implementation(*_other.dataPtr),
      [](Implementation *_impl) { delete _impl; })
{
  // Do nothing
}

//////////////////////////////////////////////////
QualifiedTimeRange::QualifiedTimeRange(QualifiedTimeRange &&_other)  // NOLINT
  : dataPtr(std::move(_other.dataPtr))
{
  // Do nothing
}


//////////////////////////////////////////////////
QualifiedTimeRange &QualifiedTimeRange::operator=(
    const QualifiedTimeRange &_other)
{
  *this->dataPtr = *_other.dataPtr;
  return *this;
}

//////////////////////////////////////////////////
bool QualifiedTimeRange::operator==(const QualifiedTimeRange &_other) const
{
  return (*this->dataPtr) == (*_other.dataPtr);
}

//////////////////////////////////////////////////
bool QualifiedTimeRange::operator!=(const QualifiedTimeRange &_other) const
{
  return !(*this == _other);
}

//////////////////////////////////////////////////
QualifiedTimeRange QualifiedTimeRange::From(const QualifiedTime &_begin)
{
  return QualifiedTimeRange(_begin, QualifiedTime());
}

//////////////////////////////////////////////////
QualifiedTimeRange QualifiedTimeRange::Until(const QualifiedTime &_end)
{
  return QualifiedTimeRange(QualifiedTime(), _end);
}

//////////////////////////////////////////////////
QualifiedTimeRange QualifiedTimeRange::AllTime()
{
  return QualifiedTimeRange(QualifiedTime(), QualifiedTime());
}

//////////////////////////////////////////////////
const QualifiedTime &QualifiedTimeRange::Beginning() const
{
  return this->dataPtr->Start();
}

//////////////////////////////////////////////////
const QualifiedTime &QualifiedTimeRange::Ending() const
{
  return this->dataPtr->Finish();
}

//////////////////////////////////////////////////
bool QualifiedTimeRange::SetBeginning(const QualifiedTime &_begin)
{
  return this->dataPtr->SetBeginning(_begin);
}

//////////////////////////////////////////////////
bool QualifiedTimeRange::SetEnding(const QualifiedTime &_end)
{
  return this->dataPtr->SetEnding(_end);
}

//////////////////////////////////////////////////
bool QualifiedTimeRange::SetRange(const QualifiedTime &_begin,
                                  const QualifiedTime &_end)
{
  return this->dataPtr->SetRange(_begin, _end);
}

//////////////////////////////////////////////////
bool QualifiedTimeRange::Valid() const
{
  return this->dataPtr->Valid();
}

//////////////////////////////////////////////////
QualifiedTimeRange::~QualifiedTimeRange()
{
  // Destroy pimpl
}
