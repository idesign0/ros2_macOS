/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_PROJECTOR_HH_
#define GZ_RENDERING_PROJECTOR_HH_

#include <string>

#include <gz/math/Angle.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/Export.hh"
#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/Visual.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \class Projector Projector.hh
    /// gz/rendering/Projector.hh
    //
    /// \brief A projector that projects a texture onto a surface
    class GZ_RENDERING_VISIBLE Projector :
      public virtual Visual
    {
      /// \brief Destructor
      public: virtual ~Projector();

      /// \brief Get the projector's far clipping plane distance
      /// \return Far clipping plane distance
      public: virtual double FarClipPlane() const = 0;

      /// \brief Set the projector's far clipping plane distance
      /// \param[in] _far Far clipping plane distance
      public: virtual void SetFarClipPlane(double _far) = 0;

      /// \brief Get the projector's near clipping plane distance
      /// \return Near clipping plane distance
      public: virtual double NearClipPlane() const = 0;

      /// \brief Set the projector's near clipping plane distance
      /// \param[in] _near Near clipping plane distance
      public: virtual void SetNearClipPlane(double _near) = 0;

      /// \brief Get the projector's horizontal field-of-view
      /// \return Angle containing the projector's horizontal field-of-view
      public: virtual math::Angle HFOV() const = 0;

      /// \brief Set the projector's horizontal field-of-view
      /// \param[in] _hfov Desired horizontal field-of-view
      public: virtual void SetHFOV(const math::Angle &_hfov) = 0;

      /// \brief Get the URI of the texture file used by the projector
      /// \return URI of the texture file
      public: virtual std::string Texture() const = 0;

      /// \brief Load a texture into the projector.
      /// \param[in] _texture Name of the texture to project.
      public: virtual void SetTexture(const std::string &_texture) = 0;

      /// \brief Get whether the projector is enabled or disabled.
      /// \return true if enabled, false otherwise
      public: virtual bool IsEnabled() const = 0;

      /// \brief Set whether the projector is enabled or disabled.
      /// \param[in] _enabled True to enable the projector.
      public: virtual void SetEnabled(bool _enabled) = 0;
    };
    }
  }
}
#endif
