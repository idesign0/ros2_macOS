/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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

#ifndef GZ_RENDERING_CAPSULE_HH_
#define GZ_RENDERING_CAPSULE_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"
#include "gz/rendering/Geometry.hh"
#include "gz/rendering/Object.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \class Capsule Capsule.hh gz/rendering/Capsule
    /// \brief Geometry for a capsule shape. The APIs allow setting
    /// the capsule's radius and length directly which ensure that the
    /// the geometry remains the shape of a capsule. In comparison, if
    /// the size is changed via the parent Node's Set*Scale function, the
    /// capsule may become deformed.
    class GZ_RENDERING_VISIBLE Capsule :
      public virtual Geometry
    {
      /// \brief Destructor
      public: virtual ~Capsule();

      /// \brief Set the radius of the capsule
      public: virtual void SetRadius(double _radius) = 0;

      /// \brief Set the length of the capsule
      public: virtual void SetLength(double _length) = 0;

      /// \brief Get the radius of the capsule
      public: virtual double Radius() const = 0;

      /// \brief Get the length of the capsule
      public: virtual double Length() const = 0;
    };
    }
  }
}
#endif
