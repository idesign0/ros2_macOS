/*
 * Copyright (C) 2024 Open Source Robotics Foundation
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
#ifndef GZ_MATH_COORDINATE_VECTOR3_HH_
#define GZ_MATH_COORDINATE_VECTOR3_HH_

#include <optional>
#include <ostream>

#include <gz/math/Angle.hh>
#include <gz/math/Vector3.hh>
#include <gz/math/Helpers.hh>
#include <gz/math/config.hh>
#include <gz/utils/ImplPtr.hh>

namespace gz::math
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_MATH_VERSION_NAMESPACE {

  /// \class CoordinateVector3 CoordinateVector3.hh gz/math/CoordinateVector3.hh
  /// \brief The CoordinateVector3 class represents the vector containing 3
  /// coordinates, either metric or spherical.
  class GZ_MATH_VISIBLE CoordinateVector3
  {
    /// \brief Construct an empty metric vector.
    public: CoordinateVector3();

    public: ~CoordinateVector3();

    /// \brief Copy constructor
    /// \param[in] _other The copied value.
    public: CoordinateVector3(const CoordinateVector3 &_other);

    public: CoordinateVector3(CoordinateVector3 &&_other) noexcept;

    /// \brief Constructor for metric values.
    /// \param[in] _x value along x
    /// \param[in] _y value along y
    /// \param[in] _z value along z
    /// \return The coordinate vector.
    public: static CoordinateVector3 Metric(double _x, double _y, double _z);

    /// \brief Constructor for metric values.
    /// \param[in] _v The metric vector.
    /// \return The coordinate vector.
    public: static CoordinateVector3 Metric(const math::Vector3d &_v);

    /// \brief Constructor for spherical values.
    /// \param[in] _lat latitude value
    /// \param[in] _lon longitude value
    /// \param[in] _z value along z
    /// \return The coordinate vector.
    public: static CoordinateVector3 Spherical(
        const math::Angle &_lat, const math::Angle &_lon, double _z);

    /// \brief Whether this vector is metric.
    /// \return Whether this vector is metric.
    public: bool IsMetric() const;

    /// \brief Whether this vector is spherical.
    /// \return Whether this vector is spherical.
    public: bool IsSpherical() const;

    /// \brief Set the metric contents of the vector
    /// \param[in] _x value along x
    /// \param[in] _y value along y
    /// \param[in] _z value along z
    public: void SetMetric(double _x, double _y, double _z);

    /// \brief Set the metric contents of the vector
    /// \param[in] _v The metric vector.
    public: void SetMetric(const math::Vector3d &_v);

    /// \brief Set the spherical contents of the vector
    /// \param[in] _lat latitude value
    /// \param[in] _lon longitude value
    /// \param[in] _z value along z
    public: void SetSpherical(
      const math::Angle &_lat, const math::Angle &_lon, double _z);

    /// \brief Return this vector as a metric Vector3d (only valid for metric).
    /// \return The metric vector (or nullopt if the vector is not metric).
    public: std::optional<math::Vector3d> AsMetricVector() const;

    /// \brief Copy assignment
    /// \param[in] _other The copied value.
    /// \return Reference to this.
    public: CoordinateVector3 &operator=(const CoordinateVector3 &_other);

    /// \brief Move assignment
    /// \param[in] _other The copied value.
    /// \return Reference to this.
    public: CoordinateVector3 &operator=(CoordinateVector3 &&_other) noexcept;

    /// \brief Addition operator
    /// \param[in] _v vector to add
    /// \return the sum vector
    /// \note If one vector is metric and the other is spherical, a NaN
    ///       vector will be returned and a message logged to cerr.
    public: CoordinateVector3 operator+(const CoordinateVector3 &_v) const;

    /// \brief Addition assignment operator
    /// \param[in] _v vector to add
    /// \return the sum vector
    /// \note If one vector is metric and the other is spherical, a NaN
    ///       vector will be set and a message logged to cerr.
    public: const CoordinateVector3 &operator+=(const CoordinateVector3 &_v);

    /// \brief Negation operator
    /// \return negative of this vector
    public: CoordinateVector3 operator-() const;

    /// \brief Subtraction operators
    /// \param[in] _pt a vector to subtract
    /// \return a vector after the subtraction
    /// \note If one vector is metric and the other is spherical, a NaN
    ///       vector will be returned and a message logged to cerr.
    public: CoordinateVector3 operator-(const CoordinateVector3 &_pt) const;

    /// \brief Subtraction assignment operators
    /// \param[in] _pt subtrahend
    /// \return a vector after the subtraction
    /// \note If one vector is metric and the other is spherical, a NaN
    ///       vector will be set and a message logged to cerr.
    public: const CoordinateVector3 &operator-=(const CoordinateVector3 &_pt);

    /// \brief Equality test with tolerance.
    /// \param[in] _v the vector to compare to
    /// \param[in] _tol equality tolerance for metric components.
    /// \param[in] _ang_tol equality tolerance for spherical components.
    /// \return true if the vectors are equal within the tolerance specified by
    ///         _tol and _ang_tol.
    public: bool Equal(const CoordinateVector3 &_v,
                       const double &_tol,
                       const math::Angle &_ang_tol) const;

    /// \brief Equal to operator
    /// \param[in] _v The vector to compare against
    /// \return true if the vectors are equal within a default tolerance (1e-3),
    ///         false otherwise
    public: bool operator==(const CoordinateVector3 &_v) const;

    /// \brief Not equal to operator
    /// \param[in] _v The vector to compare against
    /// \return false if the vectors are not equal within a default tolerance
    ///         (1e-3), true otherwise
    public: bool operator!=(const CoordinateVector3 &_v) const;

    /// \brief See if all vector components are finite (e.g., not nan)
    /// \return true if is finite or false otherwise
    public: bool IsFinite() const;

    /// \brief Equality test
    /// \remarks This is equivalent to the == operator
    /// \param[in] _v the other vector
    /// \return true if the 2 vectors have the same values, false otherwise
    public: bool Equal(const CoordinateVector3 &_v) const;

    /// \brief Get the x value of a metric vector.
    /// \return The x component of the metric vector (or nullopt if spherical).
    public: std::optional<double> X() const;

    /// \brief Get the latitude of a spherical vector.
    /// \return The latitude of the spherical vector (or nullopt if metric).
    public: std::optional<math::Angle> Lat() const;

    /// \brief Get the y value of a metric vector.
    /// \return The y component of the metric vector (or nullopt if spherical).
    public: std::optional<double> Y() const;

    /// \brief Get the longitude of a spherical vector.
    /// \return The longitude of the spherical vector (or nullopt if metric).
    public: std::optional<math::Angle> Lon() const;

    /// \brief Get the z value.
    /// \return The z component of the vector (nullopt is never returned).
    public: std::optional<double> Z() const;

    /// \brief Set the x value.
    /// \param[in] _v Value for the x component.
    /// \return True if the vector is metric, false otherwise.
    public: bool X(const double &_v);

    /// \brief Set the latitude.
    /// \param[in] _v Value for the latitude.
    /// \return True if the vector is spherical, false otherwise.
    public: bool Lat(const Angle &_v);

    /// \brief Set the y value.
    /// \param[in] _v Value for the y component.
    /// \return True if the vector is metric, false otherwise.
    public: bool Y(const double &_v);

    /// \brief Set the longitude.
    /// \param[in] _v Value for the longitude.
    /// \return True if the vector is spherical, false otherwise.
    public: bool Lon(const Angle &_v);

    /// \brief Set the z value.
    /// \param[in] _v Value for the z component.
    /// \return Always true.
    public: bool Z(const double &_v);

    /// \brief Stream insertion operator
    /// \param _out output stream
    /// \param _pt CoordinateVector3 to output
    /// \return the stream
    public: friend std::ostream &operator<<(
        std::ostream &_out, const gz::math::CoordinateVector3 &_pt)
    {
      if (_pt.IsMetric())
      {
        appendToStream(_out, *_pt.X());
      }
      else
      {
        appendToStream(_out, _pt.Lat()->Degree());
        _out << "°";
      }
      _out << " ";

      if (_pt.IsMetric())
      {
        appendToStream(_out, *_pt.Y());
      }
      else
      {
        appendToStream(_out, _pt.Lon()->Degree());
        _out << "°";
      }
      _out << " ";

      appendToStream(_out, *_pt.Z());

      return _out;
    }

    GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
  };

  }  // namespace GZ_MATH_VERSION_NAMESPACE
}  // namespace gz::math
#endif  // GZ_MATH_COORDINATE_VECTOR3_HH_
