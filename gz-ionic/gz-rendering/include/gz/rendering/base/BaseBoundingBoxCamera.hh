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
#ifndef GZ_RENDERING_BASE_BASEBOUNDINGBOXCAMERA_HH_
#define GZ_RENDERING_BASE_BASEBOUNDINGBOXCAMERA_HH_

#include <vector>

#include <gz/common/Event.hh>
#include <gz/math/Color.hh>

#include "gz/rendering/base/BaseCamera.hh"
#include "gz/rendering/BoundingBoxCamera.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    template <class T>
    class BaseBoundingBoxCamera:
      public virtual BoundingBoxCamera,
      public virtual BaseCamera<T>,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseBoundingBoxCamera();

      /// \brief Destructor
      public: ~BaseBoundingBoxCamera() override;

      // Documentation inherited
      public: const std::vector<BoundingBox> &BoundingBoxData() const override;

      // Documentation inherited
      public: gz::common::ConnectionPtr ConnectNewBoundingBoxes(
        std::function<void(const std::vector<BoundingBox> &)> _subscriber)
        override = 0;

      // Documentation inherited
      public: void SetBoundingBoxType(BoundingBoxType _type) override;

      // Documentation inherited
      public: BoundingBoxType Type() const override;

      // Documentation inherited
      public: void DrawBoundingBox(unsigned char *_data,
        const math::Color &_color, const BoundingBox &_box) const override = 0;

      /// \brief The bounding box type
      protected: BoundingBoxType type = BoundingBoxType::BBT_FULLBOX2D;

      /// \brief The bounding box data
      protected: std::vector<BoundingBox> boundingBoxes;
    };

    //////////////////////////////////////////////////
    template <class T>
    BaseBoundingBoxCamera<T>::BaseBoundingBoxCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseBoundingBoxCamera<T>::~BaseBoundingBoxCamera()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    const std::vector<BoundingBox> &
    BaseBoundingBoxCamera<T>::BoundingBoxData() const
    {
      return this->boundingBoxes;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseBoundingBoxCamera<T>::SetBoundingBoxType(BoundingBoxType _type)
    {
      this->type = _type;
    }

    //////////////////////////////////////////////////
    template <class T>
    BoundingBoxType BaseBoundingBoxCamera<T>::Type() const
    {
      return this->type;
    }
    }
  }
}
#endif
