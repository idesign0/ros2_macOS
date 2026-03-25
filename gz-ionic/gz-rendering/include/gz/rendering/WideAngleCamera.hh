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
#ifndef GZ_RENDERING_WIDEANGLECAMERA_HH_
#define GZ_RENDERING_WIDEANGLECAMERA_HH_

#include <string>

#include <gz/common/Event.hh>

#include "gz/rendering/Camera.hh"
#include "gz/rendering/CameraLens.hh"
#include "gz/rendering/Scene.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \class WideAngleCamera WideAngleCamera.hh
    ///     gz/rendering/WideAngleCamera.hh
    /// \brief Wide angle camera class
    class GZ_RENDERING_VISIBLE WideAngleCamera :
      public virtual Camera
    {
      /// \brief Destructor
      public: virtual ~WideAngleCamera();

      /// \brief Set the camera lens to use for this wide angle camera
      /// \param[in] _lens Camera lens to set
      public: virtual void SetLens(const CameraLens &_lens) = 0;

      /// \brief Get the camera lens used by this wide angle camera
      /// \return Camera lens set to this wide angle camera
      public: virtual const CameraLens &Lens() const = 0;

      /// \brief Project 3D world coordinates to screen coordinates
      /// \param[in] _pt 3D world coordinates
      /// \return Screen coordinates. Z is the distance of point from camera
      /// optical center.
      public: virtual math::Vector3d Project3d(const math::Vector3d &_pt) const
           = 0;

      /// \brief Subscribes a new listener to this camera's new frame event
      /// \param[in] _subscriber New camera listener callback
      public: virtual common::ConnectionPtr ConnectNewWideAngleFrame(
          std::function<void(const unsigned char *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) = 0;
    };
    }
  }
}
#endif
