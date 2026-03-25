/*
 * Copyright 2024 CogniPilot Foundation
 * Copyright 2024 Open Source Robotics Foundation
 * Copyright 2024 Rudis Laboratories
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
#ifndef GZ_MATH_CONE_HH_
#define GZ_MATH_CONE_HH_

#include <optional>
#include "gz/math/MassMatrix3.hh"
#include "gz/math/Material.hh"
#include "gz/math/Quaternion.hh"

namespace gz::math
{
  // Forward declarations
  class ConePrivate;

  // Inline bracket to help doxygen filtering.
  inline namespace GZ_MATH_VERSION_NAMESPACE {
  //
  /// \class Cone Cone.hh gz/math/Cone.hh
  /// \brief A representation of a cone.
  ///
  /// The cone class supports defining a cone with a radius,
  /// length, rotational offset, and material properties. Radius and
  /// length are in meters. See Material for more on material properties.
  /// By default, a cone's length is aligned with the Z axis where the base
  /// of the cone is proximal to the origin and vertex points in positive Z.
  /// The rotational offset encodes a rotation from the z axis.
  template<typename Precision>
  class Cone
  {
    /// \brief Default constructor. The default radius and length are both
    /// zero. The default rotational offset is
    /// Quaternion<Precision>::Identity.
    public: Cone() = default;

    /// \brief Construct a cone with a length, radius, and optionally
    /// a rotational offset.
    /// \param[in] _length Length of the cone.
    /// \param[in] _radius Radius of the cone.
    /// \param[in] _rotOffset Rotational offset of the cone.
    public: Cone(const Precision _length, const Precision _radius,
                 const Quaternion<Precision> &_rotOffset =
                 Quaternion<Precision>::Identity);

    /// \brief Construct a cone with a length, radius, material and
    /// optionally a rotational offset.
    /// \param[in] _length Length of the cone.
    /// \param[in] _radius Radius of the cone.
    /// \param[in] _mat Material property for the cone.
    /// \param[in] _rotOffset Rotational offset of the cone.
    public: Cone(const Precision _length, const Precision _radius,
                 const Material &_mat,
                 const Quaternion<Precision> &_rotOffset =
                 Quaternion<Precision>::Identity);

    /// \brief Get the radius in meters.
    /// \return The radius of the cone in meters.
    public: Precision Radius() const;

    /// \brief Set the radius in meters.
    /// \param[in] _radius The radius of the cone in meters.
    public: void SetRadius(const Precision _radius);

    /// \brief Get the length in meters.
    /// \return The length of the cone in meters.
    public: Precision Length() const;

    /// \brief Set the length in meters.
    /// \param[in] _length The length of the cone in meters.
    public: void SetLength(const Precision _length);

    /// \brief Get the rotational offset. By default, a cone's length
    /// is aligned with the Z axis. The rotational offset encodes
    /// a rotation from the z axis.
    /// \return The cone's rotational offset.
    /// \sa void SetRotationalOffset(const Quaternion<Precision> &_rot)
    public: Quaternion<Precision> RotationalOffset() const;

    /// \brief Set the rotation offset.
    /// \param[in] _rotOffset rotational offset quaternion.
    /// See Quaternion<Precision> RotationalOffset() for details on the
    /// rotational offset.
    /// \sa Quaternion<Precision> RotationalOffset() const
    public: void SetRotationalOffset(
                const Quaternion<Precision> &_rotOffset);

    /// \brief Get the material associated with this cone.
    /// \return The material assigned to this cone
    public: const Material &Mat() const;

    /// \brief Set the material associated with this cone.
    /// \param[in] _mat The material assigned to this cone
    public: void SetMat(const Material &_mat);

    /// \brief Get the mass matrix for this cone. This function
    /// is only meaningful if the cone's radius, length, and material
    /// have been set. Optionally, set the rotational offset.
    /// \param[out] _massMat The computed mass matrix will be stored
    /// here.
    /// \return False if computation of the mass matrix failed, which
    /// could be due to an invalid radius (<=0), length (<=0), or density
    /// (<=0).
    public: bool MassMatrix(MassMatrix3d &_massMat) const;

    /// \brief Get the mass matrix for this cone. This function
    /// is only meaningful if the cone's radius, length, and material
    /// have been set. Optionally, set the rotational offset.
    /// \return The computed mass matrix if parameters are valid
    /// (radius > 0), (length > 0) and (density > 0). Otherwise
    /// std::nullopt is returned.
    public: std::optional< MassMatrix3<Precision> > MassMatrix() const;

    /// \brief Check if this cone is equal to the provided cone.
    /// Radius, length, and material properties will be checked.
    public: bool operator==(const Cone &_cone) const;

    /// \brief Get the volume of the cone in m^3.
    /// \return Volume of the cone in m^3.
    public: Precision Volume() const;

    /// \brief Compute the cone's density given a mass value. The
    /// cone is assumed to be solid with uniform density. This
    /// function requires the cone's radius and length to be set to
    /// values greater than zero. The Material of the cone is ignored.
    /// \param[in] _mass Mass of the cone, in kg. This value should be
    /// greater than zero.
    /// \return Density of the cone in kg/m^3. A negative value is
    /// returned if radius, length or _mass is <= 0.
    public: Precision DensityFromMass(const Precision _mass) const;

    /// \brief Set the density of this cone based on a mass value.
    /// Density is computed using
    /// Precision DensityFromMass(const Precision _mass) const. The
    /// cone is assumed to be solid with uniform density. This
    /// function requires the cone's radius and length to be set to
    /// values greater than zero. The existing Material density value is
    /// overwritten only if the return value from this true.
    /// \param[in] _mass Mass of the cone, in kg. This value should be
    /// greater than zero.
    /// \return True if the density was set. False is returned if the
    /// cone's radius, length, or the _mass value are <= 0.
    /// \sa Precision DensityFromMass(const Precision _mass) const
    public: bool SetDensityFromMass(const Precision _mass);

    /// \brief Radius of the cone.
    private: Precision radius = 0.0;

    /// \brief Length of the cone.
    private: Precision length = 0.0;

    /// \brief the cone's material.
    private: Material material;

    /// \brief Rotational offset.
    private: Quaternion<Precision> rotOffset =
             Quaternion<Precision>::Identity;
  };

  /// \typedef Cone<int> Conei
  /// \brief Cone with integer precision.
  typedef Cone<int> Conei;

  /// \typedef Cone<double> Coned
  /// \brief Cone with double precision.
  typedef Cone<double> Coned;

  /// \typedef Cone<float> Conef
  /// \brief Cone with float precision.
  typedef Cone<float> Conef;
  }  // namespace GZ_MATH_VERSION_NAMESPACE
}  // namespace gz::math
#include "gz/math/detail/Cone.hh"
#endif  // GZ_MATH_CONE_HH_
