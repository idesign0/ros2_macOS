/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_BASE_BASELIGHT_HH_
#define GZ_RENDERING_BASE_BASELIGHT_HH_

#include "gz/rendering/Light.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    template <class T>
    class BaseLight :
      public virtual Light,
      public virtual T
    {
      protected: BaseLight();

      public: ~BaseLight() override;

      // Documentation inherited
      public: void SetDiffuseColor(double _r, double _g, double _b,
                  double _a = 1.0) override;

      // Documentation inherited.
      public: void SetDiffuseColor(const math::Color &_color) override = 0;

      // Documentation inherited.
      public: void SetSpecularColor(double _r, double _g, double _b,
                  double _a = 1.0) override;

      // Documentation inherited.
      public: void SetSpecularColor(const math::Color &_color) override = 0;

      // Documentation inherited.
      public: void SetAttenuationConstant(double _value) override = 0;

      // Documentation inherited.
      public: void SetAttenuationLinear(double _value) override = 0;

      // Documentation inherited.
      public: void SetAttenuationQuadratic(double _value) override = 0;

      // Documentation inherited.
      public: void SetAttenuationRange(double _range) override = 0;

      // Documentation inherited.
      public: void SetCastShadows(bool _castShadows) override = 0;

      // Documentation inherited.
      protected: virtual void Reset();
    };

    template <class T>
    class BaseDirectionalLight :
      public virtual DirectionalLight,
      public virtual T
    {
      protected: BaseDirectionalLight();

      public: ~BaseDirectionalLight() override;

      public: void SetDirection(double _x, double _y, double _z) override;

      public: void SetDirection(const math::Vector3d &_dir) override = 0;

      protected: void Reset() override;
    };

    template <class T>
    class BasePointLight :
      public virtual PointLight,
      public virtual T
    {
      protected: BasePointLight();

      public: virtual ~BasePointLight();
    };

    template <class T>
    class  BaseSpotLight :
      public virtual SpotLight,
      public virtual T
    {
      protected: BaseSpotLight();

      public: ~BaseSpotLight() override;

      public: void SetDirection(double _x, double _y, double _z) override;

      public: void SetDirection(const math::Vector3d &_dir) override = 0;

      public: void SetInnerAngle(double _radians) override;

      public: void SetInnerAngle(const math::Angle &_angle) override = 0;

      public: void SetOuterAngle(double _radians) override;

      public: void SetOuterAngle(const math::Angle &_angle) override = 0;

      public: void SetFalloff(double _falloff) override = 0;

      protected: void Reset() override;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseLight<T>::BaseLight()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseLight<T>::~BaseLight()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseLight<T>::SetDiffuseColor(double _r, double _g, double _b,
        double _a)
    {
      this->SetDiffuseColor(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseLight<T>::SetSpecularColor(double _r, double _g, double _b,
        double _a)
    {
      this->SetSpecularColor(math::Color(_r, _g, _b, _a));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseLight<T>::Reset()
    {
      this->SetDiffuseColor(math::Color::White);
      this->SetSpecularColor(math::Color::White);
      this->SetAttenuationConstant(1);
      this->SetAttenuationLinear(0);
      this->SetAttenuationQuadratic(0);
      this->SetAttenuationRange(100);
      this->SetCastShadows(true);
      this->SetIntensity(1.0);
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseDirectionalLight<T>::BaseDirectionalLight()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseDirectionalLight<T>::~BaseDirectionalLight()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseDirectionalLight<T>::SetDirection(double _x, double _y, double _z)
    {
      this->SetDirection(math::Vector3d(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseDirectionalLight<T>::Reset()
    {
      T::Reset();
      this->SetDirection(0, 0, -1);
    }

    //////////////////////////////////////////////////
    template <class T>
    BasePointLight<T>::BasePointLight()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BasePointLight<T>::~BasePointLight()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseSpotLight<T>::BaseSpotLight()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseSpotLight<T>::~BaseSpotLight()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSpotLight<T>::SetDirection(double _x, double _y, double _z)
    {
      this->SetDirection(math::Vector3d(_x, _y, _z));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSpotLight<T>::SetInnerAngle(double _radians)
    {
      this->SetInnerAngle(math::Angle(_radians));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSpotLight<T>::SetOuterAngle(double _radians)
    {
      this->SetOuterAngle(math::Angle(_radians));
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseSpotLight<T>::Reset()
    {
      T::Reset();
      this->SetDirection(0, 0, -1);
      this->SetInnerAngle(GZ_PI / 4.5);
      this->SetOuterAngle(GZ_PI / 4.0);
      this->SetFalloff(1.0);
    }
    }
  }
}
#endif
