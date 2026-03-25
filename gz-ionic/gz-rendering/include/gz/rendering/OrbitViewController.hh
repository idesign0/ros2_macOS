/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_ORBITVIEWCONTROLLER_HH_
#define GZ_RENDERING_ORBITVIEWCONTROLLER_HH_

#include <memory>

#include <gz/utils/SuppressWarning.hh>

#include <gz/math/Vector3.hh>

#include "gz/rendering/config.hh"
#include "gz/rendering/Camera.hh"
#include "gz/rendering/ViewController.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    // forward declarations
    class OrbitViewControllerPrivate;

    /// \class OrbitViewController OrbitViewController.hh
    /// gz/rendering/OrbitViewController.hh
    /// \brief A camera view controller
    class GZ_RENDERING_VISIBLE OrbitViewController
        : public virtual ViewController
    {
      /// \brief Destructor
      public: OrbitViewController();

      /// \brief Destructor
      public: explicit OrbitViewController(const CameraPtr &_camera);

      /// \brief Destructor
      public: ~OrbitViewController() override;

      /// \brief Set the camera that will be controlled by this view controller.
      /// \param[in] _camera Camera to control
      public: void SetCamera(const CameraPtr &_camera) override;

      /// \brief Get the camera that is controlled by this view controller.
      /// \return Camera being controlled
      public: virtual CameraPtr Camera() const;

      /// \brief Set target point for pan, zoom, orbit
      /// \param[in] _target Target point in world coordinates
      public: void SetTarget(const math::Vector3d &_target) override;

      /// \brief Get target point for pan, zoom, orbit
      /// \return target point in world coordinates
      public: virtual const math::Vector3d &Target() const;

      /// \brief Set zoom amount
      /// \param[in] _value Camera zoom value, e.g. mouse scroll delta
      public: void Zoom(const double _value) override;

      /// \brief Set camera pan (translational movement) around target point.
      /// \param[in] _value Pan amount in image plane, e.g. mouse drag delta.
      public: void Pan(const math::Vector2d &_value) override;

      /// \brief Set camera orbit (rotational movement) around target point.
      /// \param[in] _value orbit amount in image plane, e.g. mouse drag delta
      public:  void Orbit(const math::Vector2d &_value) override;

      GZ_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
      /// \brief Private data pointer
      public: std::unique_ptr<OrbitViewControllerPrivate> dataPtr;
      GZ_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
    };
    }
  }
}
#endif
