/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_OGRE2_OGRE2PARTICLEEMITTER_HH_
#define GZ_RENDERING_OGRE2_OGRE2PARTICLEEMITTER_HH_

#include <memory>
#include <string>
#include "gz/rendering/base/BaseParticleEmitter.hh"
#include "gz/rendering/ogre2/Ogre2Visual.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // Forward declaration
    class Ogre2ParticleEmitterPrivate;

    /// \brief Class to manage a particle emitter.
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2ParticleEmitter :
      public BaseParticleEmitter<Ogre2Visual>
    {
      /// \brief Constructor
      protected: Ogre2ParticleEmitter();

      /// \brief Destructor
      public: virtual ~Ogre2ParticleEmitter();

      // Documentation inherited.
      public: void Destroy() override;

      // Documentation inherited.
      public: virtual void SetType(const EmitterType _type) override;

      // Documentation inherited.
      public: virtual void SetEmitterSize(
                  const gz::math::Vector3d &_size) override;

      // Documentation inherited.
      public: virtual void SetRate(double _rate) override;

      // Documentation inherited.
      public: virtual void SetDuration(double _duration) override;

      // Documentation inherited.
      public: virtual void SetEmitting(bool _enable) override;

      // Documentation inherited.
      public: virtual void SetParticleSize(
                  const gz::math::Vector3d &_size) override;

      // Documentation inherited.
      public: virtual void SetLifetime(double _lifetime) override;

      // Documentation inherited.
      public: virtual void SetMaterial(MaterialPtr _material,
                  bool _unique = true) override;

      // Documentation inherited.
      public: virtual void SetVelocityRange(double _minVelocity,
                                            double _maxVelocity) override;

      // Documentation inherited.
      public: virtual void SetColorRange(
                  const gz::math::Color &_colorStart,
                  const gz::math::Color &_colorEnd) override;

      // Documentation inherited.
      public: virtual void SetScaleRate(double _scaleRate) override;

      // Documentation inherited.
      public: virtual void SetColorRangeImage(const std::string &_image)
          override;

      // Documentation inherited.
      public: virtual void PreRender() override;

      /// \brief Particle system visibility flags
      public: static const uint32_t kParticleVisibilityFlags;

      // Documentation inherited.
      protected: virtual void Init() override;

      /// \brief Create the particle system
      private: void CreateParticleSystem();

      /// \brief Only the ogre scene can instantiate this class
      private: friend class Ogre2Scene;

      /// \brief Private data class
      private: std::unique_ptr<Ogre2ParticleEmitterPrivate> dataPtr;
    };
    }
  }
}
#endif
