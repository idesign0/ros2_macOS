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
#ifndef GZ_RENDERING_OGRE2_OGRE2GLOBALILLUMINATIONCIVCT_HH_
#define GZ_RENDERING_OGRE2_OGRE2GLOBALILLUMINATIONCIVCT_HH_

#include "gz/rendering/GlobalIlluminationCiVct.hh"

#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2Object.hh"

#include <memory>

namespace Ogre
{
  class HlmsPbs;
  class VctCascadeSetting;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declaration
    class Ogre2CiVctCascadePrivate;
    class Ogre2GlobalIlluminationCiVctPrivate;

    /// \brief Ogre2.x implementation of the CiVctCascade class
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2CiVctCascade :
      public virtual CiVctCascade
    {
      /// \brief Constructor
      public: explicit Ogre2CiVctCascade();

      public: ~Ogre2CiVctCascade() override;

      /// \brief Initializes the cascade
      /// \param[in] _cascade Cascade we control
      /// \param[in] _ref Reference to clone settings from (can be nullptr)
      public: void Init(Ogre::VctCascadeSetting *_cascade,
                        const CiVctCascade *_ref);

      /// \brief Re-initializes the cascade.
      /// See Ogre2GlobalIlluminationCiVct::Reset
      /// \param[in] _cascade Cascade we control. All data already filled.
      public: void ReInit(Ogre::VctCascadeSetting *_cascade);

      // Documentation inherited.
      public: virtual void SetCorrectAreaLightShadows(
            bool _correctAreaLightShadows) override;

      // Documentation inherited.
      public: virtual bool CorrectAreaLightShadows() const override;

      // Documentation inherited.
      public: virtual void SetAutoMultiplier(bool _autoMultiplier) override;

      // Documentation inherited.
      public: virtual bool AutoMultiplier() const override;

      // Documentation inherited.
      public: virtual void SetThinWallCounter(float _thinWallCounter) override;

      // Documentation inherited.
      public: virtual float ThinWallCounter() const override;

      // Documentation inherited.
      public: virtual void SetResolution(const uint32_t _resolution[3])
          override;

      // Documentation inherited.
      public: virtual const uint32_t* Resolution() const override;

      // Documentation inherited.
      public: virtual void SetOctantCount(const uint32_t _octants[3]) override;

      // Documentation inherited.
      public: virtual const uint32_t* OctantCount() const override;

      // Documentation inherited.
      public: virtual void SetAreaHalfSize(
            const gz::math::Vector3d &_areaHalfSize) override;

      // Documentation inherited.
      public: virtual gz::math::Vector3d AreaHalfSize() const override;

      // Documentation inherited.
      public: virtual void SetCameraStepSize(
            const gz::math::Vector3d &_stepSize) override;

      // Documentation inherited.
      public: virtual gz::math::Vector3d CameraStepSize() const override;

      /// \brief Pointer to private data class
      private: std::unique_ptr<Ogre2CiVctCascadePrivate> dataPtr;
    };

    /// \brief Ogre2.x implementation of the GlobalIlluminationCiVct class
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2GlobalIlluminationCiVct :
      public virtual GlobalIlluminationCiVct, public virtual Ogre2Object
    {
      /// \brief Constructor
      protected: Ogre2GlobalIlluminationCiVct();

      /// \brief Destructor
      public: virtual ~Ogre2GlobalIlluminationCiVct() override;

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
      public: virtual void SetMaxCascades(const uint32_t _maxCascades) override;

      // Documentation inherited
      public: virtual CiVctCascadePtr AddCascade(
            const CiVctCascade* _ref) override;

      // Documentation inherited
      public: virtual void PopCascade() override;

      // Documentation inherited
      public: virtual void AutoCalculateStepSizes(
            const gz::math::Vector3d &_stepSize) override;

      // Documentation inherited.
      public: virtual void SetConsistentCascadeSteps(
            bool _consistentCascadeSteps) override;

      // Documentation inherited.
      public: virtual bool ConsistentCascadeSteps() const override;

      // Documentation inherited.
      public: virtual bool Started() const override;

      // Documentation inherited
      public: virtual void Start(
            uint32_t _bounceCount, bool _anisotropic) override;

      // Documentation inherited
      public: virtual void NewSettings(
            uint32_t _bounceCount, bool _anisotropic) override;

      // Documentation inherited
      public: virtual void Reset() override;

      // Documentation inherited
      public: virtual void Bind(const CameraPtr &_camera) override;

      // Documentation inherited
      public: virtual bool Anisotropic() const override;

      // Documentation inherited
      public: virtual void SetHighQuality(bool _highQuality) override;

      // Documentation inherited
      public: virtual bool HighQuality() const override;

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

      // Documentation inherited.
      public: virtual void SetBounceCount(uint32_t _bounceCount) override;

      // Documentation inherited.
      public: virtual uint32_t BounceCount() const override;

      // Documentation inherited.
      public: void SetParticipatingVisuals(uint32_t _mask) override;

      // Documentation inherited.
      public: uint32_t ParticipatingVisuals() const override;

      /// \internal
      /// \brief Retrieves HlmsPbs
      private: Ogre::HlmsPbs* HlmsPbs() const;

      /// \internal
      /// \brief Syncs the current value of DebugVisualization with Ogre
      private: void SyncModeVisualizationMode();

      /// \brief Pointer to private data class
      private: std::unique_ptr<Ogre2GlobalIlluminationCiVctPrivate> dataPtr;

      /// \brief Make scene our friend so it can create ogre2 visuals
      private: friend class Ogre2Scene;
    };
    }
  }
}
#endif
