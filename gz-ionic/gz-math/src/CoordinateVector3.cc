/*
 * Copyright (C) 2024 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License")
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

#include "gz/math/Angle.hh"
#include "gz/math/CoordinateVector3.hh"
#include "gz/math/Vector3.hh"
#include "gz/math/Helpers.hh"
#include "gz/math/detail/Error.hh"
#include "gz/utils/ImplPtr.hh"

#include <cmath>
#include <optional>
#include <variant>

/// \brief Private data for the CoordinateVector3 class.
class gz::math::CoordinateVector3::Implementation
{
  /// \brief The metric x coordinate or spherical latitude.
  public: std::variant<double, math::Angle> x_or_lat;
  /// \brief The metric y coordinate or spherical longitude.
  public: std::variant<double, math::Angle> y_or_lon;
  /// \brief The z coordinate (always metric).
  public: double z;

  /// \brief Raw access to the stored metric X value without type checking.
  public: double X() const
  {
    return std::get<double>(this->x_or_lat);
  }

  /// \brief Raw access to the stored metric X value without type checking.
  public: double& X()
  {
    return std::get<double>(this->x_or_lat);
  }

  /// \brief Raw access to the stored spherical Lat value without type checking.
  public: const math::Angle& Lat() const
  {
    return std::get<math::Angle>(this->x_or_lat);
  }

  /// \brief Raw access to the stored spherical Lat value without type checking.
  public: math::Angle& Lat()
  {
    return std::get<math::Angle>(this->x_or_lat);
  }

  /// \brief Raw access to the stored metric Y value without type checking.
  public: double Y() const
  {
    return std::get<double>(this->y_or_lon);
  }

  /// \brief Raw access to the stored metric Y value without type checking.
  public: double& Y()
  {
    return std::get<double>(this->y_or_lon);
  }

  /// \brief Raw access to the stored spherical Lon value without type checking.
  public: const math::Angle& Lon() const
  {
    return std::get<math::Angle>(this->y_or_lon);
  }

  /// \brief Raw access to the stored spherical Lon value without type checking.
  public: math::Angle& Lon()
  {
    return std::get<math::Angle>(this->y_or_lon);
  }

  /// \brief Raw access to the stored Z value without type checking.
  public: double Z() const
  {
    return this->z;
  }

  /// \brief Raw access to the stored Z value without type checking.
  public: double& Z()
  {
    return this->z;
  }

};

using namespace gz;
using namespace math;

/////////////////////////////////////////////////
CoordinateVector3::CoordinateVector3() :
  dataPtr(gz::utils::MakeUniqueImpl<Implementation>(0.0, 0.0, 0.0))
{
}

/////////////////////////////////////////////////
CoordinateVector3::~CoordinateVector3() = default;

/////////////////////////////////////////////////
CoordinateVector3::CoordinateVector3(const CoordinateVector3 &_other) :
  dataPtr(gz::utils::MakeUniqueImpl<Implementation>())
{
  *this->dataPtr = *_other.dataPtr;
}

/////////////////////////////////////////////////
CoordinateVector3::CoordinateVector3(CoordinateVector3 &&_other) noexcept :
  dataPtr(std::exchange(_other.dataPtr, nullptr))
{
}

/////////////////////////////////////////////////
CoordinateVector3& CoordinateVector3::operator=(const CoordinateVector3 &_other)
{
  // If the object was moved from, dataPtr will be null
  if (!this->dataPtr)
    this->dataPtr = gz::utils::MakeUniqueImpl<Implementation>();

  *this->dataPtr = *_other.dataPtr;
  return *this;
}

/////////////////////////////////////////////////
CoordinateVector3&
CoordinateVector3::operator=(CoordinateVector3 &&_other) noexcept
{
  std::swap(this->dataPtr, _other.dataPtr);
  return *this;
}

/////////////////////////////////////////////////
CoordinateVector3 CoordinateVector3::Metric(
    const double _x, const double _y, const double _z)
{
  CoordinateVector3 res;
  res.SetMetric(_x, _y, _z);
  return res;
}

/////////////////////////////////////////////////
CoordinateVector3 CoordinateVector3::Metric(const Vector3d &_v)
{
  return Metric(_v.X(), _v.Y(), _v.Z());
}

/////////////////////////////////////////////////
CoordinateVector3 CoordinateVector3::Spherical(
    const Angle &_lat, const Angle &_lon, const double _z)
{
  CoordinateVector3 res;
  res.SetSpherical(_lat, _lon, _z);
  return res;
}

/////////////////////////////////////////////////
bool CoordinateVector3::IsMetric() const
{
  return std::holds_alternative<double>(this->dataPtr->x_or_lat);
}

/////////////////////////////////////////////////
bool CoordinateVector3::IsSpherical() const
{
  return std::holds_alternative<math::Angle>(this->dataPtr->x_or_lat);
}

/////////////////////////////////////////////////
void CoordinateVector3::SetMetric(
  const double _x, const double _y, const double _z)
{
  this->dataPtr->x_or_lat = _x;
  this->dataPtr->y_or_lon = _y;
  this->dataPtr->z = _z;
}

/////////////////////////////////////////////////
void CoordinateVector3::SetMetric(const Vector3d &_v)
{
  this->SetMetric(_v.X(), _v.Y(), _v.Z());
}

/////////////////////////////////////////////////
void CoordinateVector3::SetSpherical(
  const Angle &_lat, const Angle &_lon, double _z)
{
  this->dataPtr->x_or_lat = _lat;
  this->dataPtr->y_or_lon = _lon;
  this->dataPtr->z = _z;
}

/////////////////////////////////////////////////
std::optional<math::Vector3d> CoordinateVector3::AsMetricVector() const
{
  if (!this->IsMetric())
    return std::nullopt;

  return math::Vector3d{
    this->dataPtr->X(), this->dataPtr->Y(), this->dataPtr->Z()};
}

/////////////////////////////////////////////////
CoordinateVector3 CoordinateVector3::operator+(
  const CoordinateVector3 &_v) const
{
  if (this->IsMetric() != _v.IsMetric())
  {
    if (this->IsMetric())
    {
      detail::LogErrorMessage(
          "Spherical coordinates cannot be added to metric. "
          "Returning NaN.");
      return Metric(NAN_D, NAN_D, NAN_D);
    }
    else
    {
      detail::LogErrorMessage(
          "Metric coordinates cannot be added to spherical. "
          "Returning NaN.");
      return Spherical(NAN_D, NAN_D, NAN_D);
    }
  }

  if (this->IsMetric())
  {
    return CoordinateVector3::Metric(
      this->dataPtr->X() + _v.dataPtr->X(),
      this->dataPtr->Y() + _v.dataPtr->Y(),
      this->dataPtr->Z() + _v.dataPtr->Z());
  }
  else
  {
    return CoordinateVector3::Spherical(
      this->dataPtr->Lat() + _v.dataPtr->Lat(),
      this->dataPtr->Lon() + _v.dataPtr->Lon(),
      this->dataPtr->Z() + _v.dataPtr->Z());
  }
}

/////////////////////////////////////////////////
const CoordinateVector3& CoordinateVector3::operator+=(
  const CoordinateVector3 &_v)
{
  if (this->IsMetric() != _v.IsMetric())
  {
    this->dataPtr->Z() = NAN_D;
    if (this->IsMetric())
    {
      this->dataPtr->X() = this->dataPtr->Y() = NAN_D;
      detail::LogErrorMessage(
          "Spherical coordinates cannot be added to metric. "
          "Setting the result to NaN.");
    }
    else
    {
      this->dataPtr->Lat() = this->dataPtr->Lon() = NAN_D;
      detail::LogErrorMessage(
          "Metric coordinates cannot be added to spherical. "
          "Setting the result to NaN.");
    }
    return *this;
  }

  if (this->IsMetric())
  {
    this->dataPtr->X() += _v.dataPtr->X();
    this->dataPtr->Y() += _v.dataPtr->Y();
  }
  else
  {
    this->dataPtr->Lat() += _v.dataPtr->Lat();
    this->dataPtr->Lon() += _v.dataPtr->Lon();
  }
  this->dataPtr->Z() += _v.dataPtr->Z();

  return *this;
}

/////////////////////////////////////////////////
CoordinateVector3 CoordinateVector3::operator-() const
{
  if (this->IsMetric())
  {
    return Metric(
      -this->dataPtr->X(), -this->dataPtr->Y(), -this->dataPtr->Z());
  }
  else
  {
    return Spherical(
      -this->dataPtr->Lat(), -this->dataPtr->Lon(), -this->dataPtr->Z());
  }
}

/////////////////////////////////////////////////
CoordinateVector3 CoordinateVector3::operator-(
  const CoordinateVector3 &_pt) const
{
  if (this->IsMetric() != _pt.IsMetric())
  {
    if (this->IsMetric())
    {
      detail::LogErrorMessage(
          "Spherical coordinates cannot be subtracted from metric. "
          "Returning NaN.");
      return Metric(NAN_D, NAN_D, NAN_D);
    }
    else
    {
      detail::LogErrorMessage(
          "Metric coordinates cannot be subtracted from spherical. "
          "Returning NaN.");
      return Spherical(NAN_D, NAN_D, NAN_D);
    }
  }

  if (this->IsMetric())
  {
    return Metric(
      this->dataPtr->X() - _pt.dataPtr->X(),
      this->dataPtr->Y() - _pt.dataPtr->Y(),
      this->dataPtr->Z() - _pt.dataPtr->Z());
  }
  else
  {
    return Spherical(
      this->dataPtr->Lat() - _pt.dataPtr->Lat(),
      this->dataPtr->Lon() - _pt.dataPtr->Lon(),
      this->dataPtr->Z() - _pt.dataPtr->Z());
  }
}

/////////////////////////////////////////////////
const CoordinateVector3& CoordinateVector3::operator-=(
  const CoordinateVector3 &_pt)
{
  if (this->IsMetric() != _pt.IsMetric())
  {
    this->dataPtr->Z() = NAN_D;
    if (this->IsMetric())
    {
      this->dataPtr->X() = this->dataPtr->Y() = NAN_D;
      detail::LogErrorMessage(
          "Spherical coordinates cannot be subtracted from metric. "
          "Setting the result to NaN.");
    }
    else
    {
      this->dataPtr->Lat() = this->dataPtr->Lon() = NAN_D;
      detail::LogErrorMessage(
          "Metric coordinates cannot be subtracted from spherical. "
          "Setting the result to NaN.");
    }
    return *this;
  }

  if (this->IsMetric())
  {
    this->dataPtr->X() -= _pt.dataPtr->X();
    this->dataPtr->Y() -= _pt.dataPtr->Y();
  }
  else
  {
    this->dataPtr->Lat() -= _pt.dataPtr->Lat();
    this->dataPtr->Lon() -= _pt.dataPtr->Lon();
  }
  this->dataPtr->Z() -= _pt.dataPtr->Z();

  return *this;
}

/////////////////////////////////////////////////
bool CoordinateVector3::Equal(
  const CoordinateVector3 &_v, const double &_tol, const Angle &_ang_tol) const
{
  if (this->IsMetric() != _v.IsMetric())
    return false;

  if (!equal(this->dataPtr->Z(), _v.dataPtr->Z(), _tol))
    return false;

  if (this->IsMetric())
  {
    return equal(this->dataPtr->X(), _v.dataPtr->X(), _tol) &&
      equal(this->dataPtr->Y(), _v.dataPtr->Y(), _tol);
  }
  else
  {
    return
      this->dataPtr->Lat().ShortestDistance(
        _v.dataPtr->Lat()).Abs() <= _ang_tol &&
      this->dataPtr->Lon().ShortestDistance(
        _v.dataPtr->Lon()).Abs() <= _ang_tol;
  }
}

/////////////////////////////////////////////////
bool CoordinateVector3::operator==(const CoordinateVector3 &_v) const
{
  return this->Equal(_v, 1e-3, 1e-3);
}

/////////////////////////////////////////////////
bool CoordinateVector3::operator!=(const CoordinateVector3 &_v) const
{
  return !(*this == _v);
}

/////////////////////////////////////////////////
bool CoordinateVector3::IsFinite() const
{
  if (!std::isfinite(this->dataPtr->Z()))
    return false;
  if (this->IsMetric())
    return std::isfinite(this->dataPtr->X()) &&
      std::isfinite(this->dataPtr->Y());
  else
    return std::isfinite(this->dataPtr->Lat().Radian()) &&
      std::isfinite(this->dataPtr->Lon().Radian());
}

/////////////////////////////////////////////////
bool CoordinateVector3::Equal(const CoordinateVector3 &_v) const
{
  return *this == _v;
}

/////////////////////////////////////////////////
std::optional<double> CoordinateVector3::X() const
{
  if (!this->IsMetric())
    return std::nullopt;
  return this->dataPtr->X();
}

/////////////////////////////////////////////////
std::optional<math::Angle> CoordinateVector3::Lat() const
{
  if (!this->IsSpherical())
    return std::nullopt;
  return this->dataPtr->Lat();
}

/////////////////////////////////////////////////
std::optional<double> CoordinateVector3::Y() const
{
  if (!this->IsMetric())
    return std::nullopt;
  return this->dataPtr->Y();
}

/////////////////////////////////////////////////
std::optional<math::Angle> CoordinateVector3::Lon() const
{
  if (!this->IsSpherical())
    return std::nullopt;
  return this->dataPtr->Lon();
}

/////////////////////////////////////////////////
std::optional<double> CoordinateVector3::Z() const
{
  return this->dataPtr->Z();
}

/////////////////////////////////////////////////
bool CoordinateVector3::X(const double &_v)
{
  if (!this->IsMetric())
    return false;
  this->dataPtr->X() = _v;
  return true;
}

/////////////////////////////////////////////////
bool CoordinateVector3::Lat(const Angle &_v)
{
  if (!this->IsSpherical())
    return false;
  this->dataPtr->Lat() = _v;
  return true;
}

/////////////////////////////////////////////////
bool CoordinateVector3::Y(const double &_v)
{
  if (!this->IsMetric())
    return false;
  this->dataPtr->Y() = _v;
  return true;
}

/////////////////////////////////////////////////
bool CoordinateVector3::Lon(const Angle &_v)
{
  if (!this->IsSpherical())
    return false;
  this->dataPtr->Lon() = _v;
  return true;
}

/////////////////////////////////////////////////
bool CoordinateVector3::Z(const double &_v)
{
  this->dataPtr->z = _v;
  return true;
}
