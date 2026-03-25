/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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
#ifndef GZ_RENDERING_BASE_BASERENDERPASS_HH_
#define GZ_RENDERING_BASE_BASERENDERPASS_HH_

#include <string>
#include "gz/rendering/RenderPass.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /* \class BaseRenderPass BaseRenderPass.hh \
     * gz/rendering/base/BaseRenderPass.hh
     */
    /// \brief Base render pass that can be applied to a render target
    template <class T>
    class BaseRenderPass:
      public virtual RenderPass,
      public T
    {
      /// \brief Constructor
      protected: BaseRenderPass();

      /// \brief Destructor
      public: virtual ~BaseRenderPass();

      // Documentation inherited
      public: virtual void SetEnabled(bool _enabled) override;

      // Documentation inherited
      public: virtual bool IsEnabled() const override;

      // Documentation inherited
      public: virtual void PreRender(const CameraPtr &_camera) override;

      // Documentation inherited
      public: void SetWideAngleCameraAfterStitching(bool _afterStitching)
          override;

      // Documentation inherited
      public: bool WideAngleCameraAfterStitching() const  override;

      /// \brief Flag to indicate if render pass is enabled or not
      protected: bool enabled = true;

      /// \brief Flag tracking the current value of
      /// SetWideAngleCameraAfterStitching()
      protected: bool afterStitching = false;
    };

    //////////////////////////////////////////////////
    // BaseRenderPass
    //////////////////////////////////////////////////
    template <class T>
    BaseRenderPass<T>::BaseRenderPass()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    BaseRenderPass<T>::~BaseRenderPass()
    {
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderPass<T>::SetEnabled(bool _enabled)
    {
      this->enabled = _enabled;
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseRenderPass<T>::IsEnabled() const
    {
      return this->enabled;
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderPass<T>::PreRender(const CameraPtr &/*_camera*/)
    {
      T *thisT = this;
      thisT->PreRender();  // NOT the same as doing T::PreRender
    }

    //////////////////////////////////////////////////
    template <class T>
    void BaseRenderPass<T>::SetWideAngleCameraAfterStitching(
          bool _afterStitching)
    {
      this->afterStitching = _afterStitching;
    }

    //////////////////////////////////////////////////
    template <class T>
    bool BaseRenderPass<T>::WideAngleCameraAfterStitching() const
    {
      return this->afterStitching;
    }
    }
  }
}
#endif
