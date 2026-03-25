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

#include "gz/rendering/ogre2/Ogre2WideAngleCamera.hh"

#include "gz/rendering/CameraLens.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2RenderPass.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"

#include "gz/common/Util.hh"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorManager2.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuad.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuadDef.h>
#include <Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h>
#include <OgreDepthBuffer.h>
#include <OgreImage2.h>
#include <OgrePass.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#include <OgreTextureBox.h>
#include <OgreTextureGpuManager.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

// clang-format off
namespace gz
{
namespace rendering
{
inline namespace GZ_RENDERING_VERSION_NAMESPACE {
/// \brief Helper class for setting up Camera and Materials when rendering
/// via Ogre2WideAngleCamera.
class GZ_RENDERING_OGRE2_HIDDEN Ogre2WideAngleCameraWorkspaceListenerPrivate :
    public Ogre::CompositorWorkspaceListener
{
  public: gz::rendering::Ogre2WideAngleCamera &owner;

  /// \brief constructor
  /// \param[in] _scene the scene manager responsible for rendering
  public: explicit Ogre2WideAngleCameraWorkspaceListenerPrivate(
        gz::rendering::Ogre2WideAngleCamera &_owner) :
    owner(_owner)
  {
  }

  /// \brief Called when each pass is about to be executed.
  /// \param[in] _pass Ogre pass which is about to execute
  public: virtual void passPreExecute(Ogre::CompositorPass *_pass) override;
};

// Matches CompositorPass::CubemapRotations from OgreCompositorPass.cpp
const Ogre::Quaternion kCubemapRotations[6] = {
  Ogre::Quaternion(Ogre::Radian(-1.570796f), Ogre::Vector3(0, 1, 0)),  // +X
  Ogre::Quaternion(Ogre::Radian(1.570796f), Ogre::Vector3(0, 1, 0)),   // -X
  Ogre::Quaternion(Ogre::Radian(1.570796f), Ogre::Vector3(1, 0, 0)),   // +Y
  Ogre::Quaternion(Ogre::Radian(-1.570796f), Ogre::Vector3(1, 0, 0)),  // -Y
  Ogre::Quaternion(1, 0, 0, 0),                            // +Z
  Ogre::Quaternion(Ogre::Radian(3.1415927f), Ogre::Vector3(0, 1, 0))   // -Z
};

static const char *kWideAngleCameraSuffixes[6] = { "PX", "NX", "PY",
                                                   "NY", "PZ", "NZ" };
}
}
}
// clang-format on

static const uint32_t kWideAngleNumCubemapFaces = 6u;

static const uint32_t kStichTmpTexture = 0u;
static const uint32_t kStichFinalTexture = 1u;
static const uint32_t kNumStichTextures = 2u;

/// \brief Private data for the WideAngleCamera class
class gz::rendering::Ogre2WideAngleCamera::Implementation
{
  // clang-format off
  /// \brief Environment texture size
  public: uint32_t envTextureSize = 512u;

  /// \brief A single cube map texture
  public: Ogre::TextureGpu *envCubeMapTexture = nullptr;

  /// \brief Temp 2D textures that will be later saved to envCubeMapTexture
  /// We use two for ping pong effects.
  public: Ogre::TextureGpu *ogreTmpTextures[2]{};

  /// \brief Output texture with the the toutput for stitches
  /// [kStichTmpTexture] = Temp 2D texture for ping point effects
  /// [kStichFinalTexture] = Output texture with the final output
  public: Ogre::TextureGpu *ogreStitchTexture[kNumStichTextures] =
  { nullptr, nullptr };

  /// \brief Compositor workspace. Does all the work. One for each face
  public: Ogre::CompositorWorkspace *ogreCompositorWorkspace[6];

  /// \brief Compositor workspace. Converts the cubemap into a "fish eye"
  public: Ogre::CompositorWorkspace *ogreCompositorFinalPass = nullptr;

  /// \brief Main pass definition (used for visibility mask manipuluation).
  public: Ogre::CompositorPassSceneDef *cubePassSceneDef = nullptr;

  /// \brief Pointer to material, used for second rendering pass
  public: Ogre::MaterialPtr compMat;

  /// \brief Camera lens description
  public: CameraLens lens;

  /// \brief Pointer to the ogre camera
  /// OgreNext rotates to face each of the 6 sides
  /// We use listeners to set FOV to the proper setting depending on the pass
  public: Ogre::Camera *ogreCamera = nullptr;

  /// \brief Dummy texture
  public: Ogre2RenderTexturePtr wideAngleTexture;

  /// \brief A chain of render passes applied to the render target
  public: std::vector<RenderPassPtr> renderPasses;

  /// \brief A chain of render passes applied to final stitched render target
  public: std::vector<RenderPassPtr> finalStitchRenderPasses;

  /// \brief Event used to signal camera data
  public: gz::common::EventT<void(const unsigned char *,
              unsigned int, unsigned int, unsigned int,
              const std::string &)> newImageFrame;

  /// \brief See Ogre2WideAngleCameraWorkspaceListenerPrivate
  public: Ogre2WideAngleCameraWorkspaceListenerPrivate workspaceListener;

  /// \brief Name of sky box material
  public: const std::string kSkyboxMaterialName = "SkyBox";

  /// \brief Background material of the render target
  public: MaterialPtr backgroundMaterial;

  /// \brief Flag to indicate if the render target background material has
  /// changed
  public: bool backgroundMaterialDirty = false;

  /// \brief Destination image data - used if image format is not rgb
  ///  and needs to be converted to another format.
  public: std::unique_ptr<unsigned char []> dstImgData;

  explicit Implementation(gz::rendering::Ogre2WideAngleCamera &_owner) :
    workspaceListener(_owner)
  {
    for (unsigned int i = 0; i < 6; ++i)
    {
      this->ogreCompositorWorkspace[i] = nullptr;
    }
  }
  // clang-format on
};

using namespace gz;
using namespace rendering;

// Arbitrary values
static constexpr uint32_t kWideAngleCameraCubemapPassId = 1276660u;
static constexpr uint32_t kWideAngleCameraQuadPassId = 1276661u;

//////////////////////////////////////////////////
Ogre2WideAngleCamera::Ogre2WideAngleCamera() :
  dataPtr(utils::MakeUniqueImpl<Implementation>(*this))
{
}

//////////////////////////////////////////////////
Ogre2WideAngleCamera::~Ogre2WideAngleCamera()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::Init()
{
  BaseWideAngleCamera::Init();
  this->CreateCamera();
  this->CreateRenderTexture();
  this->Reset();
}

/////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateRenderTexture()
{
  this->DestroyRenderTexture();
  RenderTexturePtr base = this->scene->CreateRenderTexture();
  this->dataPtr->wideAngleTexture =
    std::dynamic_pointer_cast<Ogre2RenderTexture>(base);
  this->dataPtr->wideAngleTexture->SetWidth(1);
  this->dataPtr->wideAngleTexture->SetHeight(1);
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::DestroyRenderTexture()
{
  if (this->dataPtr->wideAngleTexture)
  {
    dynamic_cast<Ogre2RenderTexture *>(this->dataPtr->wideAngleTexture.get())
      ->Destroy();
    this->dataPtr->wideAngleTexture.reset();
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::PreRender()
{
  BaseCamera::PreRender();

  if (this->dataPtr->backgroundMaterialDirty)
  {
    this->UpdateBackgroundMaterial();
    this->RemoveAllRenderPasses();
    this->DestroyTextures();
  }

  {
    auto thisAsCameraPtr =
      std::dynamic_pointer_cast<Camera>(this->shared_from_this());

    for (RenderPassPtr &pass : this->dataPtr->renderPasses)
    {
      pass->PreRender(thisAsCameraPtr);
    }
    for (RenderPassPtr &pass : this->dataPtr->finalStitchRenderPasses)
    {
      pass->PreRender(thisAsCameraPtr);
    }
  }

  if (!this->dataPtr->ogreStitchTexture[kStichFinalTexture])
  {
    this->CreateWideAngleTexture();
  }
  else
  {
    if (!this->dataPtr->ogreCompositorWorkspace[0])
    {
      // If render passes were added/destroyed, ogreCompositorWorkspace
      // will become nullptr and must be recreated.
      const uint8_t msaa =
        Ogre2RenderTarget::TargetFSAA(static_cast<uint8_t>(this->antiAliasing));
      this->CreateFacesWorkspaces(msaa > 1u);
    }

    if (!this->dataPtr->ogreCompositorFinalPass)
    {
      // If render passes were added/destroyed, ogreCompositorWorkspace
      // will become nullptr and must be recreated.
      this->CreateStitchWorkspace();
    }
  }

  this->UpdateRenderPasses();
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::Destroy()
{
  this->RemoveAllRenderPasses();
  this->DestroyTextures();
  this->DestroyRenderTexture();
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::DestroyTextures()
{
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();

  this->DestroyStitchWorkspace();
  this->DestroyFacesWorkspaces();

  for (uint32_t i = 0u; i < 2u; ++i)
  {
    if (this->dataPtr->ogreTmpTextures[i])
    {
      textureMgr->destroyTexture(this->dataPtr->ogreTmpTextures[i]);
      this->dataPtr->ogreTmpTextures[i] = nullptr;
    }
  }

  if (this->dataPtr->envCubeMapTexture)
  {
    textureMgr->destroyTexture(this->dataPtr->envCubeMapTexture);
    this->dataPtr->envCubeMapTexture = nullptr;
  }

  for (uint32_t i = 0; i < kNumStichTextures; ++i)
  {
    if (this->dataPtr->ogreStitchTexture[i])
    {
      textureMgr->destroyTexture(this->dataPtr->ogreStitchTexture[i]);
      this->dataPtr->ogreStitchTexture[i] = nullptr;
    }
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::AddRenderPass(const RenderPassPtr &_pass)
{
  // Do NOT pass it to super class.
  if (_pass->WideAngleCameraAfterStitching())
  {
    this->dataPtr->finalStitchRenderPasses.push_back(_pass);
    this->DestroyStitchWorkspace();

    if (this->HasTempStitchTexture() != this->NeedsTempStitchTexture())
    {
      this->DestroyTextures();
    }
  }
  else
  {
    this->dataPtr->renderPasses.push_back(_pass);
    this->DestroyFacesWorkspaces();
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::RemoveRenderPass(const RenderPassPtr &_pass)
{
  // Do NOT pass it to super class.
  auto it = std::find(this->dataPtr->renderPasses.begin(),
                      this->dataPtr->renderPasses.end(), _pass);
  if (it != this->dataPtr->renderPasses.end())
  {
    (*it)->Destroy();
    this->dataPtr->renderPasses.erase(it);
    this->DestroyFacesWorkspaces();
  }
  else
  {
    it = std::find(this->dataPtr->finalStitchRenderPasses.begin(),
                   this->dataPtr->finalStitchRenderPasses.end(), _pass);
    if (it != this->dataPtr->finalStitchRenderPasses.end())
    {
      (*it)->Destroy();
      this->dataPtr->finalStitchRenderPasses.erase(it);
      this->DestroyStitchWorkspace();

      if (this->HasTempStitchTexture() != this->NeedsTempStitchTexture())
      {
        this->DestroyTextures();
      }
    }
    else
    {
      gzwarn << "Ogre2WideAngleCamera::RemoveRenderPass pass not found. This "
                "is fine if you called this function twice. But it may not be "
                "fine if you changed the value "
                "RenderPass::WideAngleCameraAfterStitching (see docs)"
             << std::endl;
    }
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::RemoveAllRenderPasses()
{
  for (RenderPassPtr &pass : this->dataPtr->renderPasses)
  {
    pass->Destroy();
  }
  for (RenderPassPtr &pass : this->dataPtr->finalStitchRenderPasses)
  {
    pass->Destroy();
  }
  this->dataPtr->renderPasses.clear();
  this->dataPtr->finalStitchRenderPasses.clear();
  this->DestroyFacesWorkspaces();
}

//////////////////////////////////////////////////
uint32_t Ogre2WideAngleCamera::EnvTextureSize() const
{
  return this->dataPtr->envTextureSize;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::SetEnvTextureSize(uint32_t _size)
{
  this->dataPtr->envTextureSize = _size;
}

/////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateCamera()
{
  // Create dummy ogre camera object
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();
  if (ogreSceneManager == nullptr)
  {
    gzerr << "Scene manager cannot be obtained" << std::endl;
    return;
  }

  this->dataPtr->ogreCamera =
    ogreSceneManager->createCamera(this->Name() + "_Camera", true, true);
  if (this->dataPtr->ogreCamera == nullptr)
  {
    gzerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  // OgreNext by default attaches the camera to the Root SceneNode
  // Detach from it and use our own node.
  this->dataPtr->ogreCamera->detachFromParent();
  this->ogreNode->attachObject(this->dataPtr->ogreCamera);

  this->dataPtr->ogreCamera->setFixedYawAxis(false);
  this->dataPtr->ogreCamera->yaw(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->roll(Ogre::Degree(-90));
  this->dataPtr->ogreCamera->setAutoAspectRatio(true);

  const Ogre::Real nearPlane = static_cast<Ogre::Real>(this->NearClipPlane());
  const Ogre::Real farPlane = static_cast<Ogre::Real>(this->FarClipPlane());
  this->dataPtr->ogreCamera->setNearClipDistance(nearPlane);
  this->dataPtr->ogreCamera->setFarClipDistance(farPlane);
}

//////////////////////////////////////////////////
std::string Ogre2WideAngleCamera::WorkspaceFinalPassDefinitionName() const
{
  const std::string wsDefName = "WideAngleCamera/" + this->Name() + "/Final";
  return wsDefName;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateStitchWorkspaceDefinition()
{
  using namespace Ogre;
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  const std::string wsDefName = this->WorkspaceFinalPassDefinitionName();

  CompositorWorkspaceDef *workDef =
    ogreCompMgr->addWorkspaceDefinition(wsDefName);

  const IdString stitchPassNodeName = "WideAngleCameraFinalPass";

  const uint32_t tempStitchTextureChannel = this->TempStitchTextureChannel();

  workDef->connectExternal(0, stitchPassNodeName, tempStitchTextureChannel);
  workDef->connectExternal(1, stitchPassNodeName, !tempStitchTextureChannel);
  workDef->connectExternal(2, stitchPassNodeName, 2);

  IdString prevNode = stitchPassNodeName;
  for (RenderPassPtr &pass : this->dataPtr->finalStitchRenderPasses)
  {
    GZ_ASSERT(pass->WideAngleCameraAfterStitching(),
              "Cannot change the setting of WideAngleCameraAfterStitching "
              "after adding it to Camera");

    Ogre2RenderPass *ogre2RenderPass =
      dynamic_cast<Ogre2RenderPass *>(pass.get());

    if (ogre2RenderPass->IsEnabled())
    {
      ogre2RenderPass->CreateRenderPass();
      IdString currNode = ogre2RenderPass->OgreCompositorNodeDefinitionName();
      workDef->connect(prevNode, currNode);
      prevNode = currNode;
    }
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateStitchWorkspace()
{
  this->CreateStitchWorkspaceDefinition();

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();

  Ogre::CompositorChannelVec channelsFinalPass = {
    nullptr, nullptr, this->dataPtr->envCubeMapTexture
  };

  if (!this->NeedsTempStitchTexture())
  {
    // Set both to kStichFinalTexture to keep OgreNext happy about
    // all channels having been connected
    channelsFinalPass[0] = this->dataPtr->ogreStitchTexture[kStichFinalTexture];
    channelsFinalPass[1] = this->dataPtr->ogreStitchTexture[kStichFinalTexture];
  }
  else
  {
    channelsFinalPass[0] = this->dataPtr->ogreStitchTexture[kStichTmpTexture];
    channelsFinalPass[1] = this->dataPtr->ogreStitchTexture[kStichFinalTexture];
  }

  this->dataPtr->ogreCompositorFinalPass = ogreCompMgr->addWorkspace(
    ogreSceneManager, channelsFinalPass, this->dataPtr->ogreCamera,
    this->WorkspaceFinalPassDefinitionName(), false);
  this->dataPtr->ogreCompositorFinalPass->addListener(
    &this->dataPtr->workspaceListener);
  for (RenderPassPtr &pass : this->dataPtr->finalStitchRenderPasses)
  {
    Ogre2RenderPass *ogre2RenderPass =
      dynamic_cast<Ogre2RenderPass *>(pass.get());
    ogre2RenderPass->WorkspaceAdded(this->dataPtr->ogreCompositorFinalPass);
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::DestroyStitchWorkspace()
{
  if (this->dataPtr->ogreCompositorFinalPass)
  {
    auto engine = Ogre2RenderEngine::Instance();
    auto ogreRoot = engine->OgreRoot();
    Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

    const Ogre::IdString workspaceName =
      this->dataPtr->ogreCompositorFinalPass->getDefinition()->getName();

    for (RenderPassPtr &pass : this->dataPtr->finalStitchRenderPasses)
    {
      Ogre2RenderPass *ogre2RenderPass =
        dynamic_cast<Ogre2RenderPass *>(pass.get());
      ogre2RenderPass->WorkspaceRemoved(this->dataPtr->ogreCompositorFinalPass);
    }

    ogreCompMgr->removeWorkspace(this->dataPtr->ogreCompositorFinalPass);
    this->dataPtr->ogreCompositorFinalPass = nullptr;

    ogreCompMgr->removeWorkspaceDefinition(workspaceName);
  }
}

//////////////////////////////////////////////////
bool Ogre2WideAngleCamera::HasTempStitchTexture() const
{
  return this->dataPtr->ogreStitchTexture[kStichTmpTexture];
}

//////////////////////////////////////////////////
bool Ogre2WideAngleCamera::NeedsTempStitchTexture() const
{
  return !this->dataPtr->finalStitchRenderPasses.empty();
}

//////////////////////////////////////////////////
uint32_t Ogre2WideAngleCamera::TempStitchTextureChannel() const
{
  uint32_t enabledPasses = 0u;
  for (const auto &pass : this->dataPtr->finalStitchRenderPasses)
  {
    if (pass->IsEnabled())
    {
      ++enabledPasses;
    }
  }
  return enabledPasses & 0x1u ? 1u : 0u;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::SetupMSAA(Ogre::CompositorManager2 *_ogreCompMgr,
                                     uint8_t _msaa)
{
  GZ_ASSERT(_msaa > 1u, "Wrong API usage. Don't call this function");

  Ogre::CompositorNodeDef *nodeDef =
    _ogreCompMgr->getNodeDefinitionNonConst("WideAngleCameraCubemapPassMsaa");
  auto &textureDefs = nodeDef->getLocalTextureDefinitionsNonConst();

  GZ_ASSERT(textureDefs.size() == 1u,
            "wide_angle_camera.compositor out of sync?");
  GZ_ASSERT(textureDefs[0].getName() == "tmpMsaa",
            "wide_angle_camera.compositor out of sync?");

  textureDefs[0].fsaa = std::to_string(_msaa);
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::RetrieveCubePassSceneDefs(
  Ogre::CompositorManager2 *_ogreCompMgr, bool _withMsaa)
{
  Ogre::CompositorNodeDef *nodeDef = _ogreCompMgr->getNodeDefinitionNonConst(
    _withMsaa ? "WideAngleCameraCubemapPassMsaa"
              : "WideAngleCameraCubemapPass");

  Ogre::CompositorTargetDef *target0 = nodeDef->getTargetPass(0);
  Ogre::CompositorPassDefVec &passes = target0->getCompositorPassesNonConst();
  GZ_ASSERT(passes.size() >= 1u,
            "wide_angle_camera.compositor is out of sync?");
  GZ_ASSERT(passes[0]->getType() == Ogre::PASS_SCENE,
            "wide_angle_camera.compositor is out of sync?");
  GZ_ASSERT(dynamic_cast<Ogre::CompositorPassSceneDef *>(passes[0]),
            "Memory corruption?");

  this->dataPtr->cubePassSceneDef =
    static_cast<Ogre::CompositorPassSceneDef *>(passes[0]);
}

//////////////////////////////////////////////////
std::string Ogre2WideAngleCamera::WorkspaceDefinitionName(
  uint32_t _faceIdx) const
{
  const std::string wsDefName =
    "WideAngleCamera/" + this->Name() + kWideAngleCameraSuffixes[_faceIdx];
  return wsDefName;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateWorkspaceDefinition(bool _withMsaa)
{
  using namespace Ogre;
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  const IdString cubemapPassNodeName =
    _withMsaa ? "WideAngleCameraCubemapPassMsaa" : "WideAngleCameraCubemapPass";

  bool validBackground = this->dataPtr->backgroundMaterial &&
      !this->dataPtr->backgroundMaterial->EnvironmentMap().empty();

  // render background, e.g. sky, after opaque stuff
  if (validBackground)
  {
    Ogre::CompositorNodeDef *nodeDef = ogreCompMgr->getNodeDefinitionNonConst(
        cubemapPassNodeName);
    Ogre::CompositorTargetDef *target0 = nodeDef->getTargetPass(0);

    // quad pass
    Ogre::CompositorPassQuadDef *passQuad =
        static_cast<Ogre::CompositorPassQuadDef *>(
        target0->addPass(Ogre::PASS_QUAD));
    passQuad->mMaterialName = this->dataPtr->kSkyboxMaterialName + "_"
        + this->Name();
    passQuad->mFrustumCorners =
        Ogre::CompositorPassQuadDef::CAMERA_DIRECTION;
  }

  for (uint32_t faceIdx = 0u; faceIdx < kWideAngleNumCubemapFaces; ++faceIdx)
  {
    const std::string wsDefName = this->WorkspaceDefinitionName(faceIdx);
    CompositorWorkspaceDef *workDef =
      ogreCompMgr->addWorkspaceDefinition(wsDefName);

    workDef->connectExternal(0, cubemapPassNodeName, 0);
    workDef->connectExternal(1, cubemapPassNodeName, 1);

    IdString prevNode = cubemapPassNodeName;
    for (RenderPassPtr &pass : this->dataPtr->renderPasses)
    {
      GZ_ASSERT(!pass->WideAngleCameraAfterStitching(),
                "Cannot change the setting of WideAngleCameraAfterStitching "
                "after adding it to Camera");

      Ogre2RenderPass *ogre2RenderPass =
        dynamic_cast<Ogre2RenderPass *>(pass.get());

      if (ogre2RenderPass->IsEnabled())
      {
        ogre2RenderPass->CreateRenderPass();
        IdString currNode = ogre2RenderPass->OgreCompositorNodeDefinitionName();
        workDef->connect(prevNode, currNode);
        prevNode = currNode;
      }
    }

    const IdString copyPass = std::string("WideAngleCameraCubemapCopy") +
                              kWideAngleCameraSuffixes[faceIdx];
    workDef->connect(prevNode, copyPass);
    workDef->connectExternal(2, copyPass, 2);
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateFacesWorkspaces(bool _withMsaa)
{
  this->CreateWorkspaceDefinition(_withMsaa);

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();
  Ogre::SceneManager *ogreSceneManager = this->scene->OgreSceneManager();

  this->RetrieveCubePassSceneDefs(ogreCompMgr, _withMsaa);

  const Ogre::CompositorChannelVec channels = {
    this->dataPtr->ogreTmpTextures[0], this->dataPtr->ogreTmpTextures[1],
    this->dataPtr->envCubeMapTexture
  };

  for (uint32_t i = 0u; i < kWideAngleNumCubemapFaces; ++i)
  {
    GZ_ASSERT(!this->dataPtr->ogreCompositorWorkspace[i], "Must be nullptr!");

    this->dataPtr->ogreCompositorWorkspace[i] = ogreCompMgr->addWorkspace(
      ogreSceneManager, channels, this->dataPtr->ogreCamera,
      this->WorkspaceDefinitionName(i), false);
    this->dataPtr->ogreCompositorWorkspace[i]->addListener(
      &this->dataPtr->workspaceListener);

    for (RenderPassPtr &pass : this->dataPtr->renderPasses)
    {
      Ogre2RenderPass *ogre2RenderPass =
        dynamic_cast<Ogre2RenderPass *>(pass.get());
      ogre2RenderPass->WorkspaceAdded(
        this->dataPtr->ogreCompositorWorkspace[i]);
    }
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::DestroyFacesWorkspaces()
{
  using namespace Ogre;
  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  for (uint32_t i = 0u; i < kWideAngleNumCubemapFaces; ++i)
  {
    if (this->dataPtr->ogreCompositorWorkspace[i])
    {
      const IdString workspaceName =
        this->dataPtr->ogreCompositorWorkspace[i]->getDefinition()->getName();

      for (RenderPassPtr &pass : this->dataPtr->renderPasses)
      {
        Ogre2RenderPass *ogre2RenderPass =
          dynamic_cast<Ogre2RenderPass *>(pass.get());
        ogre2RenderPass->WorkspaceRemoved(
          this->dataPtr->ogreCompositorWorkspace[i]);
      }
      ogreCompMgr->removeWorkspace(this->dataPtr->ogreCompositorWorkspace[i]);
      this->dataPtr->ogreCompositorWorkspace[i] = nullptr;

      ogreCompMgr->removeWorkspaceDefinition(workspaceName);
    }
  }
  this->dataPtr->cubePassSceneDef = nullptr;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::UpdateRenderPasses()
{
  using namespace Ogre;
  bool updateConnection = false;

  // set node instance to render pass and update enabled state
  for (const auto &pass : this->dataPtr->renderPasses)
  {
    Ogre2RenderPass *ogre2RenderPass =
      dynamic_cast<Ogre2RenderPass *>(pass.get());

    const IdString currNode =
      ogre2RenderPass->OgreCompositorNodeDefinitionName();

    GZ_ASSERT(!pass->WideAngleCameraAfterStitching(),
              "Cannot change the setting of WideAngleCameraAfterStitching "
              "after adding it to Camera");

    for (size_t i = 0u; i < kWideAngleNumCubemapFaces; ++i)
    {
      Ogre::CompositorNode *node =
        this->dataPtr->ogreCompositorWorkspace[i]->findNodeNoThrow(currNode);

      // RemoveRenderPass destroys the workspace; which means we can't reach
      // here where we update an already existing workspace; but rather
      // have ended up in CreateFacesWorkspaces()
      //
      // Note: We assume ogre2RenderPass->OgreCompositorNodeDefinitionName
      // returns the same name (given the same ogre2RenderPass ptr)
      GZ_ASSERT(node,
                "This can't be possible. "
                "RemoveRenderPass() should've destroyed the workspace.");

      if (node->getEnabled() != ogre2RenderPass->IsEnabled())
      {
        node->setEnabled(ogre2RenderPass->IsEnabled());
        updateConnection = true;
      }
    }
  }

  for (const auto &pass : this->dataPtr->finalStitchRenderPasses)
  {
    Ogre2RenderPass *ogre2RenderPass =
      dynamic_cast<Ogre2RenderPass *>(pass.get());

    const IdString currNode =
      ogre2RenderPass->OgreCompositorNodeDefinitionName();

    GZ_ASSERT(pass->WideAngleCameraAfterStitching(),
              "Cannot change the setting of WideAngleCameraAfterStitching "
              "after adding it to Camera");
    Ogre::CompositorNode *node =
      this->dataPtr->ogreCompositorFinalPass->findNodeNoThrow(currNode);

    // RemoveRenderPass destroys the workspace; which means we can't reach
    // here where we update an already existing workspace; but rather
    // have ended up in CreateFacesWorkspaces()
    //
    // Note: We assume ogre2RenderPass->OgreCompositorNodeDefinitionName
    // returns the same name (given the same ogre2RenderPass ptr)
    GZ_ASSERT(node,
              "This can't be possible. "
              "RemoveRenderPass() should've destroyed the workspace.");

    if (node->getEnabled() != ogre2RenderPass->IsEnabled())
    {
      node->setEnabled(ogre2RenderPass->IsEnabled());
      updateConnection = true;
    }
  }

  if (!updateConnection)
    return;

  const uint8_t msaa =
    Ogre2RenderTarget::TargetFSAA(static_cast<uint8_t>(this->antiAliasing));
  const bool withMsaa = msaa > 1u;

  const IdString cubemapPassNodeName =
    withMsaa ? "WideAngleCameraCubemapPassMsaa" : "WideAngleCameraCubemapPass";

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  for (uint32_t faceIdx = 0u; faceIdx < kWideAngleNumCubemapFaces; ++faceIdx)
  {
    const std::string wsDefName = this->WorkspaceDefinitionName(faceIdx);
    CompositorWorkspaceDef *workDef =
      ogreCompMgr->getWorkspaceDefinitionNoThrow(wsDefName);

    workDef->clearAllInterNodeConnections();
    workDef->clearOutputConnections();

    workDef->connectExternal(0, cubemapPassNodeName, 0);
    workDef->connectExternal(1, cubemapPassNodeName, 1);

    IdString prevNode = cubemapPassNodeName;
    for (RenderPassPtr &pass : this->dataPtr->renderPasses)
    {
      GZ_ASSERT(!pass->WideAngleCameraAfterStitching(),
                "Cannot change the setting of WideAngleCameraAfterStitching "
                "after adding it to Camera");

      Ogre2RenderPass *ogre2RenderPass =
        dynamic_cast<Ogre2RenderPass *>(pass.get());

      if (ogre2RenderPass->IsEnabled())
      {
        ogre2RenderPass->CreateRenderPass();
        IdString currNode = ogre2RenderPass->OgreCompositorNodeDefinitionName();
        workDef->connect(prevNode, currNode);
        prevNode = currNode;
      }
    }

    const IdString copyPass = std::string("WideAngleCameraCubemapCopy") +
                              kWideAngleCameraSuffixes[faceIdx];
    workDef->connect(prevNode, copyPass);
    workDef->connectExternal(2, copyPass, 2);
  }

  {
    const std::string wsDefName = this->WorkspaceFinalPassDefinitionName();
    CompositorWorkspaceDef *workDef =
      ogreCompMgr->getWorkspaceDefinitionNoThrow(wsDefName);

    workDef->clearAllInterNodeConnections();
    workDef->clearOutputConnections();

    const IdString stitchPassNodeName = "WideAngleCameraFinalPass";

    const uint32_t tempStitchTextureChannel = this->TempStitchTextureChannel();

    workDef->connectExternal(0, stitchPassNodeName, tempStitchTextureChannel);
    workDef->connectExternal(1, stitchPassNodeName, !tempStitchTextureChannel);
    workDef->connectExternal(2, stitchPassNodeName, 2);

    IdString prevNode = stitchPassNodeName;
    for (RenderPassPtr &pass : this->dataPtr->finalStitchRenderPasses)
    {
      GZ_ASSERT(pass->WideAngleCameraAfterStitching(),
                "Cannot change the setting of WideAngleCameraAfterStitching "
                "after adding it to Camera");

      Ogre2RenderPass *ogre2RenderPass =
        dynamic_cast<Ogre2RenderPass *>(pass.get());

      if (ogre2RenderPass->IsEnabled())
      {
        ogre2RenderPass->CreateRenderPass();
        IdString currNode = ogre2RenderPass->OgreCompositorNodeDefinitionName();
        workDef->connect(prevNode, currNode);
        prevNode = currNode;
      }
    }
  }

  for (size_t i = 0u; i < kWideAngleNumCubemapFaces; ++i)
  {
    this->dataPtr->ogreCompositorWorkspace[i]->reconnectAllNodes();
  }

  this->dataPtr->ogreCompositorFinalPass->reconnectAllNodes();
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::CreateWideAngleTexture()
{
  if (this->dataPtr->ogreCamera == nullptr)
  {
    gzerr << "Ogre camera cannot be created" << std::endl;
    return;
  }

  auto engine = Ogre2RenderEngine::Instance();
  auto ogreRoot = engine->OgreRoot();
  Ogre::TextureGpuManager *textureMgr =
    ogreRoot->getRenderSystem()->getTextureGpuManager();

  for (uint32_t i = this->NeedsTempStitchTexture() ? 0u : 1u;
       i < kNumStichTextures; ++i)
  {
    GZ_ASSERT(!this->dataPtr->ogreStitchTexture[i], "Should be nullptr");

    this->dataPtr->ogreStitchTexture[i] = textureMgr->createTexture(
      this->Name() + "_wideAngleCameraStitchTex" + std::to_string(i),  //
      Ogre::GpuPageOutStrategy::Discard,                               //
      Ogre::TextureFlags::RenderToTexture,                             //
      Ogre::TextureTypes::Type2D,                                      //
      Ogre::BLANKSTRING,                                               //
      0u);

    this->dataPtr->ogreStitchTexture[i]->setResolution(this->ImageWidth(),
                                                       this->ImageHeight());
    this->dataPtr->ogreStitchTexture[i]->setPixelFormat(
      Ogre::PFG_RGBA8_UNORM_SRGB);
    this->dataPtr->ogreStitchTexture[i]->_setDepthBufferDefaults(
      Ogre::DepthBuffer::POOL_NO_DEPTH, false, Ogre::PFG_UNKNOWN);
    this->dataPtr->ogreStitchTexture[i]->scheduleTransitionTo(
      Ogre::GpuResidency::Resident);
  }

  this->dataPtr->envCubeMapTexture =
    textureMgr->createTexture(this->Name() + "_cube_wideAngleCamera",  //
                              Ogre::GpuPageOutStrategy::Discard,       //
                              Ogre::TextureFlags::RenderToTexture,     //
                              Ogre::TextureTypes::TypeCube,            //
                              Ogre::BLANKSTRING,                       //
                              0u);

  this->dataPtr->envCubeMapTexture->setResolution(
    this->dataPtr->envTextureSize, this->dataPtr->envTextureSize);
  this->dataPtr->envCubeMapTexture->setPixelFormat(Ogre::PFG_RGBA8_UNORM_SRGB);

  this->dataPtr->envCubeMapTexture->scheduleTransitionTo(
    Ogre::GpuResidency::Resident);

  for (uint32_t i = 0u; i < 2u; ++i)
  {
    this->dataPtr->ogreTmpTextures[i] = textureMgr->createTexture(
      this->Name() + "_tmpTexture2d/" + std::to_string(i),  //
      Ogre::GpuPageOutStrategy::Discard,                    //
      Ogre::TextureFlags::RenderToTexture,                  //
      Ogre::TextureTypes::Type2D,                           //
      Ogre::BLANKSTRING,                                    //
      0u);
    this->dataPtr->ogreTmpTextures[i]->setResolution(
      this->dataPtr->envTextureSize, this->dataPtr->envTextureSize);
    this->dataPtr->ogreTmpTextures[i]->setPixelFormat(
      this->dataPtr->envCubeMapTexture->getPixelFormat());
    this->dataPtr->ogreTmpTextures[i]->scheduleTransitionTo(
      Ogre::GpuResidency::Resident);
  }

  // Create compositor workspace
  Ogre::CompositorManager2 *ogreCompMgr = ogreRoot->getCompositorManager2();

  const uint8_t msaa =
    Ogre2RenderTarget::TargetFSAA(static_cast<uint8_t>(this->antiAliasing));

  if (msaa > 1u)
  {
    this->SetupMSAA(ogreCompMgr, msaa);
  }

  this->CreateFacesWorkspaces(msaa > 1u);
  this->CreateStitchWorkspace();
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::Render()
{
  // make sure we do not alter the reserved visibility flags
  const uint32_t currVisibilityMask = this->VisibilityMask() &
    Ogre::VisibilityFlags::RESERVED_VISIBILITY_FLAGS;
  this->dataPtr->cubePassSceneDef->mVisibilityMask = currVisibilityMask;

  this->scene->StartRendering(this->dataPtr->ogreCamera);

  Ogre::vector<Ogre::TextureGpu *>::type swappedTargets;

  const Ogre::Quaternion oldCameraOrientation(
    this->dataPtr->ogreCamera->getOrientation());

  for (size_t i = 0u; i < kWideAngleNumCubemapFaces; ++i)
  {
    this->dataPtr->ogreCompositorWorkspace[i]->setEnabled(true);

    this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation *
                                              kCubemapRotations[i]);
    this->scene->UpdateAllHeightmaps(this->dataPtr->ogreCamera);

    this->dataPtr->ogreCompositorWorkspace[i]->_validateFinalTarget();
    this->dataPtr->ogreCompositorWorkspace[i]->_beginUpdate(false);
    this->dataPtr->ogreCompositorWorkspace[i]->_update();
    this->dataPtr->ogreCompositorWorkspace[i]->_endUpdate(false);

    swappedTargets.clear();
    this->dataPtr->ogreCompositorWorkspace[i]->_swapFinalTarget(swappedTargets);

    this->dataPtr->ogreCompositorWorkspace[i]->setEnabled(false);
  }

  this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation);

  {
    this->dataPtr->ogreCompositorFinalPass->setEnabled(true);

    this->dataPtr->ogreCompositorFinalPass->_validateFinalTarget();
    this->dataPtr->ogreCompositorFinalPass->_beginUpdate(false);
    this->dataPtr->ogreCompositorFinalPass->_update();
    this->dataPtr->ogreCompositorFinalPass->_endUpdate(false);

    swappedTargets.clear();
    this->dataPtr->ogreCompositorFinalPass->_swapFinalTarget(swappedTargets);

    this->dataPtr->ogreCompositorFinalPass->setEnabled(false);
  }

  this->scene->FlushGpuCommandsAndStartNewFrame(kWideAngleNumCubemapFaces,
                                                false);
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::Copy(Image &_image) const
{
  if (_image.Width() != this->ImageWidth() ||
      _image.Height() != this->ImageHeight())
  {
    gzerr << "Invalid image dimensions" << std::endl;
    return;
  }

  Ogre::PixelFormatGpu dstOgrePf = Ogre2Conversions::Convert(_image.Format());
  Ogre::TextureGpu *texture =
    this->dataPtr->ogreStitchTexture[kStichFinalTexture];

  if (Ogre::PixelFormatGpuUtils::isSRgb(dstOgrePf) !=
      Ogre::PixelFormatGpuUtils::isSRgb(texture->getPixelFormat()))
  {
    // Formats are identical except for sRGB-ness.
    // Force a raw copy by making them match (no conversion!).
    // We can't change the TextureGpu format now, so we change dstOgrePf
    if (Ogre::PixelFormatGpuUtils::isSRgb(texture->getPixelFormat()))
      dstOgrePf = Ogre::PixelFormatGpuUtils::getEquivalentSRGB(dstOgrePf);
    else
      dstOgrePf = Ogre::PixelFormatGpuUtils::getEquivalentLinear(dstOgrePf);
  }

  Ogre::TextureBox dstBox(
    texture->getInternalWidth(), texture->getInternalHeight(),
    texture->getDepth(), texture->getNumSlices(),
    static_cast<uint32_t>(
      Ogre::PixelFormatGpuUtils::getBytesPerPixel(dstOgrePf)),
    static_cast<uint32_t>(Ogre::PixelFormatGpuUtils::getSizeBytes(
      texture->getInternalWidth(), 1u, 1u, 1u, dstOgrePf, 1u)),
    static_cast<uint32_t>(Ogre::PixelFormatGpuUtils::getSizeBytes(
      texture->getInternalWidth(), texture->getInternalHeight(), 1u, 1u,
      dstOgrePf, 1u)));
  dstBox.data = _image.Data();

  Ogre::Image2::copyContentsToMemory(texture, texture->getEmptyBox(0u), dstBox,
                                     dstOgrePf);
}

//////////////////////////////////////////////////
math::Vector3d Ogre2WideAngleCamera::Project3d(const math::Vector3d &_pt) const
{
  using namespace Ogre;

  this->dataPtr->ogreCamera->setAspectRatio(1.0f);
  this->dataPtr->ogreCamera->setFOVy(Ogre::Degree(90));
  const Quaternion oldCameraOrientation(
    this->dataPtr->ogreCamera->getOrientation());

  // project world point to camera clip space.
  const Matrix4 projMatrix = this->dataPtr->ogreCamera->getProjectionMatrix();
  // const Vector4 basePos = viewProj * Vector4(Ogre2Conversions::Convert(_pt));
  const Vector3 basePos = Ogre2Conversions::Convert(_pt);

  // project onto cubemap face then onto
  gz::math::Vector3d screenPos;
  // loop through all env cameras can find the one that sees the 3d world point
  for (unsigned int i = 0u; i < kWideAngleNumCubemapFaces; ++i)
  {
    // project world point to camera clip space for each face.
    this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation *
                                              kCubemapRotations[i]);
    const Matrix4 viewProj =
      projMatrix * this->dataPtr->ogreCamera->getViewMatrix();
    Vector4 pos = viewProj * Vector4(basePos);
    pos.x /= pos.w;
    pos.y /= pos.w;
    pos.z /= pos.w;
    // check if point is visible
    if (std::fabs(pos.x) <= 1 && std::fabs(pos.y) <= 1 && std::fabs(pos.z) <= 1)
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
      double theta =
        std::atan2(std::sqrt(dir.X() * dir.X() + dir.Y() * dir.Y()), dir.Z());
      double phi = std::atan2(dir.Y(), dir.X());
      // this also works:
      // double theta = std::acos(dir.Z());
      // double phi = std::asin(dir.Y() / std::sin(theta));

      double f = this->Lens().F();
      double fov = this->HFOV().Radian();
      // recompute f if scale to HFOV is true
      if (this->Lens().ScaleToHFOV())
      {
        double param = (fov / 2.0) / this->Lens().C2() + this->Lens().C3();
        double funRes =
          this->Lens().ApplyMappingFunction(static_cast<float>(param));
        f = 1.0 / (this->Lens().C1() * funRes);
      }

      // Apply fisheye lens mapping function
      // r is distance of point from image center
      double r = this->Lens().C1() * f *
                 this->Lens().ApplyMappingFunction(theta / this->Lens().C2() +
                                                   this->Lens().C3());

      // compute projected x and y in clip space
      double x = cos(phi) * r;
      double y = sin(phi) * r;

      const uint32_t vpWidth =
        this->dataPtr->ogreStitchTexture[kStichFinalTexture]->getWidth();
      const uint32_t vpHeight =
        this->dataPtr->ogreStitchTexture[kStichFinalTexture]->getHeight();
      // env cam cube map texture is square and likely to be different size from
      // viewport. We need to adjust projected pos based on aspect ratio
      double asp = static_cast<double>(vpWidth) / static_cast<double>(vpHeight);
      y *= asp;

      // convert to screen space
      screenPos.X() = ((x / 2.0) + 0.5) * vpWidth;
      screenPos.Y() = (1 - ((y / 2.0) + 0.5)) * vpHeight;

      // r will be > 1.0 if point is not visible (outside of image)
      screenPos.Z() = r;

      this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation);

      return screenPos;
    }
  }

  this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation);

  return screenPos;
}

//////////////////////////////////////////////////
Ogre::Ray Ogre2WideAngleCamera::CameraToViewportRay(
  const math::Vector2d &_screenPos, uint32_t _faceIdx)
{
  this->dataPtr->ogreCamera->setAspectRatio(1.0f);
  this->dataPtr->ogreCamera->setFOVy(Ogre::Degree(90));
  const Ogre::Quaternion oldCameraOrientation(
    this->dataPtr->ogreCamera->getOrientation());

  this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation *
                                            kCubemapRotations[_faceIdx]);

  Ogre::Ray ray = this->dataPtr->ogreCamera->getCameraToViewportRay(
    static_cast<Ogre::Real>(_screenPos.X()),
    static_cast<Ogre::Real>(_screenPos.Y()));

  this->dataPtr->ogreCamera->setOrientation(oldCameraOrientation);

  return ray;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::PostRender()
{
  for (RenderPassPtr &pass : this->dataPtr->renderPasses)
  {
    pass->PostRender();
  }
  for (RenderPassPtr &pass : this->dataPtr->finalStitchRenderPasses)
  {
    pass->PostRender();
  }

  if (this->dataPtr->newImageFrame.ConnectionCount() <= 0u)
    return;

  PixelFormat format = this->ImageFormat();
  const unsigned int width = this->ImageWidth();
  const unsigned int height = this->ImageHeight();
  unsigned int channelCount = PixelUtil::ChannelCount(format);
  unsigned int bytesPerChannel = PixelUtil::BytesPerChannel(format);

  // blit data from gpu to cpu
  Ogre::TextureGpu *texture =
      this->dataPtr->ogreStitchTexture[kStichFinalTexture];
  void *rawData = nullptr;
  Ogre::Image2 ogreImage;
  if (format == PF_R8G8B8)
  {
    ogreImage.convertFromTexture(texture, 0u, 0u);
    Ogre::TextureBox box = ogreImage.getData(0u);

    // Convert in-place from RGBA32 to RGB24 reusing the same memory region.
    // The data contained will no longer be meaningful to Image2, but that
    // class will no longer manipulate that data. We also store it contiguously
    // (which is what gazebo expects), instead of aligning rows to 4 bytes like
    // Ogre does. This saves RAM and lots of bandwidth.
    uint8_t *RESTRICT_ALIAS rgb24 =
      reinterpret_cast<uint8_t * RESTRICT_ALIAS>(box.data);
    for (size_t y = 0; y < box.height; ++y)
    {
      uint8_t *RESTRICT_ALIAS rgba32 =
        reinterpret_cast<uint8_t * RESTRICT_ALIAS>(box.at(0u, y, 0u));
      for (size_t x = 0; x < box.width; ++x)
      {
        *rgb24++ = *rgba32++;
        *rgb24++ = *rgba32++;
        *rgb24++ = *rgba32++;
        ++rgba32;
      }
    }
    rawData = box.data;
  }
  else
  {
    // convert to destination format
    Ogre::PixelFormatGpu dstOgrePf = Ogre2Conversions::Convert(format);
    Ogre::TextureBox dstBox(
      texture->getInternalWidth(), texture->getInternalHeight(),
      texture->getDepth(), texture->getNumSlices(),
      static_cast<uint32_t>(
        Ogre::PixelFormatGpuUtils::getBytesPerPixel(dstOgrePf)),
      static_cast<uint32_t>(Ogre::PixelFormatGpuUtils::getSizeBytes(
        texture->getInternalWidth(), 1u, 1u, 1u, dstOgrePf, 1u)),
      static_cast<uint32_t>(Ogre::PixelFormatGpuUtils::getSizeBytes(
        texture->getInternalWidth(), texture->getInternalHeight(), 1u, 1u,
        dstOgrePf, 1u)));
    if (!this->dataPtr->dstImgData)
    {
      this->dataPtr->dstImgData = std::make_unique<unsigned char []>(
          width * height * channelCount * bytesPerChannel);
    }
    dstBox.data = this->dataPtr->dstImgData.get();
    Ogre::Image2::copyContentsToMemory(texture, texture->getEmptyBox(0u),
                                       dstBox, dstOgrePf);
    rawData = dstBox.data;
  }
  this->dataPtr->newImageFrame(reinterpret_cast<uint8_t *>(rawData), width,
                               height, channelCount,
                               PixelUtil::Name(format));

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
common::ConnectionPtr Ogre2WideAngleCamera::ConnectNewWideAngleFrame(
  std::function<void(const unsigned char *, unsigned int, unsigned int,
                     unsigned int, const std::string &)>
    _subscriber)
{
  return this->dataPtr->newImageFrame.Connect(_subscriber);
}

//////////////////////////////////////////////////
RenderTargetPtr Ogre2WideAngleCamera::RenderTarget() const
{
  return this->dataPtr->wideAngleTexture;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::PrepareForCubemapFacePass(
  Ogre::CompositorPass *_pass)
{
  this->dataPtr->ogreCamera->setFOVy(Ogre::Degree(90));

  auto const &bgColor = this->scene->BackgroundColor();
  _pass->getRenderPassDesc()->setClearColour(
    Ogre2Conversions::Convert(bgColor));
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::PrepareForFinalPass(Ogre::Pass *_pass)
{
  const double ratio = static_cast<double>(this->ImageWidth()) /
                       static_cast<double>(this->ImageHeight());
  const double vfov = 2.0 * atan(tan(this->HFOV().Radian() / 2.0) / ratio);

  // Setting the fov is likely not necessary in the final pass but
  // clamp to max fov supported by ogre to avoid issues with building the
  // frustum
  this->dataPtr->ogreCamera->setFOVy(Ogre::Radian(
      Ogre::Real(std::clamp(vfov, 0.0, GZ_PI))));

  const float localHfov = static_cast<float>(this->HFOV().Radian());

  Ogre::GpuProgramParametersSharedPtr psParams =
    _pass->getFragmentProgramParameters();

  psParams->setNamedConstant("c1", static_cast<Ogre::Real>(this->Lens().C1()));
  psParams->setNamedConstant("c2", static_cast<Ogre::Real>(this->Lens().C2()));
  psParams->setNamedConstant("c3", static_cast<Ogre::Real>(this->Lens().C3()));

  if (this->Lens().ScaleToHFOV())
  {
    float param = (localHfov / 2) / this->Lens().C2() + this->Lens().C3();
    float funRes = this->Lens().ApplyMappingFunction(static_cast<float>(param));

    float newF = 1.0f / (this->Lens().C1() * funRes);

    psParams->setNamedConstant("f", static_cast<Ogre::Real>(newF));
  }
  else
  {
    psParams->setNamedConstant("f", static_cast<Ogre::Real>(this->Lens().F()));
  }

  auto vecFun = this->Lens().MappingFunctionAsVector3d();

  psParams->setNamedConstant("fun",
                             Ogre::Vector3(vecFun.X(), vecFun.Y(), vecFun.Z()));

  psParams->setNamedConstant(
    "cutOffAngle", static_cast<Ogre::Real>(this->Lens().CutOffAngle()));

  Ogre::GpuProgramParametersSharedPtr vsParams =
    _pass->getVertexProgramParameters();
  vsParams->setNamedConstant("ratio", static_cast<Ogre::Real>(ratio));
}

//////////////////////////////////////////////////
void Ogre2WideAngleCameraWorkspaceListenerPrivate::passPreExecute(
  Ogre::CompositorPass *_pass)
{
  const uint32_t identifier = _pass->getDefinition()->mIdentifier;
  if (identifier == kWideAngleCameraCubemapPassId)
  {
    this->owner.PrepareForCubemapFacePass(_pass);
  }
  else if (identifier == kWideAngleCameraQuadPassId)
  {
    GZ_ASSERT(dynamic_cast<Ogre::CompositorPassQuad *>(_pass),
              "Impossible! Corrupted memory? wide_angle_camera.compositor out "
              "of sync?");
    Ogre::CompositorPassQuad *passQuad =
      static_cast<Ogre::CompositorPassQuad *>(_pass);

    Ogre::Pass *pass = passQuad->getPass();

    this->owner.PrepareForFinalPass(pass);
  }
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::SetBackgroundMaterial(MaterialPtr _material)
{
  this->dataPtr->backgroundMaterial = _material;
  this->dataPtr->backgroundMaterialDirty = true;
}

//////////////////////////////////////////////////
MaterialPtr Ogre2WideAngleCamera::BackgroundMaterial() const
{
  return this->dataPtr->backgroundMaterial;
}

//////////////////////////////////////////////////
void Ogre2WideAngleCamera::UpdateBackgroundMaterial()
{
  if (!this->dataPtr->backgroundMaterialDirty)
    return;

  bool validBackground = this->dataPtr->backgroundMaterial &&
      !this->dataPtr->backgroundMaterial->EnvironmentMap().empty();

  if (validBackground)
  {
    Ogre::MaterialManager &matManager = Ogre::MaterialManager::getSingleton();
    std::string skyMatName = this->dataPtr->kSkyboxMaterialName + "_"
        + this->Name();
    auto mat = matManager.getByName(skyMatName);
    if (!mat)
    {
      auto skyboxMat = matManager.getByName(this->dataPtr->kSkyboxMaterialName);
      if (!skyboxMat)
      {
        gzerr << "Unable to find skybox material" << std::endl;
        return;
      }
      mat = skyboxMat->clone(skyMatName);
    }
    Ogre::TextureUnitState *texUnit =
        mat->getTechnique(0u)->getPass(0u)->getTextureUnitState(0u);
    texUnit->setTextureName(this->dataPtr->backgroundMaterial->EnvironmentMap(),
        Ogre::TextureTypes::TypeCube);
    texUnit->setHardwareGammaEnabled(false);
  }

  this->dataPtr->backgroundMaterialDirty = false;
}
