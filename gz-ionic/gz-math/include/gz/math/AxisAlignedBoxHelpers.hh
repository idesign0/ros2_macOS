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

#ifndef GZ_MATH_AXISALIGNEDBOXHELPERS_HH_
#define GZ_MATH_AXISALIGNEDBOXHELPERS_HH_

#include <gz/math/AxisAlignedBox.hh>
#include <gz/math/Box.hh>
#include <gz/math/Sphere.hh>
#include <gz/math/Capsule.hh>
#include <gz/math/Cylinder.hh>

namespace gz::math
{
  // Inline bracket to help doxygen filtering.
  inline namespace GZ_MATH_VERSION_NAMESPACE {
    /// \class AxisAlignedBoxHelpers
    ///        AxisAlignedBoxHelpers.hh gz/math/AxisAlignedBoxHelpers.hh
    /// \brief A utility class to assist in
    ///        converting different geometric shapes
    ///        into AxisAlignedBox representations.
    ///
    /// This class provides static methods for converting different types of 3D
    /// shapes, such as Box, Sphere, Capsule, and Cylinder, into AxisAlignedBox
    /// objects.
    template<typename Precision>
    class AxisAlignedBoxHelpers
    {
    public:
        /// \brief Convert a Box to an AxisAlignedBox.
        /// \param[in] _box The Box to be converted.
        /// \return An AxisAlignedBox that fully contains the given Box.
        static AxisAlignedBox ConvertToAxisAlignedBox(
          const Box<Precision> &_box);

        /// \brief Convert a Sphere to an AxisAlignedBox.
        /// \param[in] _sphere The Sphere to be converted.
        /// \return An AxisAlignedBox that fully contains the given Sphere.
        static AxisAlignedBox ConvertToAxisAlignedBox(
          const Sphere<Precision> &_sphere);

        /// \brief Convert a Capsule to an AxisAlignedBox.
        /// \param[in] _capsule The Capsule to be converted.
        /// \return An AxisAlignedBox that fully contains the given Capsule.
        static AxisAlignedBox ConvertToAxisAlignedBox(
          const Capsule<Precision> &_capsule);

        /// \brief Convert a Cylinder to an AxisAlignedBox.
        /// \param[in] _cylinder The Cylinder to be converted.
        /// \return An AxisAlignedBox that fully contains the given Cylinder.
        static AxisAlignedBox ConvertToAxisAlignedBox(
          const Cylinder<Precision> &_cylinder);
    };
  }
}  // namespace gz::math

#include "gz/math/detail/AxisAlignedBoxHelpers.hh"
#endif  // GZ_MATH_AXISALIGNEDBOXHELPERS_HH_
