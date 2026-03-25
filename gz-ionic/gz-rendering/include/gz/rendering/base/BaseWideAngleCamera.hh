/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_BASE_BASEWIDEANGLECAMERA_HH_
#define GZ_RENDERING_BASE_BASEWIDEANGLECAMERA_HH_

#include <string>

#include <gz/common/Event.hh>

#include "gz/rendering/base/BaseCamera.hh"
#include "gz/rendering/CameraLens.hh"
#include "gz/rendering/WideAngleCamera.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    template <class T>
    class BaseWideAngleCamera :
      public virtual WideAngleCamera,
      public virtual BaseCamera<T>,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseWideAngleCamera();

      /// \brief Destructor
      public: virtual ~BaseWideAngleCamera();

      // Documentation inherited.
      public: virtual void CreateWideAngleTexture();

      // Documentation inherited.
      public: virtual void SetLens(const CameraLens &_lens) override;

      // Documentation inherited.
      public: virtual const CameraLens &Lens() const override;

      // Documentation inherited.
      public: virtual math::Vector3d Project3d(const math::Vector3d &_pt) const
          override;

      // Documentation inherited.
      public: virtual common::ConnectionPtr ConnectNewWideAngleFrame(
          std::function<void(const unsigned char*, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) override;

      /// \brief Camera lens used by this wide angle camera
      protected: CameraLens lens;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseWideAngleCamera<T>::BaseWideAngleCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseWideAngleCamera<T>::~BaseWideAngleCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseWideAngleCamera<T>::SetLens(const CameraLens &_lens)
    {
      this->lens = _lens;
    }

    //////////////////////////////////////////////////
    template <class T>
    const CameraLens &BaseWideAngleCamera<T>::Lens() const
    {
      return this->lens;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseWideAngleCamera<T>::CreateWideAngleTexture()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    math::Vector3d BaseWideAngleCamera<T>:: Project3d(const math::Vector3d &)
        const
    {
      gzerr << "Project3d is not supported for "
             << "render engine: " << this->Scene()->Engine()->Name()
             << std::endl;
      return math::Vector3d();
    }

    //////////////////////////////////////////////////
    template <class T>
    common::ConnectionPtr BaseWideAngleCamera<T>::ConnectNewWideAngleFrame(
          std::function<void(const unsigned char *, unsigned int, unsigned int,
          unsigned int, const std::string &)>)
    {
      return nullptr;
    }
  }
  }
}
#endif
