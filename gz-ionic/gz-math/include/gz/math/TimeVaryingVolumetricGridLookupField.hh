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

#ifndef GZ_MATH_TIME_VARYING_VOLUMETRIC_GRID_LOOKUP_FIELD_HH_
#define GZ_MATH_TIME_VARYING_VOLUMETRIC_GRID_LOOKUP_FIELD_HH_
#include <gz/math/VolumetricGridLookupField.hh>
#include <gz/math/detail/InterpolationPoint.hh>

#include <map>
#include <optional>
#include <utility>
#include <vector>

namespace gz::math
{
  /// \brief Lookup table for a time-varying volumetric dataset.
  /// This is an unimplemented template as the actual methods depend on the
  /// underlying structure. The key idea is that one uses a session `S` to
  /// hold a session token. This is so that we don't keep doing O(logn)
  /// lookups and instead step along the axis.
  template<typename T, typename V, typename S>
  class TimeVaryingVolumetricGridLookupField
  {
    /// \brief Constructor
    public: TimeVaryingVolumetricGridLookupField();

    /// \brief Adds a volumetric grid field.
    public: void AddVolumetricGridField(
      const T& time, const VolumetricGridLookupField<V> &_field);

    /// \brief Creates a session for querying
    public: S CreateSession() const;

    /// \brief Creates a session startingg at time T
    /// \param[in] _time The time to create the session at.
    public: S CreateSession(const T &_time) const;

    /// \brief Update session to new time. Returns new session pointer if
    /// more is available. Otherwise returns nullopt if we have run out of
    /// time.
    /// \param[in] _session - The session
    /// \param[in] _time - Time to step to.
    public: std::optional<S> StepTo(const S &_session, const T &_time);

    /// \brief Looks up interpolators at a given time step. Use the session
    /// to set the time step.
    /// \param[in] _session - The session
    /// \param[in] _point - The point to query
    /// \param[in] _tol - Tolerance
    public: std::vector<InterpolationPoint4D<T, V>>
    LookUp(const S &_session,
      const Vector3<V> &_point,
      const Vector3<V> &_tol) const;

    /// \brief Uses quadrilinear interpolation to estimate value of current
    /// point. Returns nullopt if query is out of range.
    /// \param[in] _session - The session
    /// \param[in] _points - The interpolation points retrieved from
    ///  `LookUp()`.
    /// \param[in] _values1 - Value array at timestep 1.
    /// \param[in] _values2 - Value array at timestep 2.
    /// \param[in] _default - Value used if there is a hole in the data.
    /// \returns The estimated value for the point. Nullopt if we are
    /// outside the field. Default value if in the field but no value is
    /// in the index.
    public: template<typename X>
    std::optional<X> EstimateQuadrilinear(
      const S &_session,
      const std::vector<InterpolationPoint4D<T, V>> &_points,
      const std::vector<X> &_values1,
      const std::vector<X> &_values2,
      const X _default = X(0)
    ) const;

    /// \brief Get the bounds of this grid field.
    /// \return A pair of vectors.
    public: std::pair<Vector3<V>, Vector3<V>> Bounds(const S &_session);
  };

  /// \brief An in-memory session. Loads the whole dataset in memory and
  /// performs queries.
  template<typename T, typename V>
  class InMemorySession
  {
    /// \brief Iterator which holds pointer to current state
    /// TODO(arjo): Use friend to make visible only to InMemorySession
    /// specialization
    private:
      typename std::map<T, VolumetricGridLookupField<V>>::const_iterator iter;

    /// \brief Time of last query
    public: T time;

    friend class
      TimeVaryingVolumetricGridLookupField<T, V, InMemorySession<T, V>>;
  };

  /// \brief Specialized version of `TimeVaryingVolumetricGridLookupField`
  /// for in-memory lookup. It loads the whole dataset into memory.
  template<typename T, typename V>
  class TimeVaryingVolumetricGridLookupField<T, V, InMemorySession<T, V>>
  {
    /// \brief Default constructor
    public: TimeVaryingVolumetricGridLookupField()
    {}

    /// \brief Documentation inherited
    public: void AddVolumetricGridField(
      const T &_time, const VolumetricGridLookupField<V> &_field) {
      this->gridFields.emplace(_time, _field);
    }

    /// \brief Documentation inherited
    public: InMemorySession<T, V> CreateSession() const {
      InMemorySession<T, V> sess;
      sess.iter = this->gridFields.begin();
      sess.time = T(0);
      return sess;
    }

    /// \brief Documentation inherited
    public: InMemorySession<T, V> CreateSession(const T &_time) const {
      InMemorySession<T, V> sess;
      sess.iter = this->gridFields.lower_bound(_time);
      sess.time = _time;
      return sess;
    }

    /// \brief In memory session
    public: bool IsValid(const InMemorySession<T, V> &_session) const {
      return this->gridFields.end() != _session.iter;
    }

    /// \brief Documentation inherited
    public: std::optional<InMemorySession<T, V>> StepTo(
      const InMemorySession<T, V> &_session, const T &_time) const {
      if (_session.iter == gridFields.end())
      {
        return std::nullopt;
      }

      InMemorySession<T, V> newSess(_session);

      auto nextTime = std::next(_session.iter);
      if (nextTime == this->gridFields.end() || _time < _session.iter->first)
      {
        return std::nullopt;
      }

      while (nextTime != this->gridFields.end()
        && nextTime->first <= _time)
      {
        newSess.iter = nextTime;
        nextTime = std::next(nextTime);
      }
      newSess.time = _time;
      return newSess;
    }

    /// \brief Documentation inherited
    public: std::vector<InterpolationPoint4D<T, V>>
      LookUp(const InMemorySession<T, V> &_session,
        const Vector3<V> &_point,
        const Vector3<V> &_tol = Vector3<V>{1e-6, 1e-6, 1e-6}) const {

      std::vector<InterpolationPoint4D<T, V>> res;

      if (_session.iter == this->gridFields.end())
      {
        // Out of bounds
        return res;
      }

      InterpolationPoint4D<T, V> slice1;
      slice1.timeSlice = _session.iter->second.GetInterpolators(
          _point, _tol.X(), _tol.Y(), _tol.Z());
      slice1.time = _session.iter->first;
      res.push_back(slice1);

      auto nextTime = std::next(_session.iter);
      if (nextTime != this->gridFields.end())
      {
        // Only add next item if field exists
        InterpolationPoint4D<T, V> slice2;
        slice2.timeSlice = nextTime->second.GetInterpolators(
          _point, _tol.X(), _tol.Y(), _tol.Z());
        slice2.time = nextTime->first;
        res.push_back(slice2);
      }
      return res;
    }

    /// \brief Uses quadrilinear interpolation to estimate value of current
    /// point. Returns nullopt if query is out of range.
    /// \param[in] _session - The session
    /// \param[in] _interpolators - The interpolation points
    /// retrieved from `LookUp()`
    /// \param[in] _values1 - Value array at timestep 1.
    /// \param[in] _values2 - Value array at timestep 2.
    /// \param[in] _position - The position to be queried.
    /// \param[in] _default - Value used if there is a hole in the data.
    /// \returns The estimated value for the point. Nullopt if we are
    /// outside the field. Default value if in the field but no value is
    /// in the index.
    public: template<typename X>
    std::optional<X> EstimateQuadrilinear(
      const InMemorySession<T, V> &_session,
      const std::vector<InterpolationPoint4D<T, V>> &_interpolators,
      const std::vector<X> &_values1,
      const std::vector<X> &_values2,
      const Vector3<X> &_position,
      const X _default = X(0)
    ) const
    {
      if (_session.iter == this->gridFields.end())
      {
        // Out of bounds
        return std::nullopt;
      }

      auto time = _session.time;
      if (_interpolators.size() == 0) return std::nullopt;
      if (_interpolators.size() == 1)
      {
        // This happens we reach the end of time
        return _session.iter->second.EstimateValueUsingTrilinear(
          _position,
          _values2,
          _default);
      }

      /// Got nothing to interpolate. Out of bounds.
      if (_interpolators[0].timeSlice.size() == 0
       && _interpolators[1].timeSlice.size() == 0)
        return std::nullopt;

      /// Only one of the two time-slices has data. Use that slice to guess.
      auto next = std::next(_session.iter);
      if (_interpolators[1].timeSlice.size() == 0)
      {
        return _session.iter->second.EstimateValueUsingTrilinear(
          _position,
          _values1,
          _default
        );
      }
      if (_interpolators[0].timeSlice.size() == 0)
      {
        return next->second.EstimateValueUsingTrilinear(
          _position,
          _values2,
          _default
        );
      }

      /// Default case where both time-slices has interpolation
      auto res1 = _session.iter->second.EstimateValueUsingTrilinear(
        _position,
        _values1,
        _default
      );

      auto res2 = next->second.EstimateValueUsingTrilinear(
        _position,
        _values2,
        _default
      );

      if (res1.has_value() || res2.has_value())
      {
        InterpolationPoint1D<T>
          pt1{_session.iter->first, 0}, pt2{next->first, 1};
        // If either has value interpolate using default value.
        std::vector<X> times{
          res1.value_or(_default), res2.value_or(_default)};
        return LinearInterpolate(pt1, pt2, times, time);
      }
      // Return nullopt if we are out of range
      return std::nullopt;
    }

    /// \brief Get the bounds of this grid field.
    /// \return A pair of vectors. All zeros if session is invalid.
    public: std::pair<Vector3<V>, Vector3<V>> Bounds(
      const InMemorySession<T, V> &_session) const
    {
      if (_session.iter == this->gridFields.end())
      {
        // Out of bounds
        return std::pair<Vector3<T>, Vector3<T>>(
          Vector3<T>{0, 0, 0}, Vector3<T>{0, 0, 0}
        );
      }
      return _session.iter->second.Bounds();
    }

    private: std::map<T, VolumetricGridLookupField<V>> gridFields;
  };
}  // namespace gz::math
#endif  // GZ_MATH_TIME_VARYING_VOLUMETRIC_GRID_LOOKUP_FIELD_HH_
