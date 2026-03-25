/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_BASEFRUSTUMVISUAL_HH_
#define GZ_RENDERING_BASEFRUSTUMVISUAL_HH_

#include <array>

#include "gz/rendering/FrustumVisual.hh"
#include "gz/rendering/base/BaseObject.hh"
#include "gz/rendering/base/BaseRenderTypes.hh"
#include "gz/rendering/Scene.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    /// \brief Base implementation of a Frustum Visual
    template <class T>
    class BaseFrustumVisual :
      public virtual FrustumVisual,
      public virtual T
    {
      // Documentation inherited
      protected: BaseFrustumVisual();

      // Documentation inherited
      public: virtual ~BaseFrustumVisual();

      // Documentation inherited
      public: virtual void PreRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: virtual void Update() override;

      // Documentation inherited
      public: virtual void Init() override;

      // Documentation inherited
      public: virtual double NearClipPlane() const override;

      // Documentation inherited
      public: virtual void SetNearClipPlane(double _near) override;

      // Documentation inherited
      public: virtual double FarClipPlane() const override;

      // Documentation inherited
      public: virtual void SetFarClipPlane(double _far) override;

      // Documentation inherited
      public: virtual math::Angle HFOV() const override;

      // Documentation inherited
      public: virtual void SetHFOV(const math::Angle &_hfov) override;

      // Documentation inherited
      public: virtual double AspectRatio() const override;

      // Documentation inherited
      public: virtual void SetAspectRatio(double _aspectRatio) override;

      // Documentation inherited
      public: virtual gz::math::Planed Plane(
                  const FrustumVisualPlane _plane) const override;

      /// \brief Create predefined materials for lidar visual
      public: virtual void CreateMaterials();

      /// \brief near value
      protected: double nearClip = 0.0;

      /// \brief far value
      protected: double farClip = 1.0;

      /// \brief fov value
      protected: gz::math::Angle hfov = gz::math::Angle(0.78539);

      /// \brief aspect ratio value
      protected: double aspectRatio = 1.0;

      /// \brief array of plane
      protected: std::array<gz::math::Planed, 6> planes;

      /// \brief pose of visual
      protected: gz::math::Pose3d pose = gz::math::Pose3d::Zero;
    };

    /////////////////////////////////////////////////
    // BaseFrustumVisual
    /////////////////////////////////////////////////
    template <class T>
    BaseFrustumVisual<T>::BaseFrustumVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    BaseFrustumVisual<T>::~BaseFrustumVisual()
    {
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseFrustumVisual<T>::PreRender()
    {
      T::PreRender();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseFrustumVisual<T>::Destroy()
    {
      T::Destroy();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseFrustumVisual<T>::Update()
    {
      // no op
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseFrustumVisual<T>::Init()
    {
      T::Init();
      this->CreateMaterials();
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseFrustumVisual<T>::SetNearClipPlane(double _near)
    {
      this->nearClip = _near;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseFrustumVisual<T>::NearClipPlane() const
    {
      return this->nearClip;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseFrustumVisual<T>::SetFarClipPlane(double _far)
    {
      this->farClip = _far;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseFrustumVisual<T>::FarClipPlane() const
    {
      return this->farClip;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseFrustumVisual<T>::SetHFOV(
          const gz::math::Angle &_hfov)
    {
      this->hfov = _hfov;
    }

    /////////////////////////////////////////////////
    template <class T>
    gz::math::Angle BaseFrustumVisual<T>::HFOV() const
    {
      return this->hfov;
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseFrustumVisual<T>::SetAspectRatio(
          double _aspectRatio)
    {
      this->aspectRatio = _aspectRatio;
    }

    /////////////////////////////////////////////////
    template <class T>
    double BaseFrustumVisual<T>::AspectRatio() const
    {
      return this->aspectRatio;
    }

    /////////////////////////////////////////////////
    template <class T>
    gz::math::Planed BaseFrustumVisual<T>::Plane(
          const FrustumVisualPlane _plane) const
    {
      return this->planes[_plane];
    }

    /////////////////////////////////////////////////
    template <class T>
    void BaseFrustumVisual<T>::CreateMaterials()
    {
      MaterialPtr mtl;

      if (!this->Scene()->MaterialRegistered("Frustum/BlueRay"))
      {
        mtl = this->Scene()->CreateMaterial("Frustum/BlueRay");
        mtl->SetAmbient(0.0, 0.0, 1.0);
        mtl->SetDiffuse(0.0, 0.0, 1.0);
        mtl->SetEmissive(0.0, 0.0, 1.0);
        mtl->SetSpecular(0.0, 0.0, 1.0);
        mtl->SetTransparency(0.0);
        mtl->SetCastShadows(false);
        mtl->SetReceiveShadows(false);
        mtl->SetLightingEnabled(false);
        mtl->SetMetalness(0.1f);
        mtl->SetReflectivity(0.2);
      }
    }
    }
  }
}
#endif
