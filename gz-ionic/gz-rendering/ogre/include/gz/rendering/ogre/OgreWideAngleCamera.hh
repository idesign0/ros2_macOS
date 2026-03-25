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
#ifndef GZ_RENDERING_OGRE_WIDEANGLECAMERA_HH_
#define GZ_RENDERING_OGRE_WIDEANGLECAMERA_HH_

#include <memory>
#include <string>
#include <vector>

#include <gz/utils/ImplPtr.hh>

#include "gz/rendering/base/BaseWideAngleCamera.hh"
#include "gz/rendering/ogre/OgreRenderTarget.hh"
#include "gz/rendering/ogre/OgreRenderTypes.hh"
#include "gz/rendering/ogre/OgreScene.hh"
#include "gz/rendering/ogre/OgreSensor.hh"

namespace Ogre
{
  class Material;
  class RenderTarget;
  class Viewport;
}

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Ogre implementation of WideAngleCamera
    class GZ_RENDERING_OGRE_VISIBLE OgreWideAngleCamera :
        public BaseWideAngleCamera<OgreSensor>,
        protected Ogre::CompositorInstance::Listener
    {
      /// \brief Constructor
      protected: OgreWideAngleCamera();

      /// \brief Destructor
      public: virtual ~OgreWideAngleCamera();

      // Documentation inherited
      public: virtual void Init() override;

      /// \brief Create a texture
      public: virtual void CreateRenderTexture();

      /// \brief Destroy render texture created by CreateRenderTexture()
      /// Note: It's not virtual.
      protected: void DestroyRenderTexture();

      /// \brief Render the camera
      public: virtual void PostRender() override;

      // Documentation inherited
      public: virtual void Destroy() override;

      // Documentation inherited
      public: void AddRenderPass(const RenderPassPtr &_pass) override;

      // Documentation inherited
      public: void RemoveRenderPass(const RenderPassPtr &_pass) override;

      // Documentation inherited
      public: void RemoveAllRenderPasses() override;

      /// \brief Update render pass chain if changes were made
      protected: void UpdateRenderPassChain();

      /// \brief Gets the environment texture size
      /// \return Texture size
      public: unsigned int EnvTextureSize() const;

      /// \brief Sets environment texture size
      /// \param[in] _size Texture size
      public: void SetEnvTextureSize(int _size);

      /// \brief Set the background color of the camera
      /// \param[in] _color Color to set the background to
      /// \return True if the background color is set successfully,
      /// false otherwise
      public: bool SetBackgroundColor(const math::Color &_color);

      /// \brief Project 3D world coordinates to screen coordinates
      /// \param[in] _pt 3D world coodinates
      /// \return Screen coordinates. Z is the distance of point from camera
      /// optical center.
      public: math::Vector3d Project3d(const math::Vector3d &_pt) const
          override;

      /// \brief Get the list of ogre cameras used to create the cube map for
      /// generating the wide angle camera image
      /// \return A list of OGRE cameras
      public: std::vector<Ogre::Camera *> OgreEnvCameras() const;

      /// \brief Set uniform variables of a shader
      ///   for the provided material technique pass
      /// \param[in] _pass Ogre::Pass used for rendering
      /// \param[in] _ratio Frame aspect ratio
      /// \param[in] _hfov Horizontal field of view
      public: void SetUniformVariables(Ogre::Pass *_pass,
          float _ratio, float _hfov);

      // Documentation inherited.
      public: virtual void PreRender() override;

      /// \brief Implementation of the render call
      public: virtual void Render() override;

      // Documentation inherited.
      public: void Copy(Image &_image) const override;

      // Documentation inherited
      public: common::ConnectionPtr ConnectNewWideAngleFrame(
          std::function<void(const unsigned char *, unsigned int, unsigned int,
          unsigned int, const std::string &)>  _subscriber) override;

      // Documentation inherited.
      public: virtual void SetVisibilityMask(uint32_t _mask) override;

      /// \brief Set the camera's render target
      protected: void CreateWideAngleTexture() override;

      /// \brief Create the camera.
      protected: void CreateCamera();

      /// \brief Creates a set of 6 cameras pointing in different directions
      protected: void CreateEnvCameras();

      /// \brief Callback that is used to set mapping material uniform values,
      ///   implements Ogre::CompositorInstance::Listener interface
      /// \param[in] _pass_id Pass identifier
      /// \param[in] _material Material whose parameters should be set
      protected: void notifyMaterialRender(Ogre::uint32 _pass_id,
          Ogre::MaterialPtr &_material) override;

      /// \brief Get a pointer to the render target.
      /// \return Pointer to the render target
      protected: virtual RenderTargetPtr RenderTarget() const override;

      private: friend class OgreScene;

      /// \cond warning
      /// \brief Private data pointer
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
      /// \endcond
    };
    }
  }
}
#endif
