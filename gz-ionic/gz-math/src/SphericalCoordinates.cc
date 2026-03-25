/*
 * Copyright (C) 2012 Open Source Robotics Foundation
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
#include <sstream>
#include <string>

#include "gz/math/Matrix3.hh"
#include "gz/math/SphericalCoordinates.hh"
#include "gz/math/detail/Error.hh"

using namespace gz;
using namespace math;

// Parameters for EARTH_WGS84 model
// wikipedia: World_Geodetic_System#A_new_World_Geodetic_System:_WGS_84

// a: Equatorial radius. Semi-major axis of the WGS84 spheroid (meters).
const double g_EarthWGS84AxisEquatorial = 6378137.0;

// b: Polar radius. Semi-minor axis of the wgs84 spheroid (meters).
const double g_EarthWGS84AxisPolar = 6356752.314245;

// if: WGS84 inverse flattening parameter (no units)
const double g_EarthWGS84Flattening = 1.0/298.257223563;

// Radius of the Earth (meters).
const double g_EarthRadius = 6371000.0;

// Radius of the Moon (meters).
// Source: https://lunar.gsfc.nasa.gov/library/451-SCI-000958.pdf
const double g_MoonRadius = 1737400.0;

// a: Equatorial radius of the Moon.
// Source : https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html
const double g_MoonAxisEquatorial = 1738100.0;

// b: Polar radius of the Moon.
// Source : https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html
const double g_MoonAxisPolar = 1736000.0;

// if: Unitless flattening parameter for the Moon.
// Source : https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html
const double g_MoonFlattening = 0.0012;

// Private data for the SphericalCoordinates class.
class gz::math::SphericalCoordinates::Implementation
{
  /// \brief Type of surface being used.
  public: SphericalCoordinates::SurfaceType surfaceType;

  /// \brief Radius of the given SurfaceType.
  public: double surfaceRadius = 0;

  /// \brief Latitude of reference point.
  public: Angle latitudeReference;

  /// \brief Longitude of reference point.
  public: Angle longitudeReference;

  /// \brief Elevation of reference point relative to sea level in meters.
  public: double elevationReference;

  /// \brief Heading offset, expressed as angle from East to
  ///        gazebo x-axis, or equivalently from North to gazebo y-axis.
  public: Angle headingOffset;

  /// \brief Semi-major axis ellipse parameter
  public: double ellA;

  /// \brief Semi-minor axis ellipse parameter
  public: double ellB;

  /// \brief Flattening ellipse parameter
  public: double ellF;

  /// \brief First eccentricity ellipse parameter
  public: double ellE;

  /// \brief Second eccentricity ellipse parameter
  public: double ellP;

  /// \brief Rotation matrix that moves ECEF to GLOBAL
  public: Matrix3d rotECEFToGlobal;

  /// \brief Rotation matrix that moves GLOBAL to ECEF
  public: Matrix3d rotGlobalToECEF;

  /// \brief Cache the ECEF position of the the origin
  public: CoordinateVector3 origin;

  /// \brief Cache cosine head transform
  public: double cosHea;

  /// \brief Cache sine head transform
  public: double sinHea;
};

//////////////////////////////////////////////////
SphericalCoordinates::SurfaceType SphericalCoordinates::Convert(
  const std::string &_str)
{
  if ("EARTH_WGS84" == _str)
    return EARTH_WGS84;
  else if ("MOON_SCS" == _str)
    return MOON_SCS;
  else if ("CUSTOM_SURFACE" == _str)
    return CUSTOM_SURFACE;

  detail::LogErrorMessage(
      "SurfaceType string not recognized, EARTH_WGS84 returned by default");
  return EARTH_WGS84;
}

//////////////////////////////////////////////////
std::string SphericalCoordinates::Convert(
    SphericalCoordinates::SurfaceType _type)
{
  if (_type == EARTH_WGS84)
    return "EARTH_WGS84";
  else if (_type == MOON_SCS)
    return "MOON_SCS";
  else if (_type == CUSTOM_SURFACE)
    return "CUSTOM_SURFACE";

  detail::LogErrorMessage(
      "SurfaceType not recognized, EARTH_WGS84 returned by default");
  return "EARTH_WGS84";
}

//////////////////////////////////////////////////
SphericalCoordinates::SphericalCoordinates()
  : dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->SetSurface(EARTH_WGS84);
  this->SetElevationReference(0.0);
}

//////////////////////////////////////////////////
SphericalCoordinates::SphericalCoordinates(const SurfaceType _type)
  : SphericalCoordinates()
{
  this->SetSurface(_type);
  this->SetElevationReference(0.0);
}

//////////////////////////////////////////////////
SphericalCoordinates::SphericalCoordinates(
    const SurfaceType _type,
    const double _axisEquatorial,
    const double _axisPolar)
  : SphericalCoordinates()
{
  // Set properties
  this->SetSurface(_type, _axisEquatorial,
      _axisPolar);

  this->SetElevationReference(0.0);
}

//////////////////////////////////////////////////
SphericalCoordinates::SphericalCoordinates(const SurfaceType _type,
    const Angle &_latitude,
    const Angle &_longitude,
    const double _elevation,
    const Angle &_heading)
  : SphericalCoordinates()
{
  // Set the reference and calculate ellipse parameters
  this->SetSurface(_type);

  // Set the coordinate transform parameters
  this->dataPtr->latitudeReference = _latitude;
  this->dataPtr->longitudeReference = _longitude;
  this->dataPtr->elevationReference = _elevation;
  this->dataPtr->headingOffset = _heading;

  // Generate transformation matrix
  this->UpdateTransformationMatrix();
}

//////////////////////////////////////////////////
SphericalCoordinates::SurfaceType SphericalCoordinates::Surface() const
{
  return this->dataPtr->surfaceType;
}

//////////////////////////////////////////////////
Angle SphericalCoordinates::LatitudeReference() const
{
  return this->dataPtr->latitudeReference;
}

//////////////////////////////////////////////////
Angle SphericalCoordinates::LongitudeReference() const
{
  return this->dataPtr->longitudeReference;
}

//////////////////////////////////////////////////
double SphericalCoordinates::ElevationReference() const
{
  return this->dataPtr->elevationReference;
}

//////////////////////////////////////////////////
Angle SphericalCoordinates::HeadingOffset() const
{
  return this->dataPtr->headingOffset;
}

//////////////////////////////////////////////////
void SphericalCoordinates::SetSurface(const SurfaceType &_type)
{
  this->dataPtr->surfaceType = _type;

  switch (this->dataPtr->surfaceType)
  {
    case EARTH_WGS84:
      {
      // Set the semi-major axis
      this->dataPtr->ellA = g_EarthWGS84AxisEquatorial;

      // Set the semi-minor axis
      this->dataPtr->ellB = g_EarthWGS84AxisPolar;

      // Set the flattening parameter
      this->dataPtr->ellF = g_EarthWGS84Flattening;

      // Set the first eccentricity ellipse parameter
      // https://en.wikipedia.org/wiki/Eccentricity_(mathematics)#Ellipses
      this->dataPtr->ellE = sqrt(1.0 -
          std::pow(this->dataPtr->ellB, 2) / std::pow(this->dataPtr->ellA, 2));

      // Set the second eccentricity ellipse parameter
      // https://en.wikipedia.org/wiki/Eccentricity_(mathematics)#Ellipses
      this->dataPtr->ellP = sqrt(
          std::pow(this->dataPtr->ellA, 2) / std::pow(this->dataPtr->ellB, 2) -
          1.0);

      // Set the radius of the surface.
      this->dataPtr->surfaceRadius = g_EarthRadius;

      break;
      }
    case MOON_SCS:
      {
      // Set the semi-major axis
      this->dataPtr->ellA = g_MoonAxisEquatorial;

      // Set the semi-minor axis
      this->dataPtr->ellB = g_MoonAxisPolar;

      // Set the flattening parameter
      this->dataPtr->ellF = g_MoonFlattening;

      // Set the first eccentricity ellipse parameter
      // https://en.wikipedia.org/wiki/Eccentricity_(mathematics)#Ellipses
      this->dataPtr->ellE = sqrt(1.0 -
          std::pow(this->dataPtr->ellB, 2) / std::pow(this->dataPtr->ellA, 2));

      // Set the second eccentricity ellipse parameter
      // https://en.wikipedia.org/wiki/Eccentricity_(mathematics)#Ellipses
      this->dataPtr->ellP = sqrt(
          std::pow(this->dataPtr->ellA, 2) / std::pow(this->dataPtr->ellB, 2) -
          1.0);

      // Set the radius of the surface.
      this->dataPtr->surfaceRadius = g_MoonRadius;

      break;
      }
    case CUSTOM_SURFACE:
      {
      detail::LogErrorMessage(
          "For custom surfaces, use SetSurface(type, radius,"
          "axisEquatorial, axisPolar)");
      break;
      }
    default:
      {
      std::ostringstream errStream;
      errStream << "Unknown surface type[" << this->dataPtr->surfaceType << "]";
      detail::LogErrorMessage(errStream.str());
      break;
      }
  }
}

//////////////////////////////////////////////////
void SphericalCoordinates::SetSurface(
    const SurfaceType &_type,
    const double _axisEquatorial,
    const double _axisPolar)
{
  if ((_type != EARTH_WGS84) &&
      (_type != MOON_SCS) &&
      (_type != CUSTOM_SURFACE))
  {
    std::ostringstream errStream;
    errStream << "Unknown surface type[" << _type << "]";
    detail::LogErrorMessage(errStream.str());
    return;
  }

  this->dataPtr->surfaceType = _type;

  if ((_axisEquatorial > 0)
      && (_axisPolar > 0)
      && (_axisPolar <= _axisEquatorial))
  {
    this->dataPtr->ellA = _axisEquatorial;
    this->dataPtr->ellB = _axisPolar;
    this->dataPtr->ellF =
      (_axisEquatorial - _axisPolar) / _axisEquatorial;
    // Arithmetic mean radius
    this->dataPtr->surfaceRadius =
      (2 * _axisEquatorial + _axisPolar) / 3.0;
  }
  else
  {
    detail::LogErrorMessage(
        "Invalid parameters found, defaulting to Earth's parameters");

    this->dataPtr->ellA = g_EarthWGS84AxisEquatorial;
    this->dataPtr->ellB = g_EarthWGS84AxisPolar;
    this->dataPtr->ellF = g_EarthWGS84Flattening;
    this->dataPtr->surfaceRadius = g_EarthRadius;
  }

  this->dataPtr->ellE = sqrt(1.0 -
      std::pow(this->dataPtr->ellB, 2) / std::pow(this->dataPtr->ellA, 2));
  this->dataPtr->ellP = sqrt(
      std::pow(this->dataPtr->ellA, 2) / std::pow(this->dataPtr->ellB, 2) -
      1.0);
}

//////////////////////////////////////////////////
void SphericalCoordinates::SetLatitudeReference(
    const Angle &_angle)
{
  this->dataPtr->latitudeReference = _angle;
  this->UpdateTransformationMatrix();
}

//////////////////////////////////////////////////
void SphericalCoordinates::SetLongitudeReference(
    const Angle &_angle)
{
  this->dataPtr->longitudeReference = _angle;
  this->UpdateTransformationMatrix();
}

//////////////////////////////////////////////////
void SphericalCoordinates::SetElevationReference(const double _elevation)
{
  this->dataPtr->elevationReference = _elevation;
  this->UpdateTransformationMatrix();
}

//////////////////////////////////////////////////
void SphericalCoordinates::SetHeadingOffset(const Angle &_angle)
{
  this->dataPtr->headingOffset.SetRadian(_angle.Radian());
  this->UpdateTransformationMatrix();
}

//////////////////////////////////////////////////
Vector3d SphericalCoordinates::SphericalFromLocalPosition(
    const Vector3d &_xyz) const
{
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  Vector3d result = this->PositionTransform(_xyz, LOCAL, SPHERICAL);
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
  result.X(GZ_RTOD(result.X()));
  result.Y(GZ_RTOD(result.Y()));
  return result;
}

//////////////////////////////////////////////////
std::optional<math::CoordinateVector3>
SphericalCoordinates::SphericalFromLocalPosition(
    const math::CoordinateVector3& _xyz) const
{
  return this->PositionTransform(_xyz, LOCAL, SPHERICAL);
}

//////////////////////////////////////////////////
Vector3d SphericalCoordinates::LocalFromSphericalPosition(
    const Vector3d &_xyz) const
{
  Vector3d result = _xyz;
  result.X(GZ_DTOR(result.X()));
  result.Y(GZ_DTOR(result.Y()));
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  return this->PositionTransform(result, SPHERICAL, LOCAL);
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

//////////////////////////////////////////////////
std::optional<math::CoordinateVector3>
SphericalCoordinates::LocalFromSphericalPosition(
  const math::CoordinateVector3& _xyz) const
{
  return this->PositionTransform(_xyz, SPHERICAL, LOCAL);
}

//////////////////////////////////////////////////
Vector3d SphericalCoordinates::GlobalFromLocalVelocity(
    const Vector3d &_xyz) const
{
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  return this->VelocityTransform(_xyz, LOCAL, GLOBAL);
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

//////////////////////////////////////////////////
std::optional<math::CoordinateVector3>
SphericalCoordinates::GlobalFromLocalVelocity(
    const math::CoordinateVector3 &_xyz) const
{
  return this->VelocityTransform(_xyz, LOCAL, GLOBAL);
}

//////////////////////////////////////////////////
Vector3d SphericalCoordinates::LocalFromGlobalVelocity(
    const Vector3d &_xyz) const
{
  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  return this->VelocityTransform(_xyz, GLOBAL, LOCAL);
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
}

//////////////////////////////////////////////////
std::optional<math::CoordinateVector3>
SphericalCoordinates::LocalFromGlobalVelocity(
    const math::CoordinateVector3 &_xyz) const
{
  return this->VelocityTransform(_xyz, GLOBAL, LOCAL);
}

//////////////////////////////////////////////////
/// Based on Haversine formula (http://en.wikipedia.org/wiki/Haversine_formula).
double SphericalCoordinates::DistanceWGS84(const Angle &_latA,
                                      const Angle &_lonA,
                                      const Angle &_latB,
                                      const Angle &_lonB)
{
  Angle dLat = _latB - _latA;
  Angle dLon = _lonB - _lonA;

  double a = sin(dLat.Radian() / 2) * sin(dLat.Radian() / 2) +
             sin(dLon.Radian() / 2) * sin(dLon.Radian() / 2) *
             cos(_latA.Radian()) * cos(_latB.Radian());

  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double d = g_EarthRadius * c;
  return d;
}

//////////////////////////////////////////////////
/// Based on Haversine formula (http://en.wikipedia.org/wiki/Haversine_formula).
/// This takes into account the surface type.
double SphericalCoordinates::DistanceBetweenPoints(
    const Angle &_latA,
    const Angle &_lonA,
    const Angle &_latB,
    const Angle &_lonB)
{
  Angle dLat = _latB - _latA;
  Angle dLon = _lonB - _lonA;

  double a = sin(dLat.Radian() / 2) * sin(dLat.Radian() / 2) +
             sin(dLon.Radian() / 2) * sin(dLon.Radian() / 2) *
             cos(_latA.Radian()) * cos(_latB.Radian());

  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double d = this->dataPtr->surfaceRadius * c;
  return d;
}

//////////////////////////////////////////////////
double SphericalCoordinates::SurfaceRadius() const
{
  return this->dataPtr->surfaceRadius;
}

//////////////////////////////////////////////////
double SphericalCoordinates::SurfaceAxisEquatorial() const
{
  return this->dataPtr->ellA;
}

//////////////////////////////////////////////////
double SphericalCoordinates::SurfaceAxisPolar() const
{
  return this->dataPtr->ellB;
}

//////////////////////////////////////////////////
double SphericalCoordinates::SurfaceFlattening() const
{
  return this->dataPtr->ellF;
}

//////////////////////////////////////////////////
void SphericalCoordinates::UpdateTransformationMatrix()
{
  // Cache trig results
  double cosLat = cos(this->dataPtr->latitudeReference.Radian());
  double sinLat = sin(this->dataPtr->latitudeReference.Radian());
  double cosLon = cos(this->dataPtr->longitudeReference.Radian());
  double sinLon = sin(this->dataPtr->longitudeReference.Radian());

  // Create a rotation matrix that moves ECEF to GLOBAL
  // http://www.navipedia.net/index.php/
  // Transformations_between_ECEF_and_ENU_coordinates
  this->dataPtr->rotECEFToGlobal = Matrix3d(
                      -sinLon,           cosLon,          0.0,
                      -cosLon * sinLat, -sinLon * sinLat, cosLat,
                       cosLon * cosLat,  sinLon * cosLat, sinLat);

  // Create a rotation matrix that moves GLOBAL to ECEF
  // http://www.navipedia.net/index.php/
  // Transformations_between_ECEF_and_ENU_coordinates
  this->dataPtr->rotGlobalToECEF = Matrix3d(
                      -sinLon, -cosLon * sinLat, cosLon * cosLat,
                       cosLon, -sinLon * sinLat, sinLon * cosLat,
                       0,      cosLat,           sinLat);

  // Cache heading transforms -- note that we have to negate the heading in
  // order to preserve backward compatibility. ie. Gazebo has traditionally
  // expressed positive angle as a CLOCKWISE rotation that takes the GLOBAL
  // frame to the LOCAL frame. However, right hand coordinate systems require
  // this to be expressed as an ANTI-CLOCKWISE rotation. So, we negate it.
  this->dataPtr->cosHea = cos(-this->dataPtr->headingOffset.Radian());
  this->dataPtr->sinHea = sin(-this->dataPtr->headingOffset.Radian());

  // Cache the ECEF coordinate of the origin
  this->dataPtr->origin = CoordinateVector3::Spherical(
    this->dataPtr->latitudeReference,
    this->dataPtr->longitudeReference,
    this->dataPtr->elevationReference);
  this->dataPtr->origin =
    *this->PositionTransform(this->dataPtr->origin, SPHERICAL, ECEF);
}

namespace
{

/////////////////////////////////////////////////
std::optional<CoordinateVector3> PositionTransformTmp(
    const gz::utils::ImplPtr<SphericalCoordinates::Implementation>& dataPtr,
    const CoordinateVector3 &_pos,
    const SphericalCoordinates::CoordinateType &_in,
    const SphericalCoordinates::CoordinateType &_out)
{
  // This unusual concept of passing dataPtr as a free-function argument is just
  // a temporary measure for GZ 8. Starting with GZ 9, the code of this function
  // should be moved inside PositionTransform(CoordinateVector3).

  if ((_in == SphericalCoordinates::SPHERICAL) != _pos.IsSpherical())
  {
    detail::LogErrorMessage(
        "Invalid input to PositionTransform. "
        "The passed coordinate vector has wrong type.");
    return std::nullopt;
  }
  Vector3d tmp;

  // Convert whatever arrives to a more flexible ECEF coordinate
  switch (_in)
  {
    // East, North, Up (ENU)
    // When branching code for GZ 9, replace this code block with the block
    // from LOCAL2 and remove the LOCAL2 block.
    case SphericalCoordinates::LOCAL:
      {
        // This is incorrect computation
        tmp.X(-*_pos.X() * dataPtr->cosHea + *_pos.Y() * dataPtr->sinHea);
        tmp.Y(-*_pos.X() * dataPtr->sinHea - *_pos.Y() * dataPtr->cosHea);
        tmp.Z(*_pos.Z());
        tmp = *dataPtr->origin.AsMetricVector() +
          dataPtr->rotGlobalToECEF * tmp;
        break;
      }

    GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
    case SphericalCoordinates::LOCAL2:
    GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
      {
        // This is correct computation
        tmp.X(*_pos.X() * dataPtr->cosHea + *_pos.Y() * dataPtr->sinHea);
        tmp.Y(-*_pos.X() * dataPtr->sinHea + *_pos.Y() * dataPtr->cosHea);
        tmp.Z(*_pos.Z());
        tmp = *dataPtr->origin.AsMetricVector() +
          dataPtr->rotGlobalToECEF * tmp;
        break;
      }

    case SphericalCoordinates::GLOBAL:
      {
        tmp = *_pos.AsMetricVector();
        tmp = *dataPtr->origin.AsMetricVector() +
          dataPtr->rotGlobalToECEF * tmp;
        break;
      }

    case SphericalCoordinates::SPHERICAL:
      {
        // Cache trig results
        const auto latRad = _pos.Lat()->Radian();
        const auto lonRad = _pos.Lon()->Radian();
        double cosLat = cos(latRad);
        double sinLat = sin(latRad);
        double cosLon = cos(lonRad);
        double sinLon = sin(lonRad);

        // Radius of planet curvature (meters)
        double curvature =
          1.0 - dataPtr->ellE * dataPtr->ellE * sinLat * sinLat;
        curvature = dataPtr->ellA / sqrt(curvature);

        tmp.X((*_pos.Z() + curvature) * cosLat * cosLon);
        tmp.Y((*_pos.Z() + curvature) * cosLat * sinLon);
        tmp.Z(((dataPtr->ellB * dataPtr->ellB)/
              (dataPtr->ellA * dataPtr->ellA) *
              curvature + *_pos.Z()) * sinLat);
        break;
      }

    // Just copy the vector.
    case SphericalCoordinates::ECEF:
      tmp = *_pos.AsMetricVector();
      break;
    default:
      {
        std::ostringstream errStream;
        errStream << "Invalid coordinate type[" << _in << "]";
        detail::LogErrorMessage(errStream.str());
        return std::nullopt;
      }
  }

  CoordinateVector3 res;
  // Convert ECEF to the requested output coordinate system
  switch (_out)
  {
    case SphericalCoordinates::SPHERICAL:
      {
        // Convert from ECEF to SPHERICAL
        double p = sqrt(tmp.X() * tmp.X() + tmp.Y() * tmp.Y());
        double theta = atan((tmp.Z() * dataPtr->ellA) /
            (p * dataPtr->ellB));

        // Calculate latitude and longitude
        double lat = atan(
            (tmp.Z() + std::pow(dataPtr->ellP, 2) * dataPtr->ellB *
             std::pow(sin(theta), 3)) /
            (p - std::pow(dataPtr->ellE, 2) *
             dataPtr->ellA * std::pow(cos(theta), 3)));

        double lon = atan2(tmp.Y(), tmp.X());

        // Recalculate radius of planet curvature at the current latitude.
        double nCurvature = 1.0 - std::pow(dataPtr->ellE, 2) *
          std::pow(sin(lat), 2);
        nCurvature = dataPtr->ellA / sqrt(nCurvature);

        res = CoordinateVector3::Spherical(lat, lon, p/cos(lat) - nCurvature);
        break;
      }

    // Convert from ECEF TO GLOBAL
    case SphericalCoordinates::GLOBAL:
      tmp = dataPtr->rotECEFToGlobal *
        (tmp - *dataPtr->origin.AsMetricVector());
      res.SetMetric(tmp);
      break;

    // Convert from ECEF TO LOCAL
    case SphericalCoordinates::LOCAL:
    GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
    case SphericalCoordinates::LOCAL2:
    GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
      tmp = dataPtr->rotECEFToGlobal *
        (tmp - *dataPtr->origin.AsMetricVector());

      res.SetMetric(
          tmp.X() * dataPtr->cosHea - tmp.Y() * dataPtr->sinHea,
          tmp.X() * dataPtr->sinHea + tmp.Y() * dataPtr->cosHea,
          tmp.Z());
      break;

    // Return ECEF (do nothing)
    case SphericalCoordinates::ECEF:
      res.SetMetric(tmp);
      break;

    default:
      {
        std::ostringstream errStream;
        errStream << "Unknown coordinate type[" << _out << "]";
        detail::LogErrorMessage(errStream.str());
        return std::nullopt;
      }
  }

  return res;
}

}

/////////////////////////////////////////////////
Vector3d SphericalCoordinates::PositionTransform(
    const Vector3d &_pos,
    const CoordinateType &_in, const CoordinateType &_out) const
{
  // This deprecated implementation accepts and returns radians for spherical
  // coordinates and has a computation bug when working with LOCAL frames.

  CoordinateVector3 vec = _in == SPHERICAL ?
    CoordinateVector3::Spherical(_pos.X(), _pos.Y(), _pos.Z()) :
    CoordinateVector3::Metric(_pos.X(), _pos.Y(), _pos.Z());

  const auto result = PositionTransformTmp(this->dataPtr, vec, _in, _out);
  if (!result)
    return _pos;

  return result->IsMetric() ?
    *result->AsMetricVector() :
    Vector3d{result->Lat()->Radian(), result->Lon()->Radian(), *result->Z()};
}

/////////////////////////////////////////////////
std::optional<CoordinateVector3> SphericalCoordinates::PositionTransform(
  const CoordinateVector3 &_pos,
  const CoordinateType &_in, const CoordinateType &_out) const
{
  // Temporarily, for Gazebo 8, this function turns all LOCAL frames into
  // LOCAL2 to get correct results. Basically, LOCAL and LOCAL2 are equal
  // when PositionTransform() is called with a CoordinateVector3 argument, while
  // it returns the compatible (but wrong) result when called with Vector3d and
  // LOCAL. From Gazebo 9 onwards, LOCAL2 frame will be removed and these
  // differences will disappear.
  // TODO(peci1): Move PositionTransformTmp code into this function in GZ 9.

  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  const auto in = _in == LOCAL ? LOCAL2 : _in;
  const auto out = _out == LOCAL ? LOCAL2 : _out;
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
  return PositionTransformTmp(this->dataPtr, _pos, in, out);
}

namespace
{

//////////////////////////////////////////////////
std::optional<CoordinateVector3> VelocityTransformTmp(
    const gz::utils::ImplPtr<SphericalCoordinates::Implementation>& dataPtr,
    const CoordinateVector3 &_vel,
    const SphericalCoordinates::CoordinateType &_in,
    const SphericalCoordinates::CoordinateType &_out)
{
  // This unusual concept of passing dataPtr as a free-function argument is just
  // a temporary measure for GZ 8. Starting with GZ 9, the code of this function
  // should be moved inside VelocityTransform(CoordinateVector3).

  // Sanity check -- velocity should not be expressed in spherical coordinates
  if (_in == SphericalCoordinates::SPHERICAL ||
      _out == SphericalCoordinates::SPHERICAL ||
      _vel.IsSpherical())
  {
    detail::LogErrorMessage(
        "Velocity cannot be expressed in spherical coordinates.");
    return std::nullopt;
  }

  // Intermediate data type
  Vector3d tmp = *_vel.AsMetricVector();

  // First, convert to an ECEF vector
  switch (_in)
  {
    // ENU
    // When branching code for GZ 9, replace this code block with the block
    // from LOCAL2 and remove the LOCAL2 block.
    case SphericalCoordinates::LOCAL:
      // This is incorrect computation
      tmp.X(-*_vel.X() * dataPtr->cosHea + *_vel.Y() * dataPtr->sinHea);
      tmp.Y(-*_vel.X() * dataPtr->sinHea - *_vel.Y() * dataPtr->cosHea);
      tmp = dataPtr->rotGlobalToECEF * tmp;
      break;
    GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
    case SphericalCoordinates::LOCAL2:
    GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
      // This is correct computation
      tmp.X(*_vel.X() * dataPtr->cosHea + *_vel.Y() * dataPtr->sinHea);
      tmp.Y(-*_vel.X() * dataPtr->sinHea + *_vel.Y() * dataPtr->cosHea);
      tmp = dataPtr->rotGlobalToECEF * tmp;
      break;
    // spherical
    case SphericalCoordinates::GLOBAL:
      tmp = dataPtr->rotGlobalToECEF * tmp;
      break;
    // Do nothing
    case SphericalCoordinates::ECEF:
      break;
    default:
      {
        std::ostringstream errStream;
        errStream << "Unknown coordinate type[" << _in << "]";
        detail::LogErrorMessage(errStream.str());
        return std::nullopt;
      }
  }

  CoordinateVector3 res;

  // Then, convert to the request coordinate type
  switch (_out)
  {
    // ECEF, do nothing
    case SphericalCoordinates::ECEF:
      res.SetMetric(tmp);
      break;

    // Convert from ECEF to global
    case SphericalCoordinates::GLOBAL:
      tmp = dataPtr->rotECEFToGlobal * tmp;
      res.SetMetric(tmp);
      break;

    // Convert from ECEF to local
    case SphericalCoordinates::LOCAL:
    GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
    case SphericalCoordinates::LOCAL2:
    GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
      tmp = dataPtr->rotECEFToGlobal * tmp;
      res.SetMetric(
          tmp.X() * dataPtr->cosHea - tmp.Y() * dataPtr->sinHea,
          tmp.X() * dataPtr->sinHea + tmp.Y() * dataPtr->cosHea,
          tmp.Z());
      break;

    default:
      {
        std::ostringstream errStream;
        errStream << "Unknown coordinate type[" << _out << "]";
        detail::LogErrorMessage(errStream.str());
        return std::nullopt;
      }
  }

  return res;
}

}

//////////////////////////////////////////////////
Vector3d SphericalCoordinates::VelocityTransform(
    const Vector3d &_vel,
    const CoordinateType &_in, const CoordinateType &_out) const
{
  auto vec = CoordinateVector3::Metric(_vel);

  const auto result = VelocityTransformTmp(this->dataPtr, vec, _in, _out);
  if (!result || !result->IsMetric())
    return _vel;

  return *result->AsMetricVector();
}

//////////////////////////////////////////////////
std::optional<CoordinateVector3> SphericalCoordinates::VelocityTransform(
    const CoordinateVector3 &_vel,
    const CoordinateType &_in, const CoordinateType &_out) const
{
  // Temporarily, for Gazebo 8, this function turns all LOCAL frames into
  // LOCAL2 to get correct results. Basically, LOCAL and LOCAL2 are equal
  // when VelocityTransform() is called with a CoordinateVector3 argument, while
  // it returns the compatible (but wrong) result when called with Vector3d and
  // LOCAL. From Gazebo 9 onwards, LOCAL2 frame will be removed and these
  // differences will disappear.
  // TODO(peci1): Move VelocityTransformTmp code into this function in GZ 9.

  GZ_UTILS_WARN_IGNORE__DEPRECATED_DECLARATION
  const auto in = _in == LOCAL ? LOCAL2 : _in;
  const auto out = _out == LOCAL ? LOCAL2 : _out;
  GZ_UTILS_WARN_RESUME__DEPRECATED_DECLARATION
  return VelocityTransformTmp(this->dataPtr, _vel, in, out);
}

//////////////////////////////////////////////////
bool SphericalCoordinates::operator==(const SphericalCoordinates &_sc) const
{
  return this->Surface() == _sc.Surface() &&
         this->LatitudeReference() == _sc.LatitudeReference() &&
         this->LongitudeReference() == _sc.LongitudeReference() &&
         equal(this->ElevationReference(), _sc.ElevationReference()) &&
         this->HeadingOffset() == _sc.HeadingOffset();
}

//////////////////////////////////////////////////
bool SphericalCoordinates::operator!=(const SphericalCoordinates &_sc) const
{
  return !(*this == _sc);
}
