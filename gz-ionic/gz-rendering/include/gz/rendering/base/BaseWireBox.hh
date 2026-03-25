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
#ifndef GZ_RENDERING_BASE_WIREBOX_HH_
#define GZ_RENDERING_BASE_WIREBOX_HH_

#include <string>
#include "gz/rendering/WireBox.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Base implementation of a wireframe box.
    template <class T>
    class BaseWireBox :
      public virtual WireBox,
      public virtual T
    {
      /// \brief Constructor
      protected: BaseWireBox();

      /// \brief Destructor
      public: ~BaseWireBox() override;

      // Documentation inherited.
      public: void PreRender() override;

      // Documentation inherited.
      public: void Destroy() override;

      // Documentation inherited.
      public: void SetBox(const gz::math::AxisAlignedBox &_box) override;

      // Documentation inherited.
      public: gz::math::AxisAlignedBox Box() const override;

      // \brief Underlying axis aligned box that the wire box reflects.
      protected: gz::math::AxisAlignedBox box;

      /// \brief Flag to indicate WireBox properties have changed
      protected: bool wireBoxDirty = false;
    };

    //////////////////////////////////////////////////
    // BaseWireBox
    //////////////////////////////////////////////////
    template <class T>
    BaseWireBox<T>::BaseWireBox()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseWireBox<T>::~BaseWireBox()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseWireBox<T>::PreRender()
    {
      T::PreRender();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseWireBox<T>::Destroy()
    {
      T::Destroy();
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseWireBox<T>::SetBox(const gz::math::AxisAlignedBox &_box)
    {
      this->box = _box;
      this->wireBoxDirty = true;
    }

    //////////////////////////////////////////////////
    template <class T>
    gz::math::AxisAlignedBox BaseWireBox<T>::Box() const
    {
      return this->box;
    }
    }
  }
}
#endif
