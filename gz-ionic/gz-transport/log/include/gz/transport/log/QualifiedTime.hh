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
#ifndef GZ_TRANSPORT_LOG_QUALIFIEDTIME_HH_
#define GZ_TRANSPORT_LOG_QUALIFIEDTIME_HH_

#include <chrono>
#include <cstdint>
#include <memory>

#include <gz/transport/config.hh>
#include <gz/transport/log/Export.hh>

namespace gz::transport::log
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_TRANSPORT_VERSION_NAMESPACE {
  //
  //////////////////////////////////////////////////
  /// \brief Since time is continuous, it may be difficult to know the exact
  /// time stamp of a desired message. The QualifiedTime class provides a
  /// way to tailor how a time stamp is interpreted by the message query.
  ///
  /// Note that the value of this time object may be interpreted as a
  /// relative time or as an absolute time stamp depending on the context in
  /// which it gets used.
  class GZ_TRANSPORT_LOG_VISIBLE QualifiedTime
  {
    /// \brief The Qualifier determines the behavior of how a message is
    /// selected.
    public: enum class Qualifier : int64_t
    {
      /// \brief This point in time should be seen as a closed (inclusive)
      /// time marker. When used in a QualifiedTimeRange, the range will
      /// include this exact point in time.
      INCLUSIVE = 0,

      /// \brief This point in time should be seen as an open (exclusive)
      /// time marker. When used in a QualifiedTimeRange, the range will
      /// NOT include this exact point in time, but it will include
      /// everything leading up to it.
      EXCLUSIVE
    };

    /// \brief Convenient typedef for our time representation. Note that any
    /// std::chrono::duration type can be implicitly converted to this type.
    /// E.g. you can pass a
    ///
    /// \code
    /// std::chrono::seconds(value)
    /// std::chrono::milliseconds(value)
    /// std::chrono::minutes(value)
    /// // ... etc ...
    /// \endcode
    ///
    /// into any function that accepts this Time type.
    using Time = std::chrono::nanoseconds;

    /// \brief Construct a qualified time specifier.
    /// \param[in] _time The time stamp that is used as the focal point of
    /// this qualified time.
    /// \param[in] _qualifier The qualifier that determines the exact
    /// interpretation of the _time value.
    public: QualifiedTime(const Time &_time,
                          Qualifier _qualifier = Qualifier::INCLUSIVE);

    /// \brief Construct a qualified time specifier. This allows implicit
    /// conversion from any std::chrono::duration type.
    /// \param[in] _time The time stamp that is used as the focal point of
    /// this qualified time.
    /// \param[in] _qualifier The qualifier that determines the exact
    /// interpretation of the _time value.
    public: template <typename Rep, typename Period>
    QualifiedTime(const std::chrono::duration<Rep, Period> &_time,
                  Qualifier _qualifier = Qualifier::INCLUSIVE)
      : QualifiedTime(static_cast<const Time &>(_time), _qualifier) { }

    /// \brief Default constructor. The time will be treated as
    /// indeterminate. This means that the QualifiedTime object will be
    /// taken to indicate that no time has been specified at all.
    /// \sa IsIndeterminate()
    public: QualifiedTime(/* Indeterminate */);

    /// \brief Copy constructor.
    /// \param[in] _other Another QualifiedTime
    public: QualifiedTime(const QualifiedTime &_other);

    /// \brief Copy assignment operator.
    /// \param[in] _other Another QualifiedTime
    /// \return Reference to this object
    public: QualifiedTime &operator=(const QualifiedTime &_other);

    /// \brief move constructor
    /// \param[in] _old the instance being moved into this one
    public: QualifiedTime(QualifiedTime &&_old) = default;  // NOLINT

    /// \brief move assignment operator
    /// \return Reference to this object
    public: QualifiedTime &operator=(
        QualifiedTime &&) = default;  // NOLINT(build/c++11)

    /// \brief Equality operator.
    /// \param[in] _other Another QualifiedTime
    /// \return True if the times are equal. When either time is
    /// indeterminate, this will always return false, similar to the
    /// behavior of NaN comparisons.
    public: bool operator==(const QualifiedTime &_other) const;

    /// \brief Inequality operator.
    /// \param[in] _other Another QualifiedTime
    /// \return Opposite value of operator==(const QualifiedTime &)
    public: bool operator!=(const QualifiedTime &_other) const;

    /// \brief Indicates whether this QualifiedTime object is indeterminate.
    ///
    /// When an indeterminate time is used as the end of a range, it implies
    /// that the range should go on endlessly. When used as the beginning of
    /// a range, it means that the range should extend as far into the past
    /// as possible.
    ///
    /// \return true if this QualifiedTime is indeterminate, or false if it
    /// does have a determined time.
    public: bool IsIndeterminate() const;

    /// \brief Get the time stamp for this qualified time, unless the time
    /// is indeterminate.
    /// \return A pointer to the time value that is specified by this
    /// QualifiedTime object, or a nullptr if this QualifiedTime is
    /// indeterminate.
    public: const Time *GetTime() const;

    /// \brief Get the qualifier for this qualified time, unless the time
    /// is indeterminate.
    /// \return A pointer to the qualifier that is specified by this
    /// QualifiedTime object, or a nullptr if this QualifiedTime is
    /// indeterminate.
    public: const Qualifier *GetQualifier() const;

    /// \brief Set the time that this QualifiedTime object represents.
    /// \param[in] _time The time stamp that is used as the focal point of
    /// this qualified time.
    /// \param[in] _qualifier The qualifier that determines the exact
    /// interpretation of the _time value.
    public: void SetTime(const Time &_time,
                         Qualifier _qualifier = Qualifier::INCLUSIVE);

    /// \brief Set this QualifiedTime object to be indeterminate.
    /// \sa IsIndeterminate()
    public: void Clear();

    /// \brief Destructor
    public: ~QualifiedTime();

    /// \internal Implementation class
    private: class Implementation;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal PIMPL pointer
    /// We need a custom deleter here due to a compiler bug in MSVC 2017
    private: std::unique_ptr<Implementation, void (*)(Implementation*)> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };

  //////////////////////////////////////////////////
  /// \brief The QualifiedTimeRange class provides a pair of qualified times
  /// that represent a range. This is used to specify a desired time range
  /// to the BasicQueryOptions class.
  class GZ_TRANSPORT_LOG_VISIBLE QualifiedTimeRange
  {
    /// \brief Construct a time range.
    /// \param[in] _begin The beginning of the time range.
    /// \param[in] _end The end of the time range.
    /// \sa From()
    /// \sa Until()
    /// \sa AllTime()
    public: QualifiedTimeRange(
      const QualifiedTime &_begin,
      const QualifiedTime &_end);

    /// \brief Copy constructor
    /// \param[in] _other Another QualifiedTimeRange
    public: QualifiedTimeRange(const QualifiedTimeRange &_other);

    /// \brief Copy assignment operator
    /// \param[in] _other Another QualifiedTimeRange
    /// \return Reference to this object
    public: QualifiedTimeRange &operator=(const QualifiedTimeRange &_other);

    /// \brief Default move constructor
    /// \param[in] _old the instance being moved into this one
    public: QualifiedTimeRange(QualifiedTimeRange &&_old);  // NOLINT

    /// \brief Default move assignment
    /// \return Reference to this object
    public: QualifiedTimeRange &operator=(
        QualifiedTimeRange &&) = default;  // NOLINT(build/c++11)

    /// \brief Equality operator.
    /// \param[in] _other Another QualifiedTimeRange
    /// \return true if the time ranges represented by each object are
    /// equivalent
    public: bool operator==(const QualifiedTimeRange &_other) const;

    /// \brief Inequality operator.
    /// \param[in] _other Another QualifiedTimeRange
    /// \return Opposite of operator==(const QualifiedTimeRange &)
    public: bool operator!=(const QualifiedTimeRange &_other) const;

    /// \brief Construct a time range that begins at _begin and never ends.
    /// \param[in] _begin The beginning of the time range.
    /// \return A time range that begins at _begin and never ends.
    public: static QualifiedTimeRange From(const QualifiedTime &_begin);

    /// \brief Construct a time range that ends at _end and has no
    /// beginning.
    /// \param[in] _end The end of the time range.
    /// \return A time range that ends at _end and has no beginning.
    public: static QualifiedTimeRange Until(const QualifiedTime &_end);

    /// \brief Construct a time range that has no beginning or end.
    /// \return A time range that has no beginning or end.
    public: static QualifiedTimeRange AllTime();

    /// \brief Get a reference to the start time of this range.
    /// \return A reference to the start time.
    public: const QualifiedTime &Beginning() const;

    /// \brief Get a reference to the end time of this range.
    /// \return A reference to the end time.
    public: const QualifiedTime &Ending() const;

    /// \brief Set the start time of this range. Passing in an indeterminate
    /// QualifiedTime (its default constructor) will tell this range to have
    /// no beginning (effectively, it should start at the beginning of the
    /// log).
    /// \param[in] _begin The start time of this range.
    /// \return True if the new range is valid. False if the range is now
    /// invalid. The _begin value will be accepted either way.
    /// \sa SetRange()
    /// \sa Valid()
    public: bool SetBeginning(const QualifiedTime &_begin);

    /// \brief Set the finish time of this range. Passing in an
    /// indeterminate QualifiedTime (its default constructor) will tell this
    /// range to have no end (effectively, it should not stop until the end
    /// of the log).
    /// \param[in] _end The finish time of this range.
    /// \return True if this new range is valid. False if the range is now
    /// invalid. The _end value will be accepted either way.
    /// \sa SetRange()
    /// \sa Valid()
    public: bool SetEnding(const QualifiedTime &_end);

    /// \brief Set both endpoints of the range.
    /// \param[in] _begin The start time of this range.
    /// \param[in] _end The end time of this range.
    /// \return True if this new range is valid. False if the range is now
    /// invalid. The values for _begin and _end will be accepted either
    /// way.
    /// \sa SetBeginning()
    /// \sa SetEnding()
    /// \sa Valid()
    public: bool SetRange(const QualifiedTime &_begin,
                          const QualifiedTime &_end);

    /// \brief Check if the range is valid. A valid range means that the
    /// finish time is guaranteed to be later than or coincident with the
    /// start time.
    /// \return True if the range is valid, false if invalid.
    public: bool Valid() const;

    /// \brief Virtual destructor
    public: ~QualifiedTimeRange();

    /// \internal Implementation class
    private: class Implementation;

#ifdef _WIN32
// Disable warning C4251 which is triggered by
// std::*
#pragma warning(push)
#pragma warning(disable: 4251)
#endif
    /// \internal PIMPL pointer
    /// We need a custom deleter here due to a compiler bug in MSVC 2017
    private: std::unique_ptr<Implementation, void(*)(Implementation*)> dataPtr;
#ifdef _WIN32
#pragma warning(pop)
#endif
  };
  }
}

#endif
