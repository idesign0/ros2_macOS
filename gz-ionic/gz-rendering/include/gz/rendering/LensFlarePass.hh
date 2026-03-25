/*
 * Copyright (C) 2022 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_LENSFLAREPASS_HH_
#define GZ_RENDERING_LENSFLAREPASS_HH_

#include <string>
#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"
#include "gz/rendering/RenderPass.hh"

#include "gz/math/Vector3.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \brief A render pass that applies Lens Flare to the render target
    class GZ_RENDERING_VISIBLE LensFlarePass
      : public virtual RenderPass
    {
      /// \brief Constructor
      public: LensFlarePass();

      /// \brief Destructor
      public: virtual ~LensFlarePass();

      /// \brief Initializes the Lens Flare Pass with given scene
      /// \param[in] _scene Pointer to scene
      public: virtual void Init(ScenePtr _scene) = 0;

      /// \brief Set the light that generates lens flare
      /// \param[in] _light Pointer to light
      public: virtual void SetLight(LightPtr _light) = 0;

      /// \brief Set the scale of lens flare.
      /// \param[in] _scale Scale of lens flare
      public: virtual void SetScale(double _scale) = 0;

      /// \brief Returns the scale set in SetScale()
      /// \return Scale of lens flare
      public: virtual double Scale() const = 0;

      /// \brief Set the color of lens flare.
      /// \param[in] _color Color of lens flare
      public: virtual void SetColor(const math::Vector3d &_color) = 0;

      /// \brief Returns the color set in SetColor()
      /// \return Color of lens flare
      public: virtual const math::Vector3d &Color() const = 0;

      /// \brief Set the number of steps to take in each direction when
      /// checking for occlusions.
      /// \param[in] _occlusionSteps number of steps to take in each direction
      /// when checking for occlusion. A value of 0 disables occlusion.
      public: virtual void SetOcclusionSteps(uint32_t _occlusionSteps) = 0;

      /// \brief Returns the number of steps set in SetOcclusionSteps()
      /// \return Number of occlusion steps
      public: virtual uint32_t OcclusionSteps() const = 0;
    };
    }
  }
}
#endif
