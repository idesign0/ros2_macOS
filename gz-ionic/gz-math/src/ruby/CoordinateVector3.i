/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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

#ifdef SWIGRUBY
%begin %{
#define HAVE_ISFINITE 1
%}
#endif

%module coordinatevector3
%{
#include <optional>

#include <gz/math/Angle.hh>
#include <gz/math/CoordinateVector3.hh>
#include <gz/math/Vector3.hh>
%}

namespace gz
{
  namespace math
  {
    class CoordinateVector3
    {
      public: CoordinateVector3();
      public: CoordinateVector3(const CoordinateVector3 &_other);
      public: static CoordinateVector3 Metric(double _x, double _y, double _z);
      public: static CoordinateVector3 Metric(
                  const gz::math::Vector3<double> &_v);
      public: static CoordinateVector3 Spherical(const gz::math::Angle& _lat,
                  const gz::math::Angle& _lon, double _z);
      public: bool IsMetric() const;
      public: bool IsSpherical() const;
      public: void SetMetric(double _x, double _y, double _z);
      public: void SetMetric(const gz::math::Vector3<double> &_v);
      public: void SetSpherical(const gz::math::Angle& _lat,
                  const gz::math::Angle& _lon, double _z);
      public: std::optional<gz::math::Vector3d> AsMetricVector() const;
      public: CoordinateVector3 operator+(const CoordinateVector3 &_v) const;
      public: CoordinateVector3 operator-() const;
      public: CoordinateVector3 operator-(const CoordinateVector3 &_pt) const;
      public: bool Equal(const CoordinateVector3 &_v, const double &_tol,
                  const gz::math::Angle &_ang_tol) const;
      public: bool operator==(const CoordinateVector3 &_v) const;
      public: bool IsFinite() const;
      public: bool Equal(const CoordinateVector3 &_v) const;
      public: std::optional<double> X() const;
      public: std::optional<gz::math::Angle> Lat() const;
      public: std::optional<double> Y() const;
      public: std::optional<gz::math::Angle> Lon() const;
      public: std::optional<double> Z() const;
      public: void X(const double &_v);
      public: void Lat(const gz::math::Angle &_v);
      public: void Y(const double &_v);
      public: void Lon(const gz::math::Angle &_v);
      public: void Z(const double &_v);
    };
  }
}
