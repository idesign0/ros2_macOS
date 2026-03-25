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
 *
*/
#ifndef GZ_RENDERING_FRUSTUMVISUAL_HH_
#define GZ_RENDERING_FRUSTUMVISUAL_HH_

#include <gz/math/Angle.hh>
#include <gz/math/Plane.hh>
#include <gz/math/Pose3.hh>
#include <gz/utils/ImplPtr.hh>
#include "gz/rendering/config.hh"
#include "gz/rendering/Visual.hh"
#include "gz/rendering/Object.hh"
#include "gz/rendering/RenderTypes.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Planes that define the boundaries of the frustum.
    enum GZ_RENDERING_VISIBLE FrustumVisualPlane
    {
      /// \brief Near plane
      FRUSTUM_PLANE_NEAR   = 0,

      /// \brief Far plane
      FRUSTUM_PLANE_FAR    = 1,

      /// \brief Left plane
      FRUSTUM_PLANE_LEFT   = 2,

      /// \brief Right plane
      FRUSTUM_PLANE_RIGHT  = 3,

      /// \brief Top plane
      FRUSTUM_PLANE_TOP    = 4,

      /// \brief Bottom plane
      FRUSTUM_PLANE_BOTTOM = 5
    };

    /// \brief Mathematical representation of a frustum and related functions.
    /// This is also known as a view frustum.
    class GZ_RENDERING_VISIBLE FrustumVisual : public virtual Visual
    {
      /// \brief Default constructor. With the following default values:
      ///
      /// * near: 0.0
      /// * far: 1.0
      /// * hfov: 0.78539 radians (45 degrees)
      /// * aspect ratio: 1.0
      /// * pose: Pose3d::Zero
      protected: FrustumVisual();

      /// \brief Destructor
      public: virtual ~FrustumVisual();

      /// \brief Update the Visual
      public: virtual void Update() = 0;

      /// \brief Get the near distance. This is the distance from the
      /// frustum's vertex to the closest plane.
      /// \return Near distance.
      /// \sa SetNearClipPlane
      public: virtual double NearClipPlane() const = 0;

      /// \brief Set the near distance. This is the distance from the
      /// frustum's vertex to the closest plane.
      /// \param[in] _near Near distance.
      /// \sa NearClipPlane
      public: virtual void SetNearClipPlane(double _near) = 0;

      /// \brief Get the far distance. This is the distance from the
      /// frustum's vertex to the farthest plane.
      /// \return Far distance.
      /// \sa SetFarClipPlane
      public: virtual double FarClipPlane() const = 0;

      /// \brief Set the far distance. This is the distance from the
      /// frustum's vertex to the farthest plane.
      /// \param[in] _far Far distance.
      /// \sa FarClipPlane
      public: virtual void SetFarClipPlane(double _far) = 0;

      /// \brief Get the horizontal field of view. The field of view is the
      /// angle between the frustum's vertex and the edges of the near or far
      /// plane. This value represents the horizontal angle.
      /// \return The field of view.
      /// \sa SetHFOV
      public: virtual gz::math::Angle HFOV() const = 0;

      /// \brief Set the horizontal field of view. The field of view is the
      /// angle between the frustum's vertex and the edges of the near or far
      /// plane. This value represents the horizontal angle.
      /// \param[in] _hfov The field of view.
      /// \sa HFOV
      public: virtual void SetHFOV(const gz::math::Angle &_hfov) = 0;

      /// \brief Get the aspect ratio, which is the width divided by height
      /// of the near or far planes.
      /// \return The frustum's aspect ratio.
      /// \sa SetAspectRatio
      public: virtual double AspectRatio() const = 0;

      /// \brief Set the aspect ratio, which is the width divided by height
      /// of the near or far planes.
      /// \param[in] _aspectRatio The frustum's aspect ratio.
      /// \sa AspectRatio
      public: virtual void SetAspectRatio(double _aspectRatio) = 0;

      /// \brief Get a plane of the frustum.
      /// \param[in] _plane The plane to return.
      /// \return Plane of the frustum.
      public: virtual gz::math::Planed Plane(
                  const FrustumVisualPlane _plane) const = 0;

      /// \brief Compute the planes of the frustum. This is called whenever
      /// a property of the frustum is changed.
      private: void ComputePlanes();
    };
    }
  }
}
#endif
