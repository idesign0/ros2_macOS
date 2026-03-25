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

#ifdef __APPLE__
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
#else
#ifndef _WIN32
  #include <GL/gl.h>
  #include <GL/glext.h>

  // OGRE/RenderSystems/GL/GL/glext.h does not seem to have
  // this defined
  #ifndef GL_TEXTURE_CUBE_MAP_SEAMLESS
    #define GL_TEXTURE_CUBE_MAP_SEAMLESS 0x884F
  #endif
#endif
#endif

#include "gz/rendering/CameraLens.hh"

#include "gz/rendering/ogre/OgreWideAngleCamera.hh"

#include "gz/rendering/ogre/OgreConversions.hh"
#include "gz/rendering/ogre/OgreRenderEngine.hh"
#include "gz/rendering/ogre/OgreRenderPass.hh"
#include "gz/rendering/ogre/OgreRTShaderSystem.hh"

/// \brief Private data for the WideAngleCamera class
class gz::rendering::OgreWideAngleCamera::Implementation
{
  /// \brief Environment texture size
  public: int envTextureSize = 512u;

  /// \brief Compositor used to render rectangle with attached cube map
  public: Ogre::CompositorInstance *cubeMapCompInstance = nullptr;

  /// \brief Number of cameras used to create the cubemap
  public: static const unsigned int kEnvCameraCount = 6u;

  /// \brief A Set of 6 cameras,
  ///   each pointing in different direction with FOV of 90deg
  public: Ogre::Camera *envCameras[kEnvCameraCount];

  /// \brief Render targets for envCameras
  public: Ogre::RenderTarget *envRenderTargets[6];

  /// \brief Viewports for the render targets
  public: Ogre::Viewport *envViewports[6];

  /// \brief Pixel format for cube map texture
  public: Ogre::PixelFormat envCubeMapTextureFormat = Ogre::PF_BYTE_RGB;

  /// \brief A single cube map texture
  public: Ogre::Texture *envCubeMapTexture = nullptr;

  /// \brief Output texture
  public: Ogre::Texture *ogreRenderTexture = nullptr;

  /// \brief Pointer to material, used for second rendering pass
  public: Ogre::MaterialPtr compMat;

  /// \brief Camera lens description
  public: CameraLens lens;

  /// \brief Pointer to the ogre camera
  public: Ogre::Camera *ogreCamera = nullptr;

  /// \brief Dummy texture
  public: OgreRenderTexturePtr wideAngleTexture;

  /// \brief A chain of render passes applied to the render target
  public: std::vector<RenderPassPtr> renderPasses;

  /// \brief A chain of render passes applied to final stitched render target
  public: std::vector<RenderPassPtr> finalStitchRenderPasses;

  /// \brief Flag to indicate if render pass need to be rebuilt
  public: bool renderPassDirty = false;

  /// \brief The image buffer
  public: unsigned char *imageBuffer = nullptr;

  /// \brief Outgoing image data, used by newImageFrame event.
  public: unsigned char *wideAngleImage = nullptr;

  /// \brief Event used to signal camera data
  public: gz::common::EventT<void(const unsigned char *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newImageFrame;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreWideAngleCamera::OgreWideAngleCamera()
    : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
  for (unsigned int i = 0u; i < this->dataPtr->kEnvCameraCount; ++i)
  {
    this->dataPtr->envCameras[i] = nullptr;
    this->dataPtr->envRenderTargets[i] = nullptr;
    this->dataPtr->envViewports[i] = nullptr;
  }
}
//////////////////////////////////////////////////
OgreWideAngleCamera::~OgreWideAngleCamera()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::Init()
{
  BaseWideAngleCamera::Init();
  this->CreateCamera();
  this->CreateRenderTexture();
  this->Reset();
}

/////////////////////////////////////////////////
void OgreWideAngleCamera::CreateRenderTexture()
{
  this->DestroyRenderTexture();
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->wideAngleTexture =
      std::dynamic_pointer_cast<OgreRenderTexture>(base);
  this->dataPtr->wideAngleTexture->SetWidth(1);
  this->dataPtr->wideAngleTexture->SetHeight(1);
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::DestroyRenderTexture()
{
  if (this->dataPtr->wideAngleTexture)
  {
    dynamic_cast<OgreRenderTexture *>(this->dataPtr->wideAngleTexture.get())
      ->Destroy();
    this->dataPtr->wideAngleTexture.reset();
  }
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::PreRender()
{
  BaseCamera::PreRender();
  if (!this->dataPtr->ogreRenderTexture)
    this->CreateWideAngleTexture();

  this->UpdateRenderPassChain();

  for (auto &pass : this->dataPtr->renderPasses)
  {
    pass->PreRender(
      std::dynamic_pointer_cast<Camera>(this->shared_from_this()));
  }
  for (auto &pass : this->dataPtr->finalStitchRenderPasses)
  {
    pass->PreRender(
      std::dynamic_pointer_cast<Camera>(this->shared_from_this()));
  }
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::Destroy()
{
  if (!this->dataPtr->ogreCamera)
    return;

  this->RemoveAllRenderPasses();

  if (this->dataPtr->imageBuffer)
  {
    delete [] this->dataPtr->imageBuffer;
    this->dataPtr->imageBuffer = nullptr;
  }

  if (this->dataPtr->wideAngleImage)
  {
    delete [] this->dataPtr->wideAngleImage;
    this->dataPtr->wideAngleImage = nullptr;
  }

  if (this->dataPtr->wideAngleTexture)
  {
    this->dataPtr->wideAngleTexture->Destroy();
    this->dataPtr->wideAngleTexture = nullptr;
  }

  for (unsigned int i = 0u; i < this->dataPtr->kEnvCameraCount; ++i)
  {
    if (this->dataPtr->envRenderTargets[i])
    {
      OgreRTShaderSystem::DetachViewport(this->dataPtr->envViewports[i],
                                         this->scene);
      this->dataPtr->envRenderTargets[i]->removeAllViewports();
      this->dataPtr->envRenderTargets[i] = nullptr;
    }

    if (this->dataPtr->envCameras[i])
    {
      this->scene->OgreSceneManager()->destroyCamera(
          this->dataPtr->envCameras[i]->getName());
      this->dataPtr->envCameras[i] = nullptr;
    }
  }

  if (this->dataPtr->ogreCamera)
  {
    this->scene->OgreSceneManager()->destroyCamera(
        this->dataPtr->ogreCamera->getName());
    this->dataPtr->ogreCamera = nullptr;
  }

  if (this->dataPtr->envCubeMapTexture)
  {
    Ogre::TextureManager::getSingleton().remove(
      this->dataPtr->envCubeMapTexture->getName());
    this->dataPtr->envCubeMapTexture = nullptr;
  }

  if (this->dataPtr->ogreRenderTexture)
  {
    Ogre::TextureManager::getSingleton().remove(
      this->dataPtr->ogreRenderTexture->getName());
    this->dataPtr->ogreRenderTexture = nullptr;
  }

  if (!this->dataPtr->compMat.isNull())
  {
    Ogre::MaterialManager::getSingleton().remove(
        this->dataPtr->compMat->getName());
    this->dataPtr->compMat.setNull();
  }

  this->DestroyRenderTexture();
  BaseWideAngleCamera::Destroy();
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::AddRenderPass(const RenderPassPtr &_pass)
{
  // Do NOT pass it to super class.
  if (_pass->WideAngleCameraAfterStitching())
  {
    this->dataPtr->finalStitchRenderPasses.push_back(_pass);
  }
  else
  {
    this->dataPtr->renderPasses.push_back(_pass);
  }
  this->dataPtr->renderPassDirty = true;
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::RemoveRenderPass(const RenderPassPtr &_pass)
{
  // Do NOT pass it to super class.
  auto it = std::find(this->dataPtr->renderPasses.begin(),
                      this->dataPtr->renderPasses.end(), _pass);
  if (it != this->dataPtr->renderPasses.end())
  {
    (*it)->Destroy();
    this->dataPtr->renderPasses.erase(it);
    this->dataPtr->renderPassDirty = true;
  }
  else
  {
    it = std::find(this->dataPtr->finalStitchRenderPasses.begin(),
                   this->dataPtr->finalStitchRenderPasses.end(), _pass);
    if (it != this->dataPtr->finalStitchRenderPasses.end())
    {
      (*it)->Destroy();
      this->dataPtr->finalStitchRenderPasses.erase(it);
      this->dataPtr->renderPassDirty = true;
    }
    else
    {
      gzwarn << "OgreWideAngleCamera::RemoveRenderPass pass not found. This "
                "is fine if you called this function twice. But it may not be "
                "fine if you changed the value "
                "RenderPass::WideAngleCameraAfterStitching (see docs)"
             << std::endl;
    }
  }
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::RemoveAllRenderPasses()
{
  for (auto &pass : this->dataPtr->renderPasses)
  {
    pass->Destroy();
  }
  for (auto &pass : this->dataPtr->finalStitchRenderPasses)
  {
    pass->Destroy();
  }
  this->dataPtr->renderPasses.clear();
  this->dataPtr->finalStitchRenderPasses.clear();
  this->dataPtr->renderPassDirty = true;
}

//////////////////////////////////////////////////
unsigned int OgreWideAngleCamera::EnvTextureSize() const
{
  return this->dataPtr->envTextureSize;
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::SetEnvTextureSize(int _size)
{
  this->dataPtr->envTextureSize = _size;
}

/////////////////////////////////////////////////
void OgreWideAngleCamera::CreateCamera()
{
  // Create dummy ogre camera object
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    gzerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->dataPtr->ogreCamera = ogreSceneManager->createCamera(
      this->Name() + "_Camera");
  if (this->dataPtr->ogreCamera == nullptr)
  {
    gzerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  this->ogreNode->attachObject(this->dataPtr->ogreCamera);
  this->dataPtr->ogreCamera->setFixedYawAxis(false);
  this->dataPtr->ogreCamera->yaw(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->roll(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->setAutoAspectRatio(true);
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::CreateEnvCameras()
{
  double nearPlane = this->NearClipPlane();
  double farPlane = this->FarClipPlane();
  for (unsigned int i = 0u; i < this->dataPtr->kEnvCameraCount; ++i)
  {
    std::stringstream name_str;

    name_str << this->name << "_env_" << i;

    this->dataPtr->envCameras[i] =
        this->scene->OgreSceneManager()->createCamera(name_str.str());

    this->dataPtr->envCameras[i]->setFixedYawAxis(false);
    this->dataPtr->envCameras[i]->setFOVy(Ogre::Degree(90));
    this->dataPtr->envCameras[i]->setAspectRatio(1);

    this->dataPtr->envCameras[i]->yaw(Ogre::Degree(-90.0));
    this->dataPtr->envCameras[i]->roll(Ogre::Degree(-90.0));

    this->dataPtr->envCameras[i]->setNearClipDistance(nearPlane);
    this->dataPtr->envCameras[i]->setFarClipDistance(farPlane);
  }

  for (unsigned int i = 0u; i < this->dataPtr->kEnvCameraCount; ++i)
  {
    this->ogreNode->attachObject(this->dataPtr->envCameras[i]);
  }

  // set environment cameras orientation
  this->dataPtr->envCameras[0]->yaw(Ogre::Degree(-90));
  this->dataPtr->envCameras[1]->yaw(Ogre::Degree(90));
  this->dataPtr->envCameras[2]->pitch(Ogre::Degree(90));
  this->dataPtr->envCameras[3]->pitch(Ogre::Degree(-90));
  this->dataPtr->envCameras[5]->yaw(Ogre::Degree(180));
}


//////////////////////////////////////////////////
bool OgreWideAngleCamera::SetBackgroundColor(const math::Color &_color)
{
  bool retVal = true;
  Ogre::ColourValue clr = OgreConversions::Convert(_color);
  if (this->dataPtr->ogreCamera->getViewport())
  {
    this->dataPtr->ogreCamera->getViewport()->setBackgroundColour(clr);
    for (unsigned int i = 0u; i < this->dataPtr->kEnvCameraCount; ++i)
    {
      if (this->dataPtr->envViewports[i])
      {
        this->dataPtr->envViewports[i]->setBackgroundColour(clr);
      }
      else
      {
        retVal = false;
      }
    }
  }
  else
  {
    retVal = false;
  }
  return retVal;
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::CreateWideAngleTexture()
{
  if (this->dataPtr->ogreCamera == nullptr)
  {
    gzerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  if (!this->dataPtr->ogreRenderTexture)
  {
    this->dataPtr->ogreRenderTexture =
      Ogre::TextureManager::getSingleton().createManual(
      this->Name() + "_wideAngleCamera", "General", Ogre::TEX_TYPE_2D,
      this->ImageWidth(), this->ImageHeight(), 0,
      OgreConversions::Convert(this->ImageFormat()), Ogre::TU_RENDERTARGET,
      0, false, 0).get();
    Ogre::RenderTarget *rt =
        this->dataPtr->ogreRenderTexture->getBuffer()->getRenderTarget();
    rt->setAutoUpdated(false);
    Ogre::Viewport *vp = rt->addViewport(this->dataPtr->ogreCamera);
    vp->setClearEveryFrame(true);
    vp->setShadowsEnabled(false);
    vp->setOverlaysEnabled(false);
  }

  double ratio = static_cast<double>(this->ImageWidth()) /
                 static_cast<double>(this->ImageHeight());

  double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) / ratio);
  this->dataPtr->ogreCamera->setAspectRatio(ratio);
  // Setting the fov is likely not necessary for the ogreCamera but
  // clamp to max fov supported by ogre to avoid issues with building the
  // frustum
  this->dataPtr->ogreCamera->setFOVy(Ogre::Radian(
      Ogre::Real(std::clamp(vfov, 0.0, GZ_PI))));

  // create the env cameras and textures
  this->CreateEnvCameras();

  unsigned int fsaa = 0;
  std::vector<unsigned int> fsaaLevels =
      OgreRenderEngine::Instance()->FSAALevels();
  // check if target fsaa is supported
  unsigned int targetFSAA = this->antiAliasing;
  auto const it = std::find(fsaaLevels.begin(), fsaaLevels.end(), targetFSAA);
  if (it != fsaaLevels.end())
    fsaa = targetFSAA;
  else
  {
    // output warning but only do it once
    static bool ogreFSAAWarn = false;
    if (ogreFSAAWarn)
    {
      gzwarn << "Anti-aliasing level of '" << this->antiAliasing << "' "
              << "is not supported. Setting to 0" << std::endl;
      ogreFSAAWarn = true;
    }
  }

  this->dataPtr->envCubeMapTextureFormat =
      OgreConversions::Convert(this->ImageFormat());

  this->dataPtr->envCubeMapTexture =
      Ogre::TextureManager::getSingleton().createManual(
          this->name + "_env_tex",
          "General",
          Ogre::TEX_TYPE_CUBE_MAP,
          this->dataPtr->envTextureSize,
          this->dataPtr->envTextureSize,
          0,
          this->dataPtr->envCubeMapTextureFormat,
          Ogre::TU_RENDERTARGET,
          0,
          false,
          fsaa).getPointer();

  for (unsigned int i = 0u; i < this->dataPtr->kEnvCameraCount; ++i)
  {
    Ogre::RenderTarget *rtt;
    rtt = this->dataPtr->envCubeMapTexture->getBuffer(i)->getRenderTarget();
    rtt->setAutoUpdated(false);

    Ogre::Viewport *vp = rtt->addViewport(this->dataPtr->envCameras[i]);
    vp->setClearEveryFrame(true);
    vp->setShadowsEnabled(true);
    vp->setOverlaysEnabled(false);

    OgreRTShaderSystem::AttachViewport(vp, this->scene);

    auto const &bgColor = this->scene->BackgroundColor();
    vp->setBackgroundColour(OgreConversions::Convert(bgColor));
    vp->setVisibilityMask(this->VisibilityMask());

    this->dataPtr->envViewports[i] = vp;
    this->dataPtr->envRenderTargets[i] = rtt;
  }

  // create the compositor
  this->dataPtr->cubeMapCompInstance =
    Ogre::CompositorManager::getSingleton().addCompositor(
    this->dataPtr->ogreCamera->getViewport(),
    "WideCameraLensMap/ParametrisedMap");

  this->dataPtr->compMat =
      Ogre::MaterialManager::getSingleton().getByName("WideLensMap");

  auto pass = this->dataPtr->compMat->getTechnique(0)->getPass(0);

  if (!pass->getNumTextureUnitStates())
    pass->createTextureUnitState();
  this->dataPtr->cubeMapCompInstance->addListener(this);
  this->dataPtr->cubeMapCompInstance->setEnabled(true);
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::UpdateRenderPassChain()
{
  if (!this->dataPtr->renderPassDirty)
    return;

  for (auto &pass : this->dataPtr->renderPasses)
  {
    OgreRenderPass *ogreRenderPass =
        dynamic_cast<OgreRenderPass *>(pass.get());
    ogreRenderPass->SetCameras(this->dataPtr->envCameras);
    ogreRenderPass->CreateRenderPass();
  }
  for (auto &pass : this->dataPtr->finalStitchRenderPasses)
  {
    OgreRenderPass *ogreRenderPass =
        dynamic_cast<OgreRenderPass *>(pass.get());
    ogreRenderPass->SetCamera(this->dataPtr->ogreCamera);
    ogreRenderPass->CreateRenderPass();
  }
  this->dataPtr->renderPassDirty = false;
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::Render()
{
  for (unsigned int i = 0u; i < this->dataPtr->kEnvCameraCount; ++i)
  {
    this->dataPtr->envRenderTargets[i]->update();
  }

  this->dataPtr->compMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->
      setTextureName(this->dataPtr->envCubeMapTexture->getName());

  Ogre::RenderTarget *rt =
      this->dataPtr->ogreRenderTexture->getBuffer()->getRenderTarget();
  rt->setAutoUpdated(false);
  rt->update(false);
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::Copy(Image &_image) const
{
  const unsigned int width = this->ImageWidth();
  const unsigned int height = this->ImageHeight();

  if (_image.Width() != width || _image.Height() != height)
  {
    gzerr << "Invalid image dimensions" << std::endl;
    return;
  }

  void *data = _image.Data();
  Ogre::PixelFormat imageFormat = OgreConversions::Convert(_image.Format());
  Ogre::PixelBox ogrePixelBox(width, height, 1, imageFormat, data);

  Ogre::RenderTarget *rt =
    this->dataPtr->ogreRenderTexture->getBuffer()->getRenderTarget();
  rt->copyContentsToMemory(ogrePixelBox);
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::notifyMaterialRender(Ogre::uint32 /*_pass_id*/,
                                           Ogre::MaterialPtr &_material)
{
  if (_material.isNull())
    return;

  Ogre::Technique *pTechnique = _material->getBestTechnique();
  if (!pTechnique)
    return;

  Ogre::Pass *pPass = pTechnique->getPass(0);
  if (!pPass || !pPass->hasFragmentProgram())
    return;

  this->SetUniformVariables(pPass,
    this->AspectRatio(),
    this->HFOV().Radian());

#ifndef _WIN32
  // OGRE doesn't allow to enable cubemap filtering extention thru its API
  // suppose that this function was invoked in a thread that has OpenGL context
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif
}


//////////////////////////////////////////////////
void OgreWideAngleCamera::SetUniformVariables(Ogre::Pass *_pass,
    float _ratio, float _hfov)
{
  Ogre::GpuProgramParametersSharedPtr uniforms =
    _pass->getFragmentProgramParameters();

  uniforms->setNamedConstant("c1", static_cast<Ogre::Real>(
    this->Lens().C1()));
  uniforms->setNamedConstant("c2", static_cast<Ogre::Real>(
    this->Lens().C2()));
  uniforms->setNamedConstant("c3", static_cast<Ogre::Real>(
    this->Lens().C3()));

  if (this->Lens().ScaleToHFOV())
  {
    float param = (_hfov/2)/this->Lens().C2() +
        this->Lens().C3();
    float funRes = this->Lens().ApplyMappingFunction(
        static_cast<float>(param));

    float newF = 1.0f/(this->Lens().C1() * funRes);

    uniforms->setNamedConstant("f", static_cast<Ogre::Real>(newF));
  }
  else
  {
    uniforms->setNamedConstant("f",
        static_cast<Ogre::Real>(this->Lens().F()));
  }

  auto vecFun = this->Lens().MappingFunctionAsVector3d();

  uniforms->setNamedConstant("fun", Ogre::Vector3(
      vecFun.X(), vecFun.Y(), vecFun.Z()));

  uniforms->setNamedConstant("cutOffAngle",
    static_cast<Ogre::Real>(this->Lens().CutOffAngle()));

  Ogre::GpuProgramParametersSharedPtr uniformsVs =
    _pass->getVertexProgramParameters();

  uniformsVs->setNamedConstant("ratio", static_cast<Ogre::Real>(_ratio));
}

//////////////////////////////////////////////////
math::Vector3d OgreWideAngleCamera::Project3d(
    const math::Vector3d &_pt) const
{
  // project onto cubemap face then onto
  gz::math::Vector3d screenPos;
  // loop through all env cameras can find the one that sees the 3d world point
  for (unsigned int i = 0u; i < this->dataPtr->kEnvCameraCount; ++i)
  {
    // project world point to camera clip space.
    auto viewProj = this->dataPtr->envCameras[i]->getProjectionMatrix() *
        this->dataPtr->envCameras[i]->getViewMatrix();
    auto pos = viewProj * Ogre::Vector4(OgreConversions::Convert(_pt));
    pos.x /= pos.w;
    pos.y /= pos.w;
    // check if point is visible.
    // pos.z is in range [-w; w] so we check if it's > -w then
    // it means it's in front of us.
    if (std::fabs(pos.x) <= 1 && std::fabs(pos.y) <= 1 &&
        pos.z > -std::fabs(pos.w))
    {
      // determine dir vector to projected point from env camera
      // work in y up, z forward, x right clip space
      gz::math::Vector3d dir(pos.x, pos.y, 1);
      gz::math::Quaterniond rot = gz::math::Quaterniond::Identity;

      // rotate dir vector into wide angle camera frame based on the
      // face of the cube. Note: operate in clip space so
      // left handed coordinate system rotation
      if (i == 0)
        rot = gz::math::Quaterniond(0.0, GZ_PI * 0.5, 0.0);
      else if (i == 1)
        rot = gz::math::Quaterniond(0.0, -GZ_PI * 0.5, 0.0);
      else if (i == 2)
        rot = gz::math::Quaterniond(-GZ_PI * 0.5, 0.0, 0.0);
      else if (i == 3)
        rot = gz::math::Quaterniond(GZ_PI * 0.5, 0.0, 0.0);
      else if (i == 5)
        rot = gz::math::Quaterniond(0.0, GZ_PI, 0.0);
      dir = rot * dir;
      dir.Normalize();

      // compute theta and phi from the dir vector
      // theta is angle to dir vector from z (forward)
      // phi is angle from x in x-y plane
      // direction vector (x, y, z)
      // x = sin(theta)cos(phi)
      // y = sin(theta)sin(phi)
      // z = cos(theta)
      double theta =  std::atan2(
          std::sqrt(dir.X() * dir.X() + dir.Y() * dir.Y()), dir.Z());
      double phi = std::atan2(dir.Y(), dir.X());
      // this also works:
      // double theta = std::acos(dir.Z());
      // double phi = std::asin(dir.Y() / std::sin(theta));

      double f = this->Lens().F();
      double fov = this->HFOV().Radian();
      // recompute f if scale to HFOV is true
      if (this->Lens().ScaleToHFOV())
      {
        double param = (fov/2.0) / this->Lens().C2() + this->Lens().C3();
        double funRes = this->Lens().ApplyMappingFunction(
            static_cast<float>(param));
        f = 1.0 / (this->Lens().C1() * funRes);
      }

      // Apply fisheye lens mapping function
      // r is distance of point from image center
      double r = this->Lens().C1() * f *
          this->Lens().ApplyMappingFunction(
          theta/this->Lens().C2() + this->Lens().C3());

      // compute projected x and y in clip space
      double x = cos(phi) * r;
      double y = sin(phi) * r;

      unsigned int vpWidth =
          this->dataPtr->ogreCamera->getViewport()->getActualWidth();
      unsigned int vpHeight =
          this->dataPtr->ogreCamera->getViewport()->getActualHeight();
      // env cam cube map texture is square and likely to be different size from
      // viewport. We need to adjust projected pos based on aspect ratio
      double asp = static_cast<double>(vpWidth) /
        static_cast<double>(vpHeight);
      y *= asp;

      // convert to screen space
      screenPos.X() = ((x / 2.0) + 0.5) * vpWidth;
      screenPos.Y() = (1 - ((y / 2.0) + 0.5)) * vpHeight;

      // r will be > 1.0 if point is not visible (outside of image)
      screenPos.Z() = r;
      return screenPos;
    }
  }

  return screenPos;
}

//////////////////////////////////////////////////
std::vector<Ogre::Camera *> OgreWideAngleCamera::OgreEnvCameras() const
{
  return std::vector<Ogre::Camera *>(
    std::begin(this->dataPtr->envCameras), std::end(this->dataPtr->envCameras));
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::PostRender()
{
  if (this->dataPtr->newImageFrame.ConnectionCount() <= 0u)
    return;

  for (auto &pass : this->dataPtr->renderPasses)
  {
    pass->PostRender();
  }
  for (auto &pass : this->dataPtr->finalStitchRenderPasses)
  {
    pass->PostRender();
  }

  unsigned int width = this->ImageWidth();
  unsigned int height = this->ImageHeight();
  unsigned int len = width * height;

  PixelFormat format = this->ImageFormat();
  unsigned int channelCount = PixelUtil::ChannelCount(format);
  unsigned int bytesPerChannel = PixelUtil::BytesPerChannel(format);
  unsigned int bufferSize = len * channelCount * bytesPerChannel;
  if (!this->dataPtr->wideAngleImage)
    this->dataPtr->wideAngleImage = new unsigned char[bufferSize];
  if (!this->dataPtr->imageBuffer)
    this->dataPtr->imageBuffer = new unsigned char[bufferSize];

  // get image data
  Ogre::RenderTarget *rt =
      this->dataPtr->ogreRenderTexture->getBuffer()->getRenderTarget();
  Ogre::PixelBox ogrePixelBox(width, height, 1,
      OgreConversions::Convert(format), this->dataPtr->imageBuffer);
  rt->copyContentsToMemory(ogrePixelBox);

  // fill image data
  memcpy(this->dataPtr->wideAngleImage, this->dataPtr->imageBuffer,
      bufferSize);

  this->dataPtr->newImageFrame(
      this->dataPtr->wideAngleImage, width, height, channelCount,
      PixelUtil::Name(this->ImageFormat()));

  // Uncomment to debug wide angle cameraoutput
  // gzdbg << "wxh: " << width << " x " << height << std::endl;
  // for (unsigned int i = 0; i < height; ++i)
  // {
  //   for (unsigned int j = 0; j < width * channelCount; j += channelCount)
  //   {
  //     unsigned int idx = i * width * channelCount + j;
  //     unsigned int r = this->dataPtr->wideAngleImage[idx];
  //     unsigned int g = this->dataPtr->wideAngleImage[idx + 1];
  //     unsigned int b = this->dataPtr->wideAngleImage[idx + 2];
  //     std::cout << "[" << r << "," << g << "," << b << "]";
  //   }
  //   std::cout << std::endl;
  // }
}

//////////////////////////////////////////////////
common::ConnectionPtr OgreWideAngleCamera::ConnectNewWideAngleFrame(
    std::function<void(const unsigned char *, unsigned int, unsigned int,
      unsigned int, const std::string &)>  _subscriber)
{
  return this->dataPtr->newImageFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr OgreWideAngleCamera::RenderTarget() const
{
  return this->dataPtr->wideAngleTexture;
}

//////////////////////////////////////////////////
void OgreWideAngleCamera::SetVisibilityMask(uint32_t _mask)
{
  BaseCamera::SetVisibilityMask(_mask);
  if (this->dataPtr->envViewports[0] == nullptr)
    return;
  for (unsigned int i = 0u; i < this->dataPtr->kEnvCameraCount; ++i)
  {
    auto *vp = this->dataPtr->envViewports[i];
    vp->setVisibilityMask(_mask);
  }
}
