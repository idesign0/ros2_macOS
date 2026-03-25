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
#ifndef GZ_RENDERING_OGRE2_WIDEANGLECAMERA_HH_
#define GZ_RENDERING_OGRE2_WIDEANGLECAMERA_HH_

#include <memory>
#include <string>

#include <gz/utils/ImplPtr.hh>

#include "gz/rendering/base/BaseWideAngleCamera.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2RenderTarget.hh"
#include "gz/rendering/ogre2/Ogre2Sensor.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {
    //
    /// \brief Ogre implementation of WideAngleCamera
    class GZ_RENDERING_OGRE2_VISIBLE Ogre2WideAngleCamera :
        public BaseWideAngleCamera<Ogre2Sensor>
    {
      /// \brief Constructor
      protected: Ogre2WideAngleCamera();

      /// \brief Destructor
      public: virtual ~Ogre2WideAngleCamera() override;

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

      /// \brief Destroys all textures and workspaces, possibly
      /// for recreation.
      public: void DestroyTextures();

      // Documentation inherited
      public: void AddRenderPass(const RenderPassPtr &_pass) override;

      // Documentation inherited
      public: void RemoveRenderPass(const RenderPassPtr &_pass) override;

      // Documentation inherited
      public: void RemoveAllRenderPasses() override;

      /// \brief Gets the environment texture size
      /// \return Texture size
      public: uint32_t EnvTextureSize() const;

      /// \brief Sets environment texture size
      /// \param[in] _size Texture size
      public: void SetEnvTextureSize(uint32_t _size);

      /// \brief Project 3D world coordinates to screen coordinates
      /// \param[in] _pt 3D world coordinates
      /// \return Screen coordinates. Z is the distance of point from camera
      /// optical center.
      public: math::Vector3d Project3d(const math::Vector3d &_pt) const
          override;

      /// \brief It's the same as calling ogreCamera->getCameraToViewportRay
      /// but for the specific _faceIdx.
      /// \param _screenPos Screen space position
      /// \param _faceIdx Face index in range [0; 6).
      /// See RayQuery::SetFromCamera for what each value means
      /// \return Ogre Ray pointing towards screenPos from the given face idx
      public: Ogre::Ray CameraToViewportRay(const math::Vector2d &_screenPos,
                                            uint32_t _faceIdx);

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

      /// \brief Set the background material of this camera
      /// \param[in] _material Material to set the background to
      public: void SetBackgroundMaterial(MaterialPtr _material);

      /// \brief Get the background material of this camera
      /// \return background material
      public: MaterialPtr BackgroundMaterial() const;

      /// \brief Returns the workspace name for the final pass
      /// that stitches all faces.
      /// \return Workspace definition's name
      protected: std::string WorkspaceFinalPassDefinitionName() const;

      /// \brief Creates the workspace definition for stitch (final) pass,
      /// including effects.
      protected: void CreateStitchWorkspaceDefinition();

      /// \brief Creates the stitch workspaces & their definitions.
      protected: void CreateStitchWorkspace();

      /// \brief Destroys the stitch workspaces & their definitions.
      protected: void DestroyStitchWorkspace();

      /// \brief Returns true if we have a temp texture for Render passes
      /// with WideAngleCameraAfterStitching = true
      /// \return See description.
      protected: bool HasTempStitchTexture() const;

      /// \brief Returns what HasTempStitchTexture() should return
      /// \return True if there are RenderPases with
      /// WideAngleCameraAfterStitching = true. False otherwise.
      protected: bool NeedsTempStitchTexture() const;

      /// \brief Returns what HasTempStitchTexture() should return
      /// \return Returns which channel kStichTmpTexture should be in.
      /// Depends on whether number of render passes with
      /// RenderPass::WideAngleCameraAfterStitching == true is odd or even
      protected: uint32_t TempStitchTextureChannel() const;

      /// \brief Returns the workspace name. It's unique for each camera.
      /// \param[in] _faceIdx Face index in range [0; 6)
      /// \return Workspace definition's name
      protected: std::string WorkspaceDefinitionName(uint32_t _faceIdx) const;

      /// \brief Creates the workspace definition, including effects.
      /// \param[in] _withMsaa Whether the camera is using MSAA
      protected: void CreateWorkspaceDefinition(bool _withMsaa);

      /// \brief Creates the workspaces & their definitions.
      /// \param[in] _withMsaa Whether the version we're retrieving is the MSAA
      protected: void CreateFacesWorkspaces(bool _withMsaa);

      /// \brief Destroys the workspaces & their definitions.
      protected: void DestroyFacesWorkspaces();

      /// \brief Checks if workspace nodes need to be updated since
      /// render passes can be toggled at any point.
      protected: void UpdateRenderPasses();

      /// \brief Set the camera's render target
      protected: void CreateWideAngleTexture() override;

      /// \brief Create the camera.
      protected: void CreateCamera();

      /// \brief Get a pointer to the render target.
      /// \return Pointer to the render target
      protected: virtual RenderTargetPtr RenderTarget() const override;

      /// \brief Changes the Compositor Definition to use the MSAA
      /// settings we need. Do not call this if not using MSAA.
      /// \param[in] _ogreCompMgr Ogre's Compositor Manager
      /// \param[in] _msaa Value in range [2; 256)
      protected: void SetupMSAA(Ogre::CompositorManager2 *_ogreCompMgr,
                                uint8_t _msaa);

      /// \brief Update the background material
      private: void UpdateBackgroundMaterial();

      /// \brief Saves the CompositorPassSceneDef of each of the 6 passes
      /// defined in WideAngleCamera.compositor data file for later
      /// manipulation.
      /// \param[in] _ogreCompMgr Ogre's Compositor Manager
      /// \param[in] _withMsaa Whether the version we're retrieving is the MSAA
      /// one
      private: void RetrieveCubePassSceneDefs(
            Ogre::CompositorManager2 *_ogreCompMgr, bool _withMsaa);

      /// \brief Called before each cubemap face is about to be rendered
      /// \param[in] _pass Compositor pass to set its clear colour
      private: void PrepareForCubemapFacePass(Ogre::CompositorPass *_pass);

      /// \brief Called before the final pass is about to be rendered
      /// \param[in] _pass Material Pass to setup
      private: void PrepareForFinalPass(Ogre::Pass *_pass);

      /// \cond warning
      /// \brief Private data pointer
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
      /// \endcond

      private: friend class Ogre2Scene;
      private: friend class Ogre2WideAngleCameraWorkspaceListenerPrivate;
    };
    }
  }
}
#endif
