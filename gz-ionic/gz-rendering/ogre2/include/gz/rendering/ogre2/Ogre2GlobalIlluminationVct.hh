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
#ifndef GZ_RENDERING_OGRE2_OGRE2GLOBALILLUMINATIONVCT_HH_
#define GZ_RENDERING_OGRE2_OGRE2GLOBALILLUMINATIONVCT_HH_

#include "gz/rendering/base/BaseGlobalIlluminationVct.hh"

#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2Object.hh"

#include <memory>

namespace Ogre
{
  class HlmsPbs;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class Ogre2GlobalIlluminationVctPrivate;

    /// \brief Ogre2.x implementation of the GlobalIlluminationVct class
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2GlobalIlluminationVct :
      public BaseGlobalIlluminationVct<Ogre2Object>
    {
      /// \brief Constructor
      protected: Ogre2GlobalIlluminationVct();

      /// \brief Destructor
      public: virtual ~Ogre2GlobalIlluminationVct() override;

      // Documentation inherited
      protected: virtual void Init() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void Build() override;

      // Documentation inherited
      public: virtual void UpdateLighting() override;

      // Documentation inherited
      public: virtual void UpdateCamera() override;

      // Documentation inherited
      public: virtual void SetResolution(const uint32_t _resolution[3])
          override;

      // Documentation inherited
      public: virtual const uint32_t* Resolution() const override;

      // Documentation inherited.
      public: virtual void SetOctantCount(const uint32_t _octants[3]) override;

      // Documentation inherited.
      public: virtual const uint32_t* OctantCount() const override;

      // Documentation inherited
      public: virtual void SetBounceCount(uint32_t _bounceCount) override;

      // Documentation inherited
      public: virtual uint32_t BounceCount() const override;

      // Documentation inherited.
      public: void SetParticipatingVisuals(uint32_t _mask) override;

      // Documentation inherited.
      public: uint32_t ParticipatingVisuals() const override;

      // Documentation inherited
      public: virtual void SetHighQuality(bool _highQuality) override;

      // Documentation inherited
      public: virtual bool HighQuality() const override;

      // Documentation inherited
      public: virtual void SetAnisotropic(bool _anisotropic) override;

      // Documentation inherited
      public: virtual bool Anisotropic() const override;

      // Documentation inherited
      public: virtual void SetThinWallCounter(float _thinWallCounter) override;

      // Documentation inherited
      public: virtual float ThinWallCounter() const override;

      // Documentation inherited
      public: virtual void SetConserveMemory(bool _conserveMemory) override;

      // Documentation inherited
      public: virtual bool ConserveMemory() const override;

      /// \internal
      /// \brief Sets this GI solution as enabled
      /// \param _enabled[in] True to enable, false otherwise
      protected: virtual void SetEnabled(bool _enabled) override;

      // Documentation inherited.
      public: virtual bool Enabled() const override;

      // Documentation inherited.
      public: virtual void SetDebugVisualization(
            DebugVisualizationMode _dvm) override;

      // Documentation inherited.
      public: virtual DebugVisualizationMode DebugVisualization() const
          override;

      // Documentation inherited.
      public: virtual void LightingChanged() override;

      /// \internal
      /// \brief Retrieves HlmsPbs
      private: Ogre::HlmsPbs* HlmsPbs() const;

      /// \internal
      /// \brief Syncs the current value of DebugVisualization with Ogre
      private: void SyncModeVisualizationMode();

      /// \brief Pointer to private data class
      private: std::unique_ptr<Ogre2GlobalIlluminationVctPrivate> dataPtr;

      /// \brief Make scene our friend so it can create ogre2 visuals
      private: friend class Ogre2Scene;
    };
    }
  }
}
#endif
