/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <variant>

#include <gz/math/Vector2.hh>
#include <gz/math/Vector3.hh>

#include <gz/common/Console.hh>
#include <gz/math/Helpers.hh>

#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2GpuRays.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2Heightmap.hh"
#include "gz/rendering/ogre2/Ogre2ParticleEmitter.hh"
#include "gz/rendering/ogre2/Ogre2RenderTarget.hh"
#include "gz/rendering/ogre2/Ogre2RenderTypes.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Sensor.hh"
#include "gz/rendering/ogre2/Ogre2Visual.hh"

#include "Ogre2GzHlmsSphericalClipMinDistance.hh"
#include "Ogre2ParticleNoiseListener.hh"
#include "Terra/Hlms/PbsListener/OgreHlmsPbsTerraShadows.h"

#include "Terra/Terra.h"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuad.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <OgreDepthBuffer.h>
#include <OgreItem.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {
//
/// \brief Helper class for switching the ogre item's material to laser retro
/// source material when a thermal camera is being rendered.
class GZ_RENDERING_OGRE2_HIDDEN
    Ogre2LaserRetroMaterialSwitcher : public Ogre::CompositorWorkspaceListener
{
  /// \brief constructor
  /// \param[in] _scene the scene manager responsible for rendering
  /// \param[in] _gpuRays owner of this class
  /// \param[in] _ogreCamera camera to obtain projection settings from
  public: explicit Ogre2LaserRetroMaterialSwitcher(Ogre2ScenePtr _scene,
                                                   Ogre2GpuRays *_gpuRays,
                                                   Ogre::Camera *_ogreCamera);

  /// \brief destructor
  public: virtual ~Ogre2LaserRetroMaterialSwitcher() = default;

  /// \brief Called when each pass is about to be executed.
  /// \param[in] _pass Ogre pass which is about to execute
  private: virtual void passPreExecute(
      Ogre::CompositorPass *_pass) override;

  /// \brief Callback when each pass is finished executing.
  /// \param[in] _pass Ogre pass which has already executed
  private: virtual void passPosExecute(
      Ogre::CompositorPass *_pass) override;

  /// \brief Scene manager
  private: Ogre2ScenePtr scene = nullptr;

  /// \brief Ogre2GpuRays, our creator
  private: Ogre2GpuRays *gpuRays = nullptr;

  /// \brief Ogre Camera. We do not own this ptr
  private: Ogre::Camera *ogreCamera = nullptr;

  /// \brief A map of ogre datablock pointer to their original blendblocks
  private: std::unordered_map<Ogre::HlmsDatablock *,
      const Ogre::HlmsBlendblock *> datablockMap;

  /// \brief A map of ogre sub item pointer to their original low level
  /// material.
  /// Most objects don't use one so it should be almost always empty.
  private:
    std::vector<std::pair<Ogre::SubItem *, Ogre::MaterialPtr>> materialMap;
};
}
}
}


/// \internal
/// \brief Private data for the Ogre2GpuRays class
class GZ_RENDERING_OGRE2_HIDDEN gz::rendering::Ogre2GpuRaysPrivate
{
  /// \brief Event triggered when new gpu rays range data are available.
  /// \param[in] _frame New frame containing raw gpu rays data.
  /// \param[in] _width Width of frame.
  /// \param[in] _height Height of frame.
  /// \param[in] _channel Number of channels
  /// \param[in] _format Format of frame.
  public: gz::common::EventT<void(const float *,
               unsigned int, unsigned int, unsigned int,
               const std::string &)> newGpuRaysFrame;

  /// \brief Outgoing gpu rays data, used by newGpuRaysFrame event.
  public: float *gpuRaysScan = nullptr;

  /// \brief Cubemap camera
  public: Ogre::Camera *cubeCam{nullptr};

  /// \brief Texture packed with cubemap face and uv data
  public: Ogre::TextureGpu *cubeUVTexture = nullptr;

  /// \brief Temporary texture where to render a side of the cubemap
  /// during GpuRays1stPass. Shared across all active faces to save memory
  public: Ogre::TextureGpu *colorTexture = nullptr;

  /// \brief Temporary texture where to render a side of the cubemap
  /// during GpuRays1stPass. Shared across all active faces to save memory
  public: Ogre::TextureGpu *depthTexture = nullptr;

  /// \brief Temporary texture where to render a side of the cubemap
  /// during GpuRays1stPass. Shared across all active faces to save memory
  public: Ogre::TextureGpu *particleTexture = nullptr;

  /// \brief Temporary texture where to render a side of the cubemap
  /// during GpuRays1stPass. Shared across all active faces to save memory
  public: Ogre::TextureGpu *particleDepthTexture = nullptr;

  /// \brief Set of cubemap faces that are needed to generate the final
  /// range data
  public: std::set<unsigned int> cubeFaceIdx;

  /// \brief Main pass definition (used for visibility mask manipulation).
  public: Ogre::CompositorPassSceneDef *mainPassSceneDef = nullptr;

  /// \brief 1st pass compositor workspace. One for each cubemap camera
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace1st[6];

  /// \brief 2nd pass compositor workspace.
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace2nd = nullptr;

  /// \brief An array of first pass textures. One for each cubemap camera.
  public: Ogre::TextureGpu * firstPassTextures[6];

  /// \brief Second pass texture.
  public: Ogre::TextureGpu * secondPassTexture = nullptr;

  /// \brief Pointer to the ogre camera
  public: Ogre::Camera *ogreCamera = nullptr;

  /// \brief Image width of first pass.
  public: unsigned int w1st = 0u;

  /// \brief Image height of first pass.
  public: unsigned int h1st = 0u;

  /// \brief Image width of second pass.
  public: unsigned int w2nd = 0u;

  /// \brief Image height of second pass.
  public: unsigned int h2nd = 0u;

  /// \brief Dummy render texture for the gpu rays
  public: RenderTexturePtr renderTexture;

  /// \brief Pointer to material switcher
  public: std::unique_ptr<Ogre2LaserRetroMaterialSwitcher>
      laserRetroMaterialSwitcher[6];

  /// \brief Near clip plane for cube camera
  public: float nearClipCube = 0.0;

  /// \brief Min allowed angle in radians;
  public: const math::Angle kMinAllowedAngle = 1e-4;

  /// \brief Max number of cameras used for creating the cubemap of depth
  /// textures for generating lidar data
  public: const unsigned int kCubeCameraCount = 6;

  /// \brief Execution mask for this workspace
  /// If particles exist in the scene, the execution mask of the particle
  /// target will be updated to match the workspace's execution mask so that
  /// these passes are executed, otherwise they will be skipped for performance
  /// improvement.
  public: const uint8_t kGpuRaysExecutionMask = 0xEF;

  /// \brief Pointer to the particle target definition in the workspace
  public: Ogre::CompositorTargetDef *particleTargetDef{nullptr};
};

using namespace gz;
using namespace rendering;

// Arbitrary values
static const uint32_t kLaserRetroMainDepthPassId = 9525u;
static const uint32_t kLaserRetro1stPassQuad = 9526u;

/// \brief standard deviation of particle noise
static const double kParticleStddev = 0.01;

//////////////////////////////////////////////////
Ogre2LaserRetroMaterialSwitcher::Ogre2LaserRetroMaterialSwitcher(
  Ogre2ScenePtr _scene, Ogre2GpuRays *_gpuRays, Ogre::Camera *_ogreCamera)
{
  this->scene = _scene;
  this->gpuRays = _gpuRays;
  this->ogreCamera = _ogreCamera;
}

//////////////////////////////////////////////////
void Ogre2LaserRetroMaterialSwitcher::passPreExecute(
  Ogre::CompositorPass *_pass)
{
  if(_pass->getDefinition()->mIdentifier == kLaserRetro1stPassQuad)
  {
    GZ_ASSERT(dynamic_cast<Ogre::CompositorPassQuad *>(_pass),
              "Impossible! Corrupted memory?");
    Ogre::CompositorPassQuad *passQuad =
      static_cast<Ogre::CompositorPassQuad *>(_pass);

    Ogre::Pass *pass = passQuad->getPass();
    Ogre::GpuProgramParametersSharedPtr psParams =
      pass->getFragmentProgramParameters();

    const double nearPlane = this->gpuRays->NearClipPlane();
    const double farPlane = this->gpuRays->FarClipPlane();

    // Set the uniform variables (see gpu_rays_1st_pass_fs.glsl).
    // The projectParams is used to linearize depth buffer data
    // The other params are used to clamp the range output
    Ogre::Vector2 projectionAB = this->ogreCamera->getProjectionParamsAB();
    double projectionA = projectionAB.x;
    double projectionB = projectionAB.y;
    projectionB /= farPlane;
    psParams->setNamedConstant("projectionParams",
                               Ogre::Vector2(static_cast<float>(projectionA),
                                             static_cast<float>(projectionB)));
    psParams->setNamedConstant("near",
                               static_cast<float>(nearPlane));
    psParams->setNamedConstant("far",
                               static_cast<float>(farPlane));
    psParams->setNamedConstant("max",
                               static_cast<float>(this->gpuRays->dataMaxVal));
    psParams->setNamedConstant("min",
                               static_cast<float>(this->gpuRays->dataMinVal));
    psParams->setNamedConstant("particleStddev",
                               static_cast<float>(kParticleStddev));

    Ogre2ParticleNoiseListener::SetupMaterial(pass, this->scene,
                                              passQuad->getCamera());
    return;
  }

  if(_pass->getDefinition()->mIdentifier != kLaserRetroMainDepthPassId)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  engine->SetGzOgreRenderingMode(GORM_SOLID_COLOR);

  this->materialMap.clear();
  this->datablockMap.clear();
  Ogre::HlmsManager *hlmsManager = engine->OgreRoot()->getHlmsManager();

  Ogre::HlmsDatablock *defaultPbs =
    hlmsManager->getHlms(Ogre::HLMS_PBS)->getDefaultDatablock();

  // Construct one now so that datablock->setBlendblock
  // each is as fast as possible
  const Ogre::HlmsBlendblock *noBlend =
    hlmsManager->getBlendblock(Ogre::HlmsBlendblock());

  const std::string laserRetroKey = "laser_retro";

  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);

    float retroValue = 0.0f;

    // get visual
    Ogre::Any userAny = item->getUserObjectBindings().getUserAny();
    if (!userAny.isEmpty() && userAny.getType() == typeid(unsigned int))
    {
      VisualPtr result;
      try
      {
        result = this->scene->VisualById(Ogre::any_cast<unsigned int>(userAny));
      }
      catch(Ogre::Exception &e)
      {
        gzerr << "Ogre Error:" << e.getFullDescription() << "\n";
      }
      Ogre2VisualPtr ogreVisual =
          std::dynamic_pointer_cast<Ogre2Visual>(result);

      if (ogreVisual->HasUserData(laserRetroKey))
      {
        // get laser_retro
        Variant tempLaserRetro = ogreVisual->UserData(laserRetroKey);
        if (const float* floatPtr = std::get_if<float>(&tempLaserRetro))
        {
          retroValue = *floatPtr;
        }
        else if (const double* doublePtr = std::get_if<double>(&tempLaserRetro))
        {
          retroValue = static_cast<float>(*doublePtr);
        }
        else if (const int* intPtr = std::get_if<int>(&tempLaserRetro))
        {
          retroValue = static_cast<float>(*intPtr);
        }
        else
        {
          gzerr << "Error casting user data: laser_retro\n";
        }
      }

      // only accept positive laser retro value
      retroValue = std::max(retroValue, 0.0f);
    }

    const size_t numSubItems = item->getNumSubItems();
    for (size_t i = 0; i < numSubItems; ++i)
    {
      Ogre::SubItem *subItem = item->getSubItem(i);

      // limit laser retro value to 2000 (as in gazebo)
      if (retroValue > 2000.0f)
      {
        retroValue = 2000.0f;
      }
      float color = retroValue / 2000.0f;
      subItem->setCustomParameter(1u,
                                  Ogre::Vector4(color, color, color, 1.0));

      if (!subItem->getMaterial().isNull())
      {
        this->materialMap.push_back({ subItem, subItem->getMaterial() });

        // We need to keep the material's vertex shader
        // to keep vertex deformation consistent; so we use
        // a cloned material with a different pixel shader
        // https://github.com/gazebosim/gz-rendering/issues/544
        //
        // material may be a nullptr if we called setMaterial directly
        // (i.e. it's not using Ogre2Material interface).
        // In those cases we fallback to PBS in the current GORM mode.
        auto material = Ogre::MaterialManager::getSingleton().getByName(
          subItem->getMaterial()->getName() + "_solid",
          Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (material)
        {
          if (material->getLoadingState() == Ogre::Resource::LOADSTATE_UNLOADED)
          {
            // Manually defined materials like PointCloudPoint_solid need this
            material->load();
          }

          if (material->getNumSupportedTechniques() > 0u)
          {
            subItem->setMaterial(material);
          }
        }
        else
        {
          // The supplied vertex shader could not pair with the
          // pixel shader we provide. Try to salvage the situation
          // using PBS shader. Custom deformation won't work but
          // if we're lucky that won't matter
          subItem->setDatablock(defaultPbs);
        }
      }
      else
      {
        // regular Pbs Hlms datablock
        Ogre::HlmsDatablock *datablock = subItem->getDatablock();
        const Ogre::HlmsBlendblock *blendblock = datablock->getBlendblock();

        // We can't do any sort of blending. This isn't colour what we're
        // storing, but rather an ID.
        if (blendblock->mSourceBlendFactor != Ogre::SBF_ONE ||
            blendblock->mDestBlendFactor != Ogre::SBF_ZERO ||
            blendblock->mBlendOperation != Ogre::SBO_ADD ||
            (blendblock->mSeparateBlend &&
             (blendblock->mSourceBlendFactorAlpha != Ogre::SBF_ONE ||
              blendblock->mDestBlendFactorAlpha != Ogre::SBF_ZERO ||
              blendblock->mBlendOperationAlpha != Ogre::SBO_ADD)))
        {
          hlmsManager->addReference(blendblock);
          this->datablockMap[datablock] = blendblock;
          datablock->setBlendblock(noBlend);
        }
      }
    }
    itor.moveNext();
  }

  // Do the same with heightmaps / terrain
  auto heightmaps = this->scene->Heightmaps();
  for (auto h : heightmaps)
  {
    auto heightmap = h.lock();
    if (heightmap)
    {
      float retroValue = 0.0f;

      // get visual
      VisualPtr visual = heightmap->Parent();

      if (visual->HasUserData(laserRetroKey))
      {
        // get laser_retro
        Variant tempLaserRetro = visual->UserData(laserRetroKey);

        if (const float* floatPtr = std::get_if<float>(&tempLaserRetro))
        {
          retroValue = *floatPtr;
        }
        else if (const double* doublePtr = std::get_if<double>(&tempLaserRetro))
        {
          retroValue = static_cast<float>(*doublePtr);
        }
        else if (const int* intPtr = std::get_if<int>(&tempLaserRetro))
        {
          retroValue = static_cast<float>(*intPtr);
        }
        else
        {
          gzerr << "Error casting user data: laser_retro\n";
        }
      }

      // only accept positive laser retro value
      retroValue = std::max(retroValue, 0.0f);

      // limit laser retro value to 2000 (as in gazebo)
      if (retroValue > 2000.0f)
      {
        retroValue = 2000.0f;
      }
      float color = retroValue / 2000.0f;

      // TODO(anyone): Retrieve datablock and make sure it's not blending
      // like we do with Items (it should be impossible?)
      const Ogre::Vector4 customParameter =
        Ogre::Vector4(color, color, color, 1.0);
      heightmap->Terra()->SetSolidColor(1u, customParameter);
    }
  }

  // Remove the reference count on noBlend we created
  hlmsManager->destroyBlendblock(noBlend);
}

//////////////////////////////////////////////////
void Ogre2LaserRetroMaterialSwitcher::passPosExecute(
  Ogre::CompositorPass *_pass)
{
  if(_pass->getDefinition()->mIdentifier != kLaserRetroMainDepthPassId)
    return;

  auto engine = Ogre2RenderEngine::Instance();
  Ogre::HlmsManager *hlmsManager = engine->OgreRoot()->getHlmsManager();

  // Restore original blending to modified materials
  for (const auto &[datablock, blendblock] : this->datablockMap)
  {
    datablock->setBlendblock(blendblock);
    // Remove the reference we added (this won't actually destroy it)
    hlmsManager->destroyBlendblock(blendblock);
  }
  this->datablockMap.clear();

  // Remove the custom parameter. Why? If there are multiple cameras that
  // use GORM_SOLID_COLOR (or any other mode), we want them to throw if
  // that code forgot to call setCustomParameter. We may miss those errors
  // if that code forgets to call but it was already carrying the value
  // we set here.
  //
  // This consumes more performance but it's the price to pay for
  // safety.
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);
    const size_t numSubItems = item->getNumSubItems();
    for (size_t i = 0; i < numSubItems; ++i)
    {
      Ogre::SubItem *subItem = item->getSubItem(i);
      subItem->removeCustomParameter(1u);
    }
    itor.moveNext();
  }

  // Restore Items with low level materials
  for (auto subItemMat : this->materialMap)
  {
    subItemMat.first->setMaterial(subItemMat.second);
  }
  this->materialMap.clear();

  // Remove the custom parameter (same reason as with Items)
  auto heightmaps = this->scene->Heightmaps();
  for (auto h : heightmaps)
  {
    auto heightmap = h.lock();
    if (heightmap)
      heightmap->Terra()->UnsetSolidColors();
  }

  engine->SetGzOgreRenderingMode(GORM_NORMAL);
}

//////////////////////////////////////////////////
Ogre2GpuRays::Ogre2GpuRays()
  : dataPtr(new Ogre2GpuRaysPrivate)
{
  // r = depth, g = retro, and b = n/a
  this->channels = 3u;

  for (unsigned int i = 0; i < this->dataPtr->kCubeCameraCount; ++i)
  {
    this->dataPtr->ogreCompositorWorkspace1st[i] = nullptr;
    this->dataPtr->laserRetroMaterialSwitcher[i] = nullptr;
    this->dataPtr->firstPassTextures[i] = nullptr;
  }
}

//////////////////////////////////////////////////
Ogre2GpuRays::~Ogre2GpuRays()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2GpuRays::Init()
{
  BaseGpuRays::Init();

  // create internal camera
  this->CreateCamera();

  // create dummy render texture
  this->CreateRenderTexture();
}

//////////////////////////////////////////////////
void Ogre2GpuRays::Destroy()
{
  if (!this->dataPtr->ogreCamera)
    return;

  if (this->dataPtr->gpuRaysScan)
  {
    delete [] this->dataPtr->gpuRaysScan;
    this->dataPtr->gpuRaysScan = nullptr;
  }

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  auto textureGpuManager = ogreRoot->getRenderSystem()->getTextureGpuManager();

  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // remove 1st pass textures, material, compositors
  for (auto i : this->dataPtr->cubeFaceIdx)
  {
    if (this->dataPtr->ogreCompositorWorkspace1st[i])
    {
      ogreCompMgr->removeWorkspace(
          this->dataPtr->ogreCompositorWorkspace1st[i]);
      this->dataPtr->ogreCompositorWorkspace1st[i] = nullptr;
    }
    if (this->dataPtr->firstPassTextures[i])
    {
      textureGpuManager->destroyTexture(this->dataPtr->firstPassTextures[i]);
      this->dataPtr->firstPassTextures[i] = nullptr;
    }
  }
  this->dataPtr->particleTargetDef = nullptr;

  // remove 2nd pass texture, material, compositor
  if (this->dataPtr->secondPassTexture)
  {
    textureGpuManager->destroyTexture(this->dataPtr->secondPassTexture);
    this->dataPtr->secondPassTexture = nullptr;
  }

  if (this->dataPtr->ogreCompositorWorkspace2nd)
  {
    ogreCompMgr->removeWorkspace(this->dataPtr->ogreCompositorWorkspace2nd);
    this->dataPtr->ogreCompositorWorkspace2nd = nullptr;
  }

  if (this->dataPtr->cubeUVTexture)
  {
    textureGpuManager->destroyTexture(this->dataPtr->cubeUVTexture);
    this->dataPtr->cubeUVTexture = nullptr;
  }
  if (this->dataPtr->colorTexture)
  {
    textureGpuManager->destroyTexture(this->dataPtr->colorTexture);
    this->dataPtr->colorTexture = nullptr;
  }
  if (this->dataPtr->depthTexture)
  {
    textureGpuManager->destroyTexture(this->dataPtr->depthTexture);
    this->dataPtr->depthTexture = nullptr;
  }
  if (this->dataPtr->particleTexture)
  {
    textureGpuManager->destroyTexture(this->dataPtr->particleTexture);
    this->dataPtr->particleTexture = nullptr;
  }
  if (this->dataPtr->particleDepthTexture)
  {
    textureGpuManager->destroyTexture(this->dataPtr->particleDepthTexture);
    this->dataPtr->particleDepthTexture = nullptr;
  }

  if (this->scene)
  {
    Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
    if (ogreSceneManager)
    {
      ogreSceneManager->destroyCamera(this->dataPtr->ogreCamera);
      this->dataPtr->ogreCamera = nullptr;

      if (this->dataPtr->cubeCam)
      {
        ogreSceneManager->destroyCamera(this->dataPtr->cubeCam);
        this->dataPtr->cubeCam = nullptr;
      }
    }
  }

  // call base node destroy to remove parent
  Ogre2Node::Destroy();
}

/////////////////////////////////////////////////
void Ogre2GpuRays::CreateRenderTexture()
{
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->renderTexture =
      std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->renderTexture->SetWidth(1);
  this->dataPtr->renderTexture->SetHeight(1);
}

/////////////////////////////////////////////////
void Ogre2GpuRays::CreateCamera()
{
  // Create ogre camera object
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

  // by default, ogre2 cameras are attached to root scene node
  this->dataPtr->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->dataPtr->ogreCamera);
  this->dataPtr->ogreCamera->setFixedYawAxis(false);
  this->dataPtr->ogreCamera->yaw(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->roll(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->setAutoAspectRatio(true);
}

/////////////////////////////////////////////////
void Ogre2GpuRays::ConfigureCamera()
{
  // horizontal gpu rays setup
  auto hfovAngle = this->AngleMax() - this->AngleMin();
  hfovAngle = std::max(this->dataPtr->kMinAllowedAngle, hfovAngle);
  this->SetHFOV(hfovAngle);

  // vertical laser setup
  double vfovAngle;

  if (this->VerticalRangeCount() > 1)
  {
    vfovAngle = std::max(this->dataPtr->kMinAllowedAngle.Radian(),
        (this->VerticalAngleMax() - this->VerticalAngleMin()).Radian());
  }
  else
  {
    vfovAngle = 0;

    if (this->VerticalAngleMax() != this->VerticalAngleMin())
    {
      gzwarn << "Only one vertical ray but vertical min. and max. angle "
          "are not equal. Min. angle is used.\n";
      this->SetVerticalAngleMax(this->VerticalAngleMin().Radian());
    }
  }
  this->SetVFOV(vfovAngle);

  // Configure first pass texture size
  // Each cubemap texture covers 90 deg FOV so determine number of samples
  // within the view for both horizontal and vertical FOV
  unsigned int hs = (hfovAngle.Radian() < GZ_PI_2) ? this->RangeCount() :
    static_cast<unsigned int>(
    GZ_PI_2 / hfovAngle.Radian() * this->RangeCount());

  unsigned int vs = (vfovAngle < GZ_PI_2) ? this->VerticalRangeCount() :
    static_cast<unsigned int>(
    GZ_PI_2 / vfovAngle * this->VerticalRangeCount());

  // get the max number from the two
  unsigned int v = std::max(hs, vs);
  // round to next highest power of 2
  // https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;

  // This is needed for large fov with low sample count,
  // e.g. 360 degrees and only 4 samples. Otherwise the depth data returned are
  // inaccurate.
  // \todo(anyone) For small fov, we shouldn't need such a high min texture size
  // requirement, e.g. a single ray lidar only needs 1x1 texture. However,
  // using lower res textures also give inaccurate results. Look for ways
  // to compute the optimal min texture size
  unsigned int min1stPassSamples = 128u;

  // limit max texture size to 1024
  unsigned int max1stPassSamples = 1024u;
  unsigned int samples1stPass =
      std::clamp(v, min1stPassSamples, max1stPassSamples);

  this->Set1stTextureSize(samples1stPass, samples1stPass);

  // Configure second pass texture size
  this->SetRangeCount(this->RangeCount(), this->VerticalRangeCount());

  // Set ogre cam properties
  this->dataPtr->ogreCamera->setNearClipDistance(this->dataPtr->nearClipCube);
  this->dataPtr->ogreCamera->setFarClipDistance(this->FarClipPlane());
}

/////////////////////////////////////////////////////////
math::Vector2d Ogre2GpuRays::SampleCubemap(const math::Vector3d &_v,
    unsigned int &_faceIndex)
{
  math::Vector3d vAbs = _v.Abs();
  double ma;
  math::Vector2d uv;
  if (vAbs.Z() >= vAbs.X() && vAbs.Z() >= vAbs.Y())
  {
    _faceIndex = _v.Z() < 0.0 ? 5.0 : 4.0;
    ma = 0.5 / vAbs.Z();
    uv = math::Vector2d(_v.Z() < 0.0 ? -_v.X() : _v.X(), -_v.Y());
  }
  else if (vAbs.Y() >= vAbs.X())
  {
    _faceIndex = _v.Y() < 0.0 ? 3.0 : 2.0;
    ma = 0.5 / vAbs.Y();
    uv = math::Vector2d(_v.X(), _v.Y() < 0.0 ? -_v.Z() : _v.Z());
  }
  else
  {
    _faceIndex = _v.X() < 0.0 ? 1.0 : 0.0;
    ma = 0.5 / vAbs.X();
    uv = math::Vector2d(_v.X() < 0.0 ? _v.Z() : -_v.Z(), -_v.Y());
  }
  return uv * ma + 0.5;
}

/////////////////////////////////////////////////////////
void Ogre2GpuRays::CreateSampleTexture()
{
  double min = this->AngleMin().Radian();
  double max = this->AngleMax().Radian();
  double vmin = this->VerticalAngleMin().Radian();
  double vmax = this->VerticalAngleMax().Radian();

  double hAngle = std::max(this->dataPtr->kMinAllowedAngle.Radian(), max - min);
  double vAngle = std::max(this->dataPtr->kMinAllowedAngle.Radian(),
      vmax - vmin);

  double hStep = hAngle / static_cast<double>(this->dataPtr->w2nd-1);
  double vStep = 1.0;
  // non-planar case
  if (this->dataPtr->h2nd > 1)
    vStep = vAngle / static_cast<double>(this->dataPtr->h2nd-1);

  // create an RGB texture (cubeUVTex) to pack info that tells the shaders how
  // to sample from the cubemap textures.
  // Each pixel packs the follow data:
  //   R: u coordinate on the cubemap face
  //   G: v coordinate on the cubemap face
  //   B: cubemap face index
  //   A: unused
  // this texture is passed to the 2nd pass fragment shader
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();
  std::string texName = this->Name() + "_samplerTex";
  this->dataPtr->cubeUVTexture =
    textureMgr->createOrRetrieveTexture(
      texName,
      Ogre::GpuPageOutStrategy::SaveToSystemRam,
      Ogre::TextureFlags::ManualTexture,
      Ogre::TextureTypes::Type2D,
      Ogre::BLANKSTRING,
      0u);

  this->dataPtr->cubeUVTexture->setTextureType(Ogre::TextureTypes::Type2D);
  this->dataPtr->cubeUVTexture->setResolution(
    this->dataPtr->w2nd, this->dataPtr->h2nd);
  this->dataPtr->cubeUVTexture->setNumMipmaps(1u);
  this->dataPtr->cubeUVTexture->setPixelFormat(Ogre::PFG_RGBA32_FLOAT);

  const Ogre::uint32 rowAlignment = 1u;
  const size_t dataSize = Ogre::PixelFormatGpuUtils::getSizeBytes(
    this->dataPtr->cubeUVTexture->getWidth(),
    this->dataPtr->cubeUVTexture->getHeight(),
    this->dataPtr->cubeUVTexture->getDepth(),
    this->dataPtr->cubeUVTexture->getNumSlices(),
    this->dataPtr->cubeUVTexture->getPixelFormat(),
    rowAlignment);

  const size_t bytesPerRow =
    this->dataPtr->cubeUVTexture->_getSysRamCopyBytesPerRow( 0 );
  float *pDest = reinterpret_cast<float*>(
    OGRE_MALLOC_SIMD(dataSize, Ogre::MEMCATEGORY_RESOURCE));

  double v = vmin;
  int index = 0;
  for (unsigned int i = 0; i < this->dataPtr->h2nd; ++i)
  {
    const math::Quaterniond pitch(math::Vector3d(1, 0, 0), -v);
    double h = min;
    for (unsigned int j = 0; j < this->dataPtr->w2nd; ++j)
    {
      // set up dir vector to sample from a standard Y up cubemap
      math::Vector3d ray(0, 0, 1);

      math::Quaterniond yaw(math::Vector3d(0, 1, 0), -h);
      math::Vector3d dir = yaw * pitch * ray;
      unsigned int faceIdx;
      math::Vector2d uv = this->SampleCubemap(dir, faceIdx);
      this->dataPtr->cubeFaceIdx.insert(faceIdx);
      // gzdbg << "p(" << pitch << ") y(" << yaw << "): " << dir << " | "
      //       << uv << " | " << faceIdx << std::endl;
      // u
      pDest[index++] = uv.X();
      // v
      pDest[index++] = uv.Y();
      // face
      pDest[index++] = static_cast<float>(faceIdx);
      // unused
      pDest[index++] = 1.0;
      h += hStep;
    }
    v += vStep;
  }
  this->dataPtr->cubeUVTexture->_transitionTo(
    Ogre::GpuResidency::Resident,
    reinterpret_cast<Ogre::uint8*>(pDest) );
  // We have to upload the data via a StagingTexture, which acts as an
  // intermediate stash memory that is both visible to CPU and GPU.
  Ogre::StagingTexture *stagingTexture = textureMgr->getStagingTexture(
    this->dataPtr->cubeUVTexture->getWidth(),
    this->dataPtr->cubeUVTexture->getHeight(),
    this->dataPtr->cubeUVTexture->getDepth(),
    this->dataPtr->cubeUVTexture->getNumSlices(),
    this->dataPtr->cubeUVTexture->getPixelFormat() );
  stagingTexture->startMapRegion();
  // Map region of the staging texture. This function can be called from
  // any thread after startMapRegion has already been called.
  Ogre::TextureBox texBox = stagingTexture->mapRegion(
    this->dataPtr->cubeUVTexture->getWidth(),
    this->dataPtr->cubeUVTexture->getHeight(),
    this->dataPtr->cubeUVTexture->getDepth(),
    this->dataPtr->cubeUVTexture->getNumSlices(),
    this->dataPtr->cubeUVTexture->getPixelFormat());

  texBox.copyFrom(
    pDest,
    this->dataPtr->cubeUVTexture->getWidth(),
    this->dataPtr->cubeUVTexture->getHeight(),
    bytesPerRow);
  stagingTexture->stopMapRegion();
  stagingTexture->upload(texBox, this->dataPtr->cubeUVTexture, 0, 0, 0, true);
  // Tell the TextureGpuManager we're done with this StagingTexture.
  // Otherwise it will leak.
  textureMgr->removeStagingTexture(stagingTexture);
  stagingTexture = 0;
  // Do not free the pointer if texture's paging strategy is
  // GpuPageOutStrategy::AlwaysKeepSystemRamCopy
}

/////////////////////////////////////////////////////////
void Ogre2GpuRays::Setup1stPass()
{
  // Create tmp textures
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();

  GZ_ASSERT(!this->dataPtr->colorTexture &&       //
              !this->dataPtr->depthTexture &&     //
              !this->dataPtr->particleTexture &&  //
              !this->dataPtr->particleDepthTexture,
            "Textures not destroyed!");

  Ogre::CompositorChannelVec compoChannels;
  compoChannels.reserve(5u);

  std::string texName;

  texName = this->Name() + "_colorTexture";
  this->dataPtr->colorTexture = textureMgr->createTexture(
    texName, texName, Ogre::GpuPageOutStrategy::Discard,
    Ogre::TextureFlags::RenderToTexture, Ogre::TextureTypes::Type2D);
  this->dataPtr->colorTexture->setResolution(this->dataPtr->w1st,
                                             this->dataPtr->h1st);
  this->dataPtr->colorTexture->setNumMipmaps(1u);
  this->dataPtr->colorTexture->setPixelFormat(Ogre::PFG_R16_UNORM);
  this->dataPtr->colorTexture->scheduleTransitionTo(
    Ogre::GpuResidency::Resident);

  texName = this->Name() + "_depthTexture";
  this->dataPtr->depthTexture = textureMgr->createTexture(
    texName, texName, Ogre::GpuPageOutStrategy::Discard,
    Ogre::TextureFlags::RenderToTexture, Ogre::TextureTypes::Type2D);
  this->dataPtr->depthTexture->setResolution(this->dataPtr->w1st,
                                             this->dataPtr->h1st);
  this->dataPtr->depthTexture->setNumMipmaps(1u);
  this->dataPtr->depthTexture->setPixelFormat(Ogre::PFG_D32_FLOAT);
  this->dataPtr->depthTexture->scheduleTransitionTo(
    Ogre::GpuResidency::Resident);

  texName = this->Name() + "_particleTexture";
  this->dataPtr->particleTexture = textureMgr->createTexture(
    texName, texName, Ogre::GpuPageOutStrategy::Discard,
    Ogre::TextureFlags::RenderToTexture, Ogre::TextureTypes::Type2D);
  this->dataPtr->particleTexture->setResolution(this->dataPtr->w1st / 2u,
                                             this->dataPtr->h1st/ 2u);
  this->dataPtr->particleTexture->setNumMipmaps(1u);
  this->dataPtr->particleTexture->setPixelFormat(Ogre::PFG_RGBA8_UNORM);
  this->dataPtr->particleTexture->scheduleTransitionTo(
    Ogre::GpuResidency::Resident);

  texName = this->Name() + "_particleDepthTexture";
  this->dataPtr->particleDepthTexture = textureMgr->createTexture(
    texName, texName, Ogre::GpuPageOutStrategy::Discard,
    Ogre::TextureFlags::RenderToTexture, Ogre::TextureTypes::Type2D);
  this->dataPtr->particleDepthTexture->setResolution(this->dataPtr->w1st / 2u,
                                                     this->dataPtr->h1st / 2u);
  this->dataPtr->particleDepthTexture->setNumMipmaps(1u);
  this->dataPtr->particleDepthTexture->setPixelFormat(Ogre::PFG_D32_FLOAT);
  this->dataPtr->particleDepthTexture->scheduleTransitionTo(
    Ogre::GpuResidency::Resident);

  compoChannels.push_back(this->dataPtr->colorTexture);
  compoChannels.push_back(this->dataPtr->depthTexture);
  compoChannels.push_back(this->dataPtr->particleTexture);
  compoChannels.push_back(this->dataPtr->particleDepthTexture);

  // Create 1st pass compositor
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  // Cache main pass definition so we can alter its visibility mask
  Ogre::CompositorNodeDef *nodeDef =
    ogreCompMgr->getNodeDefinitionNonConst("GpuRays1stPass");
  Ogre::CompositorTargetDef *target0 = nodeDef->getTargetPass(0);
  Ogre::CompositorPassDefVec &colorPasses =
      target0->getCompositorPassesNonConst();
  assert(colorPasses.size() >= 1u);
  assert(colorPasses[0]->getType() == Ogre::PASS_SCENE);
  assert(dynamic_cast<Ogre::CompositorPassSceneDef *>(colorPasses[0]));
  this->dataPtr->mainPassSceneDef =
    static_cast<Ogre::CompositorPassSceneDef *>(colorPasses[0]);

  Ogre::CompositorTargetDef *target1 = nodeDef->getTargetPass(1);
  this->dataPtr->particleTargetDef = target1;
  GZ_ASSERT(this->dataPtr->particleTargetDef != nullptr,
            "Unable to get particle target in Ogre2GpuRays");

  const std::string wsDefName = "GpuRays1stPassWorkspace";
  Ogre::CompositorWorkspaceDef *wsDef =
      ogreCompMgr->getWorkspaceDefinition(wsDefName);

  if (!wsDef)
  {
    gzerr << "Unable to find workspace definition [" << wsDefName << "] "
           << " for " << this->Name();
  }

  // create cubemap cameras and render to texture using 1st pass compositor
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
  this->dataPtr->cubeCam = ogreSceneManager->createCamera(
      this->Name() + "_env");
  this->dataPtr->cubeCam->detachFromParent();
  this->ogreNode->attachObject(this->dataPtr->cubeCam);
  this->dataPtr->cubeCam->setFOVy(Ogre::Degree(90));
  this->dataPtr->cubeCam->setAspectRatio(1);
  this->dataPtr->cubeCam->setNearClipDistance(this->dataPtr->nearClipCube);
  this->dataPtr->cubeCam->setFarClipDistance(this->FarClipPlane());
  this->dataPtr->cubeCam->setFixedYawAxis(false);

  for (auto i : this->dataPtr->cubeFaceIdx)
  {
    // create render texture - these textures pack the range data
    // that will be used in the 2nd pass
    texName = this->Name() + "_first_pass_" + std::to_string(i);
    this->dataPtr->firstPassTextures[i] =
      textureMgr->createOrRetrieveTexture(
        texName,
        Ogre::GpuPageOutStrategy::Discard,
        Ogre::TextureFlags::RenderToTexture,
        Ogre::TextureTypes::Type2D);

    this->dataPtr->firstPassTextures[i]->setResolution(
      this->dataPtr->w1st, this->dataPtr->h1st);
    this->dataPtr->firstPassTextures[i]->setNumMipmaps(1u);
    this->dataPtr->firstPassTextures[i]->setPixelFormat(
      Ogre::PFG_RG32_FLOAT);
    this->dataPtr->firstPassTextures[i]->_setDepthBufferDefaults(
      Ogre::DepthBuffer::POOL_NO_DEPTH, false, Ogre::PFG_UNKNOWN);

    this->dataPtr->firstPassTextures[i]->scheduleTransitionTo(
      Ogre::GpuResidency::Resident);

    compoChannels.push_back(this->dataPtr->firstPassTextures[i]);

    // create compositor workspace
    this->dataPtr->ogreCompositorWorkspace1st[i] =
        ogreCompMgr->addWorkspace(
          this->scene->OgreSceneManager(),
          compoChannels,
          this->dataPtr->cubeCam,
          wsDefName,
          false, -1, 0, 0, Ogre::Vector4::ZERO, 0x00,
          this->dataPtr->kGpuRaysExecutionMask);

    compoChannels.pop_back();

    // add laser retro material switcher to workspace listener
    // so we can switch to use GORM_SOLID_COLOR
    this->dataPtr->laserRetroMaterialSwitcher[i].reset(
      new Ogre2LaserRetroMaterialSwitcher(this->scene, this,
                                          this->dataPtr->ogreCamera));
    this->dataPtr->ogreCompositorWorkspace1st[i]->addListener(
      this->dataPtr->laserRetroMaterialSwitcher[i].get());
  }
}

/////////////////////////////////////////////////////////
void Ogre2GpuRays::Setup2ndPass()
{
  // Create second pass RTT, which stores the final range data output
  // see PostRender on how we retrieve data from this texture
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();

  this->dataPtr->secondPassTexture =
    textureMgr->createOrRetrieveTexture(
      this->Name() + "_second_pass",
      Ogre::GpuPageOutStrategy::SaveToSystemRam,
      Ogre::TextureFlags::RenderToTexture,
      Ogre::TextureTypes::Type2D);

  this->dataPtr->secondPassTexture->setResolution(
    this->dataPtr->w2nd, this->dataPtr->h2nd);
  this->dataPtr->secondPassTexture->setNumMipmaps(1u);
  this->dataPtr->secondPassTexture->setPixelFormat(
    Ogre::PFG_RGBA32_FLOAT);
  this->dataPtr->secondPassTexture->_setDepthBufferDefaults(
    Ogre::DepthBuffer::POOL_NO_DEPTH, false, Ogre::PFG_UNKNOWN);

  this->dataPtr->secondPassTexture->scheduleTransitionTo(
    Ogre::GpuResidency::Resident);

  Ogre::CompositorChannelVec compoChannels;

  // The compositor will plug all these textures into the material
  // The compositor needs to know about them so it can perform
  // the propier barrier and transitions on advanced APIs like Vulkan
  compoChannels.push_back(this->dataPtr->secondPassTexture);
  compoChannels.push_back(this->dataPtr->cubeUVTexture);
  for (size_t i = 0u; i < 6u; ++i)
  {
    if(this->dataPtr->firstPassTextures[i])
    {
      compoChannels.push_back(this->dataPtr->firstPassTextures[i]);
    }
    else
    {
      // Ensure there's always non-nullptrs in all slots to avoid
      // Ogre complaining. The material pass won't be accessing those
      // indices anyway
      compoChannels.push_back(
        this->dataPtr->firstPassTextures[*this->dataPtr->cubeFaceIdx.begin()]);
    }
  }

  // create 2nd pass compositor
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  const std::string wsDefName = "GpuRays2ndPassWorkspace";
  Ogre::CompositorWorkspaceDef *wsDef =
      ogreCompMgr->getWorkspaceDefinition(wsDefName);
  if (!wsDef)
  {
    gzerr << "Unable to find workspace definition [" << wsDefName << "] "
           << " for " << this->Name();
  }

  // create the compositor workspace
  this->dataPtr->ogreCompositorWorkspace2nd =
      ogreCompMgr->addWorkspace(
        this->scene->OgreSceneManager(),
        compoChannels,
        this->dataPtr->ogreCamera,
        wsDefName,
        false);
}

/////////////////////////////////////////////////////////
void Ogre2GpuRays::CreateGpuRaysTextures()
{
  // make cube cam near clip smaller than specified and manually clip range
  // values in 1st pass shader (gpu_rays_1st_pass_fs.glsl).
  // This is so that we don't incorrectly clip the range values near the
  // corners of the cube cam viewport.

  // compute smallest box to fit in sphere with radius = this->NearClipPlane
  double boxSize = this->NearClipPlane() * 2 / std::sqrt(3.0);
  this->dataPtr->nearClipCube = boxSize * 0.5;

  this->ConfigureCamera();
  this->CreateSampleTexture();
  this->Setup1stPass();
  this->Setup2ndPass();
}

/////////////////////////////////////////////////
void Ogre2GpuRays::UpdateRenderTarget1stPass()
{
  Ogre::vector<Ogre::TextureGpu *>::type swappedTargets;
  swappedTargets.reserve(2u);

  this->dataPtr->mainPassSceneDef->setVisibilityMask(
    this->VisibilityMask() & ~Ogre2ParticleEmitter::kParticleVisibilityFlags);

  // update the compositors
  for (auto i : this->dataPtr->cubeFaceIdx)
  {
    this->dataPtr->cubeCam->setOrientation(Ogre::Quaternion::IDENTITY);
    this->dataPtr->cubeCam->yaw(Ogre::Degree(-90));
    this->dataPtr->cubeCam->roll(Ogre::Degree(-90));
    // orient camera to its corresponding cubemap face
    if (i == 0)
      this->dataPtr->cubeCam->yaw(Ogre::Degree(-90));
    else if (i == 1)
      this->dataPtr->cubeCam->yaw(Ogre::Degree(90));
    else if (i == 2)
      this->dataPtr->cubeCam->pitch(Ogre::Degree(90));
    else if (i == 3)
      this->dataPtr->cubeCam->pitch(Ogre::Degree(-90));
    else if (i == 5)
      this->dataPtr->cubeCam->yaw(Ogre::Degree(180));

    this->scene->UpdateAllHeightmaps(this->dataPtr->cubeCam);

    this->dataPtr->ogreCompositorWorkspace1st[i]->setEnabled(true);

    this->dataPtr->ogreCompositorWorkspace1st[i]->_validateFinalTarget();
    this->dataPtr->ogreCompositorWorkspace1st[i]->_beginUpdate(false);
    this->dataPtr->ogreCompositorWorkspace1st[i]->_update();
    this->dataPtr->ogreCompositorWorkspace1st[i]->_endUpdate(false);

    swappedTargets.clear();
    this->dataPtr->ogreCompositorWorkspace1st[i]->_swapFinalTarget(
          swappedTargets );

    this->dataPtr->ogreCompositorWorkspace1st[i]->setEnabled(false);
  }
}

/////////////////////////////////////////////////
void Ogre2GpuRays::UpdateRenderTarget2ndPass()
{
  this->dataPtr->ogreCompositorWorkspace2nd->_validateFinalTarget();
  this->dataPtr->ogreCompositorWorkspace2nd->_beginUpdate(false);
  this->dataPtr->ogreCompositorWorkspace2nd->_update();
  this->dataPtr->ogreCompositorWorkspace2nd->_endUpdate(false);

  Ogre::vector<Ogre::TextureGpu *>::type swappedTargets;
  swappedTargets.reserve(2u);
  this->dataPtr->ogreCompositorWorkspace2nd->_swapFinalTarget(swappedTargets);
}

//////////////////////////////////////////////////
void Ogre2GpuRays::Render()
{
  this->scene->StartRendering(this->dataPtr->ogreCamera);

  auto engine = Ogre2RenderEngine::Instance();

  // The Hlms customizations add a "spherical" clipping; which ignores depth
  // clamping as it clips before sending vertices to the pixel shader.
  // These customization can be used to implement multi-tiered
  // "near plane distances" as proposed in:
  // https://github.com/gazebosim/gz-rendering/issues/395
  Ogre2GzHlmsSphericalClipMinDistance &hlmsCustomizations =
      engine->SphericalClipMinDistance();

  hlmsCustomizations.minDistanceClip =
      static_cast<float>(this->NearClipPlane());
  this->UpdateRenderTarget1stPass();
  this->UpdateRenderTarget2ndPass();
  hlmsCustomizations.minDistanceClip = -1;

  this->scene->FlushGpuCommandsAndStartNewFrame(6u, false);
}

//////////////////////////////////////////////////
void Ogre2GpuRays::PreRender()
{
  if (!this->dataPtr->cubeUVTexture)
    this->CreateGpuRaysTextures();

  if (this->dataPtr->particleTargetDef)
  {
    bool hasParticles =
        this->scene->OgreSceneManager()->getMovableObjectIterator(
        Ogre::ParticleSystemFactory::FACTORY_TYPE_NAME).hasMoreElements();
    Ogre::CompositorPassDefVec &particlePasses =
        this->dataPtr->particleTargetDef->getCompositorPassesNonConst();
    GZ_ASSERT(particlePasses.size() == 2u,
        "Ogre2DepthCamera particle target should 2 passes");
    GZ_ASSERT(particlePasses[0]->getType() == Ogre::PASS_CLEAR,
        "Ogre2DepthCamera particle target should start with a clear pass");
    GZ_ASSERT(particlePasses[1]->getType() == Ogre::PASS_SCENE,
        "Ogre2DepthCamera particle target should end with a scene pass");
    particlePasses[0]->mExecutionMask =
      (hasParticles) ? ~this->dataPtr->kGpuRaysExecutionMask :
                       this->dataPtr->kGpuRaysExecutionMask;
    particlePasses[1]->mExecutionMask =
      (hasParticles) ? this->dataPtr->kGpuRaysExecutionMask :
                       ~this->dataPtr->kGpuRaysExecutionMask;
  }
}

//////////////////////////////////////////////////
void Ogre2GpuRays::PostRender()
{
  unsigned int width = this->dataPtr->w2nd;
  unsigned int height = this->dataPtr->h2nd;

  PixelFormat format = PF_FLOAT32_RGBA;
  unsigned int rawChannelCount = PixelUtil::ChannelCount(format);
  unsigned int bytesPerChannel = PixelUtil::BytesPerChannel(format);

  // blit data from gpu to cpu
  Ogre::Image2 image;
  image.convertFromTexture(this->dataPtr->secondPassTexture, 0u, 0u);
  Ogre::TextureBox box = image.getData(0u);
  float *bufferTmp = static_cast<float *>(box.data);

  // Metal does not support RGB32_FLOAT so the internal texture format is
  // RGBA32_FLOAT. For backward compatibility, output data is kept in RGB
  // format instead of RGBA
  int outputLen = width * height * this->Channels();
  if (!this->dataPtr->gpuRaysScan)
  {
    this->dataPtr->gpuRaysScan = new float[outputLen];
  }

  // copy data from RGBA buffer to RGB buffer
  for (unsigned int row = 0; row < height; ++row)
  {
    unsigned int rawDataRowIdx = row * box.bytesPerRow / bytesPerChannel;
    unsigned int rowIdx = row * width * this->Channels();

    // the texture box step size could be larger than our image buffer step
    // size
    for (unsigned int column = 0; column < width; ++column)
    {
      unsigned int idx = rowIdx + column * this->Channels();
      unsigned int rawIdx = rawDataRowIdx + column * rawChannelCount;

      this->dataPtr->gpuRaysScan[idx] = bufferTmp[rawIdx];
      this->dataPtr->gpuRaysScan[idx + 1] = bufferTmp[rawIdx + 1];
      this->dataPtr->gpuRaysScan[idx + 2] = bufferTmp[rawIdx + 2];
    }
  }

  this->dataPtr->newGpuRaysFrame(this->dataPtr->gpuRaysScan,
      width, height, this->Channels(), "PF_FLOAT32_RGB");

  // Uncomment to debug output
  // std::cerr << "wxh: " << width << " x " << height << std::endl;
  // for (unsigned int i = 0; i < height; ++i)
  // {
  //   for (unsigned int j = 0; j < width; ++j)
  //   {
  //     std::cerr
  //     << "["
  //     << this->dataPtr->gpuRaysScan[i*width*3 + j*3]
  //     <<  " "
  //     << this->dataPtr->gpuRaysScan[i*width*3 + j*3 + 1]
  //     <<  " "
  //     << this->dataPtr->gpuRaysScan[i*width*3 + j*3 + 2]
  //     <<  "]\n";
  //   }
  //   std::cerr << std::endl;
  // }
}

//////////////////////////////////////////////////
const float* Ogre2GpuRays::Data() const
{
  return this->dataPtr->gpuRaysScan;
}

//////////////////////////////////////////////////
void Ogre2GpuRays::Copy(float *_dataDest)
{
  unsigned int width = this->dataPtr->w2nd;
  unsigned int height = this->dataPtr->h2nd;

  memcpy(_dataDest, this->dataPtr->gpuRaysScan,
    width * height * 3 * sizeof(float));
}

/////////////////////////////////////////////////
void Ogre2GpuRays::Set1stTextureSize(
    const unsigned int _w, const unsigned int _h)
{
  this->dataPtr->w1st = _w;
  this->dataPtr->h1st = _h;
}

/////////////////////////////////////////////////
void Ogre2GpuRays::SetRangeCount(
    const unsigned int _w, const unsigned int _h)
{
  this->dataPtr->w2nd = _w;
  this->dataPtr->h2nd = _h;
}

//////////////////////////////////////////////////
common::ConnectionPtr Ogre2GpuRays::ConnectNewGpuRaysFrame(
    std::function<void(const float *_frame, unsigned int _width,
    unsigned int _height, unsigned int _channels,
    const std::string &/*_format*/)> _subscriber)
{
  return this->dataPtr->newGpuRaysFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr Ogre2GpuRays::RenderTarget() const
{
  return this->dataPtr->renderTexture;
}
