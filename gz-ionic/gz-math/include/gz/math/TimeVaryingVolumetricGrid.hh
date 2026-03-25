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

#ifndef GZ_MATH_TIME_VARYING_VOLUMETRIC_GRID_HH_
#define GZ_MATH_TIME_VARYING_VOLUMETRIC_GRID_HH_

#include <gz/math/TimeVaryingVolumetricGridLookupField.hh>
#include <gz/math/Vector3.hh>

#include <map>
#include <utility>
#include <vector>

namespace gz::math
{
/// \brief A grid with interpolation where time can be varied. This class has
/// no implementation to allow for different strategies.
/// \sa InMemoryTimeVaryingVolumetricGrid for an example
/// specialization that loads all data into memory. To use this class you should
/// use `CreateSession` so you can step through time.
template<typename T, typename V, typename S, typename P>
class TimeVaryingVolumetricGrid
{
  /// \brief Creates a session. Call this before querying the interface.
  public: S CreateSession() const;

  /// \brief Creates a session at a given time.
  /// Call this before querying the interface.
  /// \param[in] _time - The time at which to create the session.
  public: S CreateSession(const T &_time) const;

  /// \brief Returns true if a session is valid. False if invalid,
  public: bool IsValid(const S &_session) const;

  /// \brief Steps the session to a fixed time step
  /// \param[in] _session - The session to be stepped
  /// \param[in] _time - The time at which the session should be set.
  /// \return the session cursor if the step was successful. If there was
  /// no more data return nullopt.
  public: std::optional<S> StepTo(const S &_session, const T &_time) const;

  /// \brief Looks up a value at a given point in time.
  /// \param[in] _session - The session with the time stamp to be looked up.
  /// \param[in] _pos - The position of the point we want to query
  public: std::optional<V> LookUp(const S &_session, const Vector3<P> &_pos)
    const;

  /// \brief Get the spatial bounds of this grid field at given time.
  /// \return A pair of vectors. All zeros if session is invalid.
  public: std::pair<Vector3<V>, Vector3<V>> Bounds(const S &_session) const;
};

/// \brief Specialization of TimeVaryingVolumetricGrid which loads the whole of
/// the dataset into memory. To construct this class use
/// `InMemoryTimeVaryingVolumetricGridFactory`
template<typename T, typename V, typename P>
class TimeVaryingVolumetricGrid<T, V, InMemorySession<T, P>, P>
{
  /// \brief Documentation Inherited
  public: InMemorySession<T, V> CreateSession() const
  {
    return indices.CreateSession();
  }

  /// \brief Documentation Inherited
  public: InMemorySession<T, V> CreateSession(const T &_time) const
  {
    return indices.CreateSession(_time);
  }

  /// \brief Documentation Inherited
  public: bool IsValid(const InMemorySession<T, P> &_session) const
  {
    return indices.IsValid(_session);
  }

  /// \brief Documentation Inherited
  public: std::optional<InMemorySession<T, P>>
    StepTo(const InMemorySession<T, double> &_session, const T &_time) const
  {
    return indices.StepTo(_session, _time);
  }

  /// \brief Looks up a given point. If the point lies in between two time
  /// frames then it performs spatio-temporal linear interpolation.
  /// \return nullopt if the data is out of range.
  public: std::optional<V>
    LookUp(const InMemorySession<T, P> &_session,
      const Vector3<P> &_pos,
      const Vector3<V> &_tol = Vector3<V>{1e-6, 1e-6, 1e-6})
    const
  {
    auto points = indices.LookUp(_session, _pos, _tol);
    std::optional<V> result = indices.EstimateQuadrilinear(
      _session,
      points,
      values,
      values,
      _pos);
    return result;
  }

  /// \brief Get the bounds of this grid field at given time.
  /// \return A pair of vectors. All zeros if session is invalid.
  public: std::pair<Vector3<V>, Vector3<V>> Bounds(
    const InMemorySession<T, P> &_session) const
  {
    return indices.Bounds(_session);
  }

  /// Buffer for values being stored
  private: std::vector<V> values;

  /// Index table for fast lookup
  private: TimeVaryingVolumetricGridLookupField
    <T, V, InMemorySession<T, P>> indices;

  template<typename U, typename S, typename X>
  friend class InMemoryTimeVaryingVolumetricGridFactory;
};

/// \brief Alias for Specialization of TimeVaryingVolumetricGrid which loads
/// the whole of the dataset into memory.
template<typename T, typename V = T, typename P = T>
using InMemoryTimeVaryingVolumetricGrid =
  TimeVaryingVolumetricGrid<T, V, InMemorySession<T, P>, P>;

/// \brief Factory class for constructing an InMemoryTimeVaryingVolumetricGrid.
template<typename T, typename V, typename P = double>
class InMemoryTimeVaryingVolumetricGridFactory
{
  /// \brief Adds a point at a given time and position.
  /// \param[in] _time - Time
  /// \param[in] _position - Position
  /// \param[in] _value - Value of the point to be added to the field
  public: void AddPoint(
    const T &_time, const Vector3<P> &_position, const V &_value)
  {
    _points[_time].emplace_back(_position, _value);
  }

  /// \brief Builds the `InMemoryTimeVaryingVolumetricGrid<T, V, P>` object.
  public: InMemoryTimeVaryingVolumetricGrid<T, V, P> Build() const
  {
    InMemoryTimeVaryingVolumetricGrid<T, V, P> grid;
    for (auto &[time, pts] : _points)
    {
      std::vector<Vector3d> cloud;
      std::vector<std::size_t> indices;
      for (auto &[pt, val] : pts)
      {
        grid.values.push_back(val);
        cloud.push_back(pt);
        indices.push_back(grid.values.size() - 1);
      }
      VolumetricGridLookupField<V> field(cloud, indices);
      grid.indices.AddVolumetricGridField(time, field);
    }
    return grid;
  }

  /// Temporary datastore
  private: std::map<T, std::vector<std::pair<Vector3d, V>>> _points;
};
}  // namespace gz::math
#endif  // GZ_MATH_TIME_VARYING_VOLUMETRIC_GRID_HH_
