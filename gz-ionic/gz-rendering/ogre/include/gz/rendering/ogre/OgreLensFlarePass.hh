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
#ifndef GZ_RENDERING_OGRE_OGRELENSFLAREPASS_HH_
#define GZ_RENDERING_OGRE_OGRELENSFLAREPASS_HH_

#include <memory>

#include <gz/utils/ImplPtr.hh>

#include "gz/math/Vector3.hh"

#include "gz/rendering/base/BaseLensFlarePass.hh"
#include "gz/rendering/ogre/Export.hh"
#include "gz/rendering/ogre/OgreRenderPass.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \brief Ogre Implementation of a Lens Flare render pass.
    class GZ_RENDERING_OGRE_VISIBLE OgreLensFlarePass :
      public BaseLensFlarePass<OgreRenderPass>
    {
      /// \brief Constructor
      public: OgreLensFlarePass();

      /// \brief Destructor
      public: ~OgreLensFlarePass() override;

      // Documentation inherited
      public: void Init(ScenePtr _scene) override;

      // Documentation inherited
      public: void Destroy() override;

      // Documentation inherited
      public: void CreateRenderPass() override;

      // Documentation inherited
      public: void PreRender(const CameraPtr &_camera) override;

      // Documentation inherited
      public: void PostRender() override;

      // Documentation inherited
      public: void SetScale(double _scale) override;

      // Documentation inherited
      public: double Scale() const override;

      // Documentation inherited
      public: void SetColor(const math::Vector3d &_color) override;

      // Documentation inherited
      public: const math::Vector3d &Color() const override;

      // Documentation inherited
      public: void SetOcclusionSteps(uint32_t _occlusionSteps) override;

      // Documentation inherited
      public: uint32_t OcclusionSteps() const override;

      /// \brief Check to see if the lens flare is occluded and return a scaling
      /// factor that is proportional to the lens flare's visibility
      /// \remark OgreLensFlarePass::PreRender must have been called first.
      /// \param[in] _imgPos light pos in clip space
      /// \param[in] _faceIdx Face idx in range [0; 6)
      /// See RayQuery::SetFromCamera for what each value means
      /// This value is ignored if the camera is not WideAngleCamera
      private: double OcclusionScale(const math::Vector3d &_imgPos,
                                     uint32_t _faceIdx);

      /// \cond warning
      /// \brief Private data pointer
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
      /// \endcond

      private: friend class OgreLensFlareCompositorListenerPrivate;
    };
    }
  }
}
#endif
