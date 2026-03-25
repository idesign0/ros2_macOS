/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_RAYQUERY_HH_
#define GZ_RENDERING_RAYQUERY_HH_

#include <gz/utils/SuppressWarning.hh>
#include <gz/math/Vector3.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/Camera.hh"
#include "gz/rendering/Scene.hh"
#include "gz/rendering/Visual.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief A class that stores ray query intersection results.
    class GZ_RENDERING_VISIBLE RayQueryResult
    {
      /// \brief Intersection distance
      public: double distance = -1;

      /// \brief Intersection point in 3d space
      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      public: math::Vector3d point;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING

      /// \brief Intersected object id
      public: unsigned int objectId = 0;

      /// \brief Returns false if result is not valid
      public: operator bool() const
              {
                return distance > 0;
              }

      /// \brief Returns false if result is not valid
      public: operator bool()
              {
                return distance > 0;
              }
    };

    /// \class RayQuery RayQuery.hh gz/rendering/RayQuery.hh
    /// \brief A Ray Query class used for computing ray object intersections
    class GZ_RENDERING_VISIBLE RayQuery
        : public virtual Object
    {
      /// \brief Destructor
      public: virtual ~RayQuery();

      /// \brief Set ray origin
      /// \param[in] _origin Ray origin
      public: virtual void SetOrigin(const math::Vector3d &_origin) = 0;

      /// \brief Get ray origin
      /// \return Ray origin
      public: virtual math::Vector3d Origin() const = 0;

      /// \brief Set ray direction
      /// \param[in] _dir Ray origin
      public: virtual void SetDirection(const math::Vector3d &_dir) = 0;

      /// \brief Get ray direction
      /// \return Ray direction.
      public: virtual math::Vector3d Direction() const = 0;

      /// \brief Create the ray query from camera
      /// \param[in] _camera Camera to construct ray
      /// \param[in] _coord normalized device coords [-1, +1]
      public: virtual void SetFromCamera(const CameraPtr &_camera,
                const math::Vector2d &_coord) = 0;

      /// \brief Create the ray query from WideAngleCamera
      /// \param[in] _camera Camera to construct ray
      /// \param[in] _faceIdx In range [0; 6)
      ///   * 0 = +X
      ///   * 1 = -X
      ///   * 2 = +Y
      ///   * 3 = -Y
      ///   * 4 = +Z
      ///   * 5 = -Z
      /// \param[in] _coord normalized device coords [-1, +1]
      public: virtual void SetFromCamera(const WideAngleCameraPtr &_camera,
                                         uint32_t _faceIdx,
                                         const math::Vector2d &_coord) = 0;

      /// \brief If possible & supported (and user called SetFromCamera())
      /// will use the GPU to perform the ray query.
      /// Triangle-level accurate.
      /// \remark It's not guaranteed the GPU will be used. See UsesGpu()
      /// \remark Using the GPU is not necessarily faster.
      /// \param[in] _preferGpu True to use the GPU if available & possible.
      /// False to never use the GPU.
      public: virtual void SetPreferGpu(bool _preferGpu) = 0;

      /// \brief Returns true if the GPU will be used for the next
      /// ClosestPoint() call.
      /// \remark The thread ID is important. Calling this value from
      /// different threads may result in different return values.
      /// \return True if next ClosestPoint() call will use the GPU.
      public: virtual bool UsesGpu() const = 0;

      /// \brief Compute intersections
      /// \param[in] _forceSceneUpdate Performance optimization hint
      /// When true Ogre2 will update all derived transforms to their
      /// latest to get correct results.
      ///
      /// When false, that step is skipped. It is only safe to
      /// set it to false when nothing has changed since the last
      /// update (i.e. nothing moved, no new objects created).
      ///
      /// Ogre will assert if built in Debug mode if this value
      /// is set to false when it shouldn't be.
      ///
      /// See https://ogrecave.github.io/ogre-next/api/2.2/
      /// _ogre20_changes.html#AssersionCachedOutOfDate
      /// for more info
      /// \return A vector of intersection results
      public: virtual RayQueryResult ClosestPoint(
            bool _forceSceneUpdate = true) = 0;
    };
    }
  }
}
#endif
