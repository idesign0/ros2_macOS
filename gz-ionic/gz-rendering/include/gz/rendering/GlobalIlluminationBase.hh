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
#ifndef GZ_RENDERING_GLOBALILLUMINATIONBASE_HH_
#define GZ_RENDERING_GLOBALILLUMINATIONBASE_HH_

#include <cstdint>

#include "gz/rendering/Export.hh"
#include "gz/rendering/config.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \class GlobalIlluminationBase GlobalIlluminationBase.hh
    /// gz/rendering/GlobalIlluminationBase.hh
    /// \brief There can be many global illumination solutions.
    ///
    /// This class is the base of them all for (most) shared settings
    class GZ_RENDERING_VISIBLE GlobalIlluminationBase
    {
      private: friend class Ogre2Scene;

      /// \brief Bitmask of which objects are considered by GI bounces.
      /// Note that *all* objects can receive indirect illumination.
      ///
      /// However often only static objects can reflect/bounce/cast
      /// indirect illumination
      public: enum ParticipatingVisualsFlags
      {
        /// \brief Dynamic objects can bounce GI (depending on the solution,
        /// this may be slow)
        DYNAMIC_VISUALS = 1u << 0u,

        /// \brief Static objects can bounce GI
        STATIC_VISUALS = 1u << 1u
      };

      /// \brief Constructor
      public: GlobalIlluminationBase();

      /// \brief Destructor
      public: virtual ~GlobalIlluminationBase();

      /// \brief Initialize the class
      protected: virtual void Init() = 0;

      /// \brief Destroy the class
      protected: virtual void Destroy() = 0;

      /// \brief Sets this GI solution as enabled
      /// \remarks Only one GI solution can be active at the same time
      /// \see Scene::SetActiveGlobalIllumination
      /// \param[in] _enabled True to enable, false otherwise
      protected: virtual void SetEnabled(bool _enabled) = 0;

      /// \brief Returns true if this GI configuration is currently
      /// enabled
      /// \return True if this GI solution is currently enabled
      public: virtual bool Enabled() const = 0;

      /// \brief Build the internal structures required for producing GI
      /// This function may take significant time
      public: virtual void Build() = 0;

      /// \brief Called by Scene on an active GlobalIlluminationBase whenever
      /// lighting changes. This implies Build has been called already
      public: virtual void UpdateLighting() = 0;

      /// \brief Called by Scene on an active GlobalIlluminationBase whenever
      /// first rendering happens for a new frame.
      /// This implies Build has been called already
      public: virtual void UpdateCamera() = 0;

      /// \brief Set the number of GI bounces. Very high number can
      /// cause a large performance impact
      /// \param[in] _bounceCount Value in range [0; inf)
      public: virtual void SetBounceCount(uint32_t _bounceCount) = 0;

      /// \brief Get number of bounces
      /// \return Number of bounces
      public: virtual uint32_t BounceCount() const = 0;

      /// \brief Set which visuals can bounce GI
      /// \param[in] _mask See ParticipatingVisualsFlags
      public: virtual void SetParticipatingVisuals(uint32_t _mask) = 0;

      /// \brief Visuals that can bounce GI
      /// \return See ParticipatingVisualsFlags
      public: virtual uint32_t ParticipatingVisuals() const = 0;
    };
    }
  }
}
#endif
