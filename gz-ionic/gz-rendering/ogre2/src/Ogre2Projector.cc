/*
 * Copyright (C) 2023 Open Source Robotics Foundation
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

#include <string>
#include <unordered_map>

#include <OgreCamera.h>
#include <OgreDecal.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreTextureGpuManager.h>

#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2DepthCamera.hh"
#include "gz/rendering/ogre2/Ogre2Projector.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/Utils.hh"

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {
//
/// \brief Helper class for checking visibility of projecter to a camera
class Ogre2ProjectorCameraListener: public Ogre::Camera::Listener
{
  /// \brief Constructor
  /// \param[in] _decal Pointer to the ogre decal (projected texture)
  public: Ogre2ProjectorCameraListener(Ogre::Decal *_decal);

  //// \brief Set the visibility flags for this projector
  /// \param[in] _flags Visibility flags to set
  public: void SetVisibilityFlags(uint32_t _flags);

  /// \brief Callback when a camara is about to be rendered
  /// \param[in] _cam Ogre camera pointer which is about to render
  private: virtual void cameraPreRenderScene(
    Ogre::Camera * _cam) override;

  /// \brief Callback when a camera is finisned being rendered
  /// \param[in] _cam Ogre camera pointer which has already render
  private: virtual void cameraPostRenderScene(
    Ogre::Camera * _cam) override;

  /// \brief Projector's visibility flags
  private: uint32_t visibilityFlags = 0u;

  /// \brief Pointer to the decal ogre scene node
  public: Ogre::SceneNode *decalNode{nullptr};

  /// \brief Decal - Texture projected onto a surface
  public: Ogre::Decal *decal{nullptr};
};
}
}
}

using namespace gz;
using namespace rendering;

/// \brief Private data for the Ogre2Projector class
class gz::rendering::Ogre2Projector::Implementation
{
  /// \brief The decal ogre scene node
  public: Ogre::SceneNode *decalNode{nullptr};

  /// \brief Decal diffuse texture
  public: Ogre::TextureGpu *textureDiff{nullptr};

  /// \brief Decal - Texture projected onto a surface
  public: Ogre::Decal *decal{nullptr};

  /// \brief Indicate whether the projector is initialized or not
  public: bool initialized{false};

  /// \brief A map of cameras (<Camera ptr, name>) that the listener has been
  /// added to
  public: std::unordered_map<Ogre::Camera *, Ogre::IdString>
      camerasWithListener;

  /// \brief Listener for togging projector visibility
  /// We are using a custom listener because Ogre::Decal's setVisibilityFlags
  /// does not seem to work
  public: std::unique_ptr<Ogre2ProjectorCameraListener> listener;
};

/////////////////////////////////////////////////
Ogre2Projector::Ogre2Projector()
    : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
}

/////////////////////////////////////////////////
Ogre2Projector::~Ogre2Projector()
{
  if (!this->scene->IsInitialized())
    return;

  this->SetEnabled(false);

  for (const auto &ogreCamIt : this->dataPtr->camerasWithListener)
  {
    Ogre::IdString camName = ogreCamIt.second;
    auto ogreCam = this->scene->OgreSceneManager()->findCameraNoThrow(camName);
    if (ogreCam)
      ogreCam->removeListener(this->dataPtr->listener.get());
  }
  this->dataPtr->camerasWithListener.clear();

  if (this->dataPtr->textureDiff)
  {
    auto engine = Ogre2RenderEngine::Instance();
    auto ogreRoot = engine->OgreRoot();
    auto textureGpuManager =
        ogreRoot->getRenderSystem()->getTextureGpuManager();
    textureGpuManager->destroyTexture(this->dataPtr->textureDiff);
    this->dataPtr->textureDiff = nullptr;
  }
  if (this->dataPtr->decal)
  {
    this->scene->OgreSceneManager()->destroyDecal(this->dataPtr->decal);
    this->dataPtr->decal = nullptr;
  }
  if (this->dataPtr->decalNode)
  {
    this->scene->OgreSceneManager()->destroySceneNode(this->dataPtr->decalNode);
    this->dataPtr->decalNode = nullptr;
  }
}

/////////////////////////////////////////////////
void Ogre2Projector::PreRender()
{
  if (!this->dataPtr->initialized)
  {
    this->CreateProjector();
    this->dataPtr->initialized = true;
    this->SetEnabled(true);
  }

  this->UpdateCameraListener();
}

/////////////////////////////////////////////////
void Ogre2Projector::UpdateCameraListener()
{
  // if a custom visibility flag is set, we will need to use a listener
  // for toggling the visibility of the decal
  if ((this->VisibilityFlags() & GZ_VISIBILITY_ALL) == GZ_VISIBILITY_ALL)
  {
    this->dataPtr->decalNode->setVisible(true);
    this->dataPtr->decalNode->getCreator()->setDecalsDiffuse(
        this->dataPtr->decal->getDiffuseTexture());
    this->dataPtr->decalNode->getCreator()->setDecalsEmissive(
        this->dataPtr->decal->getEmissiveTexture());

    for (const auto &ogreCamIt : this->dataPtr->camerasWithListener)
    {
      const Ogre::IdString &camName = ogreCamIt.second;
      // instead of getting the camera pointer through ogreCamIt.first,
      // find camera pointer again to make sure the camera still exists
      // because there is a chance that we are holding onto a dangling pointer
      // if that camera was deleted already
      auto ogreCam =
          this->scene->OgreSceneManager()->findCameraNoThrow(camName);
      ogreCam->removeListener(this->dataPtr->listener.get());
    }
    this->dataPtr->camerasWithListener.clear();
    return;
  }

  if (!this->dataPtr->listener)
  {
    this->dataPtr->listener = std::make_unique<Ogre2ProjectorCameraListener>(
        this->dataPtr->decal);
  }
  this->dataPtr->listener->SetVisibilityFlags(this->VisibilityFlags());
  this->dataPtr->decalNode->setVisible(false);

  // loop through color cameras and add listener to toggle visibility of
  // decals in these cameras
  for (unsigned int i = 0; i < this->scene->SensorCount(); ++i)
  {
    auto sensor = this->scene->SensorByIndex(i);
    Ogre2CameraPtr camera = std::dynamic_pointer_cast<Ogre2Camera>(sensor);
    if (camera)
    {
      auto ogreCam = camera->OgreCamera();
      if (this->dataPtr->camerasWithListener.find(ogreCam)
          == this->dataPtr->camerasWithListener.end())
      {
        ogreCam->addListener(this->dataPtr->listener.get());
        this->dataPtr->camerasWithListener[ogreCam] = ogreCam->getName();
      }
    }
    else
    {
      // depth camera can also generate rgb output (when simulating
      // RGBD cameras)
      Ogre2DepthCameraPtr depthCamera =
          std::dynamic_pointer_cast<Ogre2DepthCamera>(sensor);
      if (depthCamera)
      {
        auto ogreCam = depthCamera->OgreCamera();
        if (this->dataPtr->camerasWithListener.find(ogreCam)
            == this->dataPtr->camerasWithListener.end())
        {
          ogreCam->addListener(this->dataPtr->listener.get());
          this->dataPtr->camerasWithListener[ogreCam] = ogreCam->getName();
        }
      }
    }
  }
}

/////////////////////////////////////////////////
void Ogre2Projector::CreateProjector()
{
  this->dataPtr->decalNode = this->ogreNode->createChildSceneNode();
  this->dataPtr->decalNode->roll(Ogre::Degree(90));
  this->dataPtr->decalNode->yaw(Ogre::Degree(180));

  this->dataPtr->decal = this->scene->OgreSceneManager()->createDecal();
  this->dataPtr->decalNode->attachObject(this->dataPtr->decal);

  if (common::isFile(this->textureName))
  {
    std::string baseName = common::basename(this->textureName);
    size_t idx = this->textureName.rfind(baseName);
    if (idx != std::string::npos)
    {
      std::string dirPath = this->textureName.substr(0, idx);
      if (!dirPath.empty() &&
          !Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(
          dirPath))
      {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
            dirPath, "FileSystem", "General");
      }
    }
  }
  else
  {
    gzerr << "Unable to create projector. Projector texture not found: "
          << this->textureName << std::endl;
    return;
  }

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureManager =
    ogreRoot->getRenderSystem()->getTextureGpuManager();
  int decalDiffuseId = 1;
  this->dataPtr->textureDiff = textureManager->createOrRetrieveTexture(
      this->textureName, this->textureName + "_alias",
      Ogre::GpuPageOutStrategy::Discard,
      Ogre::CommonTextureTypes::Diffuse,
      Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
      decalDiffuseId);
  this->dataPtr->textureDiff->scheduleTransitionTo(
      Ogre::GpuResidency::Resident);

  this->dataPtr->decal->setDiffuseTexture(this->dataPtr->textureDiff);
  this->dataPtr->decal->setEmissiveTexture(this->dataPtr->textureDiff);

  // approximate frustum size
  common::Image image(this->textureName);
  const double aspectRatio = static_cast<double>(image.Width()) /
                             static_cast<double>(image.Height());
  const double vfov = 2.0 * atan(tan(this->hfov.Radian() / 2.0)
      / aspectRatio);

  // ogre2 uses screen space decal. Unfortunately this is different from
  // ogre 1.x. In ogre 1.x the decal projects out like a camera frustum
  // but in ogre 2.x the decal is a rectangular volume. See related question in
  // https://forums.ogre3d.org/viewtopic.php?t=95298
  // Here we are just computing the rectangular volume of the ogre 2.x decal
  // based on projector properties. It is essentially a bounding box of the
  // frustum. \todo(anyone) We should change the implementation to match
  // ogre 1.x
  const double depth = this->farClip - this->nearClip;
  const double width = 2 * (tan(this->hfov.Radian() / 2) * this->farClip);
  const double height = 2 * (tan(vfov / 2) * this->farClip);

  this->dataPtr->decalNode->setPosition(
      Ogre::Vector3(this->nearClip + depth * 0.5, 0, 0));
  this->dataPtr->decalNode->setScale(width, depth, height);
}

/////////////////////////////////////////////////
void Ogre2Projector::SetEnabled(bool _enabled)
{
  BaseProjector::SetEnabled(_enabled);
  this->SetVisible(_enabled);
}

//////////////////////////////////////////////////
void Ogre2ProjectorCameraListener::SetVisibilityFlags(uint32_t _flags)
{
  this->visibilityFlags = _flags;
}

//////////////////////////////////////////////////
Ogre2ProjectorCameraListener::Ogre2ProjectorCameraListener(
  Ogre::Decal *_decal)
{
  this->decal = _decal;
  this->decalNode = _decal->getParentSceneNode();;
}

//////////////////////////////////////////////////
void Ogre2ProjectorCameraListener::cameraPreRenderScene(
    Ogre::Camera *_cam)
{
  uint32_t mask = _cam->getLastViewport()->getVisibilityMask();
  if (this->visibilityFlags & mask && this->decalNode && this->decal)
  {
    this->decalNode->setVisible(true);
    this->decalNode->getCreator()->setDecalsDiffuse(
        this->decal->getDiffuseTexture());
    this->decalNode->getCreator()->setDecalsEmissive(
        this->decal->getEmissiveTexture());
  }
}

//////////////////////////////////////////////////
void Ogre2ProjectorCameraListener::cameraPostRenderScene(
    Ogre::Camera * /*_cam*/)
{
  if (this->decalNode)
    this->decalNode->setVisible(false);
}
