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

#ifndef GZ_RENDERING_BASE_BASEPROJECTOR_HH_
#define GZ_RENDERING_BASE_BASEPROJECTOR_HH_

#include <string>
#include "gz/rendering/base/BaseScene.hh"
#include "gz/rendering/base/BaseNode.hh"
#include "gz/rendering/Projector.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /* \class BaseProjector BaseProjector.hh \
     * gz/rendering/base/BaseProjector.hh
     */
    /// \brief A base implementation of the Projector class
    template <class T>
    class BaseProjector :
      public virtual Projector,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseProjector();

      /// \brief Destructor
      public: virtual ~BaseProjector();

      // Documentation inherited
      public: virtual double FarClipPlane() const override;

      // Documentation inherited
      public: virtual void SetFarClipPlane(double _far) override;

      // Documentation inherited
      public: virtual double NearClipPlane() const override;

      // Documentation inherited
      public: virtual void SetNearClipPlane(double _near) override;

      // Documentation inherited
      public: virtual math::Angle HFOV() const override;

      // Documentation inherited
      public: virtual void SetHFOV(const math::Angle &_hfov) override;

      // Documentation inherited
      public: virtual std::string Texture() const override;

      // Documentation inherited
      public: void SetTexture(const std::string &_texture) override;

      // Documentation inherited
      public: bool IsEnabled() const override;

      // Documentation inherited
      public: void SetEnabled(bool _enabled) override;

      /// \brief Projector's near clip plane
      protected: double nearClip = 0.1;

      /// \brief Projector's far clip plane
      protected: double farClip = 10.0;

      /// \brief Projector's horizontal field of view
      protected: math::Angle hfov = math::Angle(0.785398);

      /// \brief Texture used by the projector
      protected: std::string textureName;

      /// \brief Indicates whether or not the projector is enabled
      protected: bool enabled = false;

      /// \brief Only the scene can create a particle emitter
      private: friend class BaseScene;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseProjector<T>::BaseProjector()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseProjector<T>::~BaseProjector()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseProjector<T>::FarClipPlane() const
    {
      return this->farClip;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseProjector<T>::SetFarClipPlane(double _far)
    {
      this->farClip = _far;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseProjector<T>::NearClipPlane() const
    {
      return this->nearClip;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseProjector<T>::SetNearClipPlane(double _near)
    {
      this->nearClip = _near;
    }

    /////////////////////////////////////////////////
    template <class T>
    math::Angle BaseProjector<T>::HFOV() const
    {
      return this->hfov;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseProjector<T>::SetHFOV(const math::Angle &_hfov)
    {
      this->hfov = _hfov;
    }

    /////////////////////////////////////////////////
    template <class T>
    std::string BaseProjector<T>::Texture() const
    {
      return this->textureName;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseProjector<T>::SetTexture(const std::string &_texture)
    {
      this->textureName = _texture;
    }

    /////////////////////////////////////////////////
    template <class T>
    bool BaseProjector<T>::IsEnabled() const
    {
      return this->enabled;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseProjector<T>::SetEnabled(bool _enabled)
    {
      this->enabled = _enabled;
    }
    }
  }
}
#endif
