/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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
 */
#ifndef GZ_MATH_DETAIL_AXISALIGNEDBOXHELPERS_HH_
#define GZ_MATH_DETAIL_AXISALIGNEDBOXHELPERS_HH_

#include <gz/math/Vector3.hh>
#include <gz/math/AxisAlignedBox.hh>
#include <gz/math/AxisAlignedBoxHelpers.hh>
#include <gz/math/Box.hh>
#include <gz/math/Sphere.hh>
#include <gz/math/Capsule.hh>
#include <gz/math/Cylinder.hh>

namespace gz::math
{
    //////////////////////////////////////////////////
    /// \brief Converts a Box object to an AxisAlignedBox.
    ///
    /// This function takes a Box instance
    /// and computes its corresponding
    /// AxisAlignedBox, which is centered at the
    /// origin and has half the size
    /// of the original box in all dimensions.
    ///
    /// \tparam Precision The numeric precision type.
    /// \param[in] _box The Box object to be converted.
    /// \return The corresponding AxisAlignedBox.
    //////////////////////////////////////////////////
    template<typename Precision>
    AxisAlignedBox AxisAlignedBoxHelpers<Precision>::ConvertToAxisAlignedBox(
        const Box<Precision> &_box)
    {
        Vector3<Precision> size = _box.Size();
        return AxisAlignedBox(
            Vector3<Precision>(-size.X() / 2, -size.Y() / 2, -size.Z() / 2),
            Vector3<Precision>(size.X() / 2, size.Y() / 2, size.Z() / 2));
    }

    //////////////////////////////////////////////////
    /// \brief Converts a Sphere object to an AxisAlignedBox.
    ///
    /// This function takes a Sphere instance
    /// and computes its corresponding
    /// AxisAlignedBox, which is a cube enclosing
    /// the sphere with side lengths
    /// equal to twice the sphere's radius.
    ///
    /// \tparam Precision The numeric precision type.
    /// \param[in] _sphere The Sphere object to be converted.
    /// \return The corresponding AxisAlignedBox.
    //////////////////////////////////////////////////
    template<typename Precision>
    AxisAlignedBox AxisAlignedBoxHelpers<Precision>::ConvertToAxisAlignedBox(
        const Sphere<Precision> &_sphere)
    {
        Precision radius = _sphere.Radius();
        return AxisAlignedBox(
            Vector3<Precision>(-radius, -radius, -radius),
            Vector3<Precision>(radius, radius, radius));
    }

    //////////////////////////////////////////////////
    /// \brief Converts a Capsule object to an AxisAlignedBox.
    ///
    /// This function takes a Capsule instance
    /// and computes its corresponding
    /// AxisAlignedBox, which fully encloses the
    /// capsule by considering both
    /// its cylindrical body and hemispherical ends.
    ///
    /// \tparam Precision The numeric precision type.
    /// \param[in] _capsule The Capsule object to be converted.
    /// \return The corresponding AxisAlignedBox.
    //////////////////////////////////////////////////
    template<typename Precision>
    AxisAlignedBox AxisAlignedBoxHelpers<Precision>::ConvertToAxisAlignedBox(
        const Capsule<Precision> &_capsule)
    {
        Precision radius = _capsule.Radius();
        Precision length = _capsule.Length();
        return AxisAlignedBox(
            Vector3<Precision>(-radius, -radius, -length / 2 - radius),
            Vector3<Precision>(radius, radius, length / 2 + radius));
    }

    //////////////////////////////////////////////////
    /// \brief Converts a Cylinder object to an AxisAlignedBox.
    ///
    /// This function takes a Cylinder instance and computes its corresponding
    /// AxisAlignedBox, which fully encloses the cylinder based on its radius
    /// and height.
    ///
    /// \tparam Precision The numeric precision type.
    /// \param[in] _cylinder The Cylinder object to be converted.
    /// \return The corresponding AxisAlignedBox.
    //////////////////////////////////////////////////
    template<typename Precision>
    AxisAlignedBox AxisAlignedBoxHelpers<Precision>::ConvertToAxisAlignedBox(
        const Cylinder<Precision> &_cylinder)
    {
        Precision radius = _cylinder.Radius();
        Precision length = _cylinder.Length();
        return AxisAlignedBox(
            Vector3<Precision>(-radius, -radius, -length / 2),
            Vector3<Precision>(radius, radius, length / 2));
    }

}  // namespace gz::math

#endif  // GZ_MATH_DETAIL_AXISALIGNEDBOXHELPERS_HH_
