/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_RENDERPASS_HH_
#define GZ_RENDERING_RENDERPASS_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/Object.hh"
#include "gz/rendering/Export.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \class RenderPass RenderPass.hh gz/rendering/RenderPass.hh
    /// \brief A render pass can be added to a camera to affect how the scene
    /// is rendered. It can be used to add post-processing effects. Multiple
    /// passes can be chained together.
    class GZ_RENDERING_VISIBLE RenderPass
      : public virtual Object
    {
      /// \brief Destructor
      public: virtual ~RenderPass();

      /// \brief Set to enable or disable the render pass
      /// \param[in] _enabled True to enable the render pass, false to disable.
      public: virtual void SetEnabled(bool _enabled) = 0;

      /// \brief Get whether or not the render pass is enabled
      /// \return True if the render pass is enabled, false otherwise.
      public: virtual bool IsEnabled() const = 0;

      /// \brief See Object::PreRender. This function will call
      /// Object::PreRender but with the added bonus that it has access
      /// to the camera that is about to render
      /// \param[in] _camera Camera that is about to render
      public: virtual void PreRender(const CameraPtr &_camera) = 0;

      /// \brief WideAngleCamera renders to 6 faces; then stitches all 6
      /// into a final "fish-eye" lens result.
      ///
      /// This function controls whether the effect is applied to each 6
      /// faces individually; or during the "stitching" pass.
      ///
      /// The default setting depends on the RenderPass (e.g. LensFlare
      /// defaults to true)
      /// \remark This setting must not be changed after adding the RenderPass
      /// to a Camera.
      /// \param[in] _afterStitching True if it should be done after stitching,
      /// False if it should be done to each of the 6 faces separately.
      public: virtual void SetWideAngleCameraAfterStitching(
            bool _afterStitching) = 0;

      /// \brief See SetWideAngleCameraAfterStitching()
      /// \return The current value set by SetWideAngleCameraAfterStitching
      public: virtual bool WideAngleCameraAfterStitching() const = 0;

      // Documentation inherited
      // Use PreRender(const CameraPtr &) instead
      private: using Object::PreRender;
    };
    }
  }
}
#endif
