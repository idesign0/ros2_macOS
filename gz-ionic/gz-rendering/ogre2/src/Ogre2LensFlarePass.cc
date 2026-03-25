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
#include "gz/rendering/ogre2/Ogre2LensFlarePass.hh"

#include <gz/common/Util.hh>

#include "gz/rendering/RayQuery.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2Light.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2WideAngleCamera.hh"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/OgreCompositorWorkspaceListener.h>
#include <Compositor/Pass/OgreCompositorPass.h>
#include <Compositor/Pass/PassQuad/OgreCompositorPassQuad.h>
#include <OgreCamera.h>
#include <OgreGpuProgram.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreVector3.h>
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
/// via Ogre2LensFlarePass.
class GZ_RENDERING_OGRE2_HIDDEN Ogre2LensFlarePassWorkspaceListenerPrivate
    final : public Ogre::CompositorWorkspaceListener
{
  public: gz::rendering::Ogre2LensFlarePass &owner;

  /// \brief constructor
  /// \param[in] _scene the scene manager responsible for rendering
  public: explicit Ogre2LensFlarePassWorkspaceListenerPrivate(
        gz::rendering::Ogre2LensFlarePass &_owner) :
    owner(_owner)
  {
  }

  /// \brief Called when each pass is about to be executed.
  /// \param[in] _pass Ogre pass which is about to execute
  public: void passPreExecute(Ogre::CompositorPass *_pass) override;
};
}
}
}
// clang-format on

/// \brief Private data for the Ogre2LensFlarePass class
class gz::rendering::Ogre2LensFlarePass::Implementation
{
  // clang-format off

  /// \brief Position of light in world frame
  public: math::Vector3d lightWorldPos;

  /// \brief Color of lens flare.
  public: math::Vector3d color = math::Vector3d(1.0, 1.0, 1.0);

  /// \brief Scale of lens flare.
  public: double scale = 1.0;

  /// \brief Number of steps to take in each
  /// direction when checking for occlusion.
  public: double occlusionSteps = 10.0;

  /// \brief Current Camera rendering
  public: CameraPtr currentCamera;

  /// \brief Current Face index being rendered. In range [0; 6)
  public: uint32_t currentFaceIdx = 1u;

  /// \brief RayQuery to perform occlusion tests
  public: RayQueryPtr rayQuery;

  /// \brief See Ogre2LensFlarePassWorkspaceListenerPrivate
  public: Ogre2LensFlarePassWorkspaceListenerPrivate workspaceListener;

  public: explicit Implementation(gz::rendering::Ogre2LensFlarePass &_owner) :
    workspaceListener(_owner)
  {
  }
  // clang-format on
};

using namespace gz;
using namespace rendering;

// Arbitrary values, but they must be in sync with the Compositor script file
static constexpr uint32_t kLensFlareNodePassQuadId = 98744413u;

//////////////////////////////////////////////////
Ogre2LensFlarePass::Ogre2LensFlarePass() :
  dataPtr(utils::MakeUniqueImpl<Implementation>(*this))
{
  this->ogreCompositorNodeDefName = "LensFlareNode";
  this->afterStitching = true;
}

//////////////////////////////////////////////////
Ogre2LensFlarePass::~Ogre2LensFlarePass()
{
  this->Destroy();
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::Destroy()
{
  this->dataPtr->rayQuery.reset();
  this->dataPtr->currentCamera.reset();
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::Init(ScenePtr _scene)
{
  this->scene = std::dynamic_pointer_cast<Ogre2Scene>(_scene);
  this->dataPtr->rayQuery = _scene->CreateRayQuery();
  this->dataPtr->rayQuery->SetPreferGpu(false);
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::PreRender(const CameraPtr &_camera)
{
  if (!this->enabled || this->light == nullptr)
    return;

  // use light's world position for lens flare position
  DirectionalLight *dirLight =
    dynamic_cast<DirectionalLight *>(this->light.get());
  if (dirLight)
  {
    // Directional lights misuse position as a direction.
    // The large multiplier is for occlusion testing and assumes the light
    // is very far away. Larger values cause the light to disappear on
    // some frames for some unknown reason.
    this->dataPtr->lightWorldPos =
      -(this->light->WorldPose().Rot() * dirLight->Direction()) * 100000.0;
  }
  else
    this->dataPtr->lightWorldPos = this->light->WorldPose().Pos();

  this->dataPtr->currentCamera = _camera;
  this->dataPtr->currentFaceIdx = 0u;
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::PostRender()
{
  if (!this->enabled || this->light == nullptr)
    return;

  Ogre2WideAngleCameraPtr wideAngleCamera =
    std::dynamic_pointer_cast<Ogre2WideAngleCamera>(
      this->dataPtr->currentCamera);
  // WideAngleCamera is supposed to rendered 6 times.
  // Normal cameras are supposed to be rendered 1 time.
  GZ_ASSERT((!wideAngleCamera && this->dataPtr->currentFaceIdx == 1u) ||
              (wideAngleCamera && this->WideAngleCameraAfterStitching() &&
               this->dataPtr->currentFaceIdx == 1u) ||
              (wideAngleCamera && !this->WideAngleCameraAfterStitching() &&
               this->dataPtr->currentFaceIdx == 6u),
            "The lens flare pass was done more times than expected");
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::SetScale(const double _scale)
{
  this->dataPtr->scale = _scale;
}

//////////////////////////////////////////////////
double Ogre2LensFlarePass::Scale() const
{
  return this->dataPtr->scale;
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::SetColor(const math::Vector3d &_color)
{
  this->dataPtr->color = _color;
}

//////////////////////////////////////////////////
const math::Vector3d &Ogre2LensFlarePass::Color() const
{
  return this->dataPtr->color;
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::SetOcclusionSteps(const uint32_t _occlusionSteps)
{
  this->dataPtr->occlusionSteps = static_cast<double>(_occlusionSteps);
}

//////////////////////////////////////////////////
uint32_t Ogre2LensFlarePass::OcclusionSteps() const
{
  return static_cast<uint32_t>(this->dataPtr->occlusionSteps);
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::WorkspaceAdded(Ogre::CompositorWorkspace *_workspace)
{
  _workspace->addListener(&this->dataPtr->workspaceListener);
}

//////////////////////////////////////////////////
void Ogre2LensFlarePass::WorkspaceRemoved(Ogre::CompositorWorkspace *_workspace)
{
  _workspace->removeListener(&this->dataPtr->workspaceListener);
}

//////////////////////////////////////////////////
double Ogre2LensFlarePass::OcclusionScale(const math::Vector3d &_imgPos,
                                          uint32_t _faceIdx)
{
  if (std::abs(this->dataPtr->occlusionSteps) <= 1e-7)
  {
    return this->dataPtr->scale;
  }

  Ogre2WideAngleCameraPtr wideAngleCamera =
    std::dynamic_pointer_cast<Ogre2WideAngleCamera>(
      this->dataPtr->currentCamera);
  if (wideAngleCamera)
  {
    this->dataPtr->rayQuery->SetFromCamera(
      wideAngleCamera, _faceIdx, math::Vector2d(_imgPos.X(), _imgPos.Y()));
  }
  else
  {
    this->dataPtr->rayQuery->SetFromCamera(
      this->dataPtr->currentCamera, math::Vector2d(_imgPos.X(), _imgPos.Y()));
  }

  const math::Vector3d lightWorldPos = this->dataPtr->lightWorldPos;

  {
    // check center point
    // if occluded than set scale to 0
    RayQueryResult result = this->dataPtr->rayQuery->ClosestPoint(false);
    bool intersect = result.distance >= 0.0;
    if (intersect &&
        (result.point.SquaredLength() < lightWorldPos.SquaredLength()))
    {
      return 0;
    }
  }

  unsigned int rays = 0;
  unsigned int occluded = 0u;
  // work in normalized device coordinates
  // lens flare's halfSize is just an approximated value
  const double halfSize = 0.05 * this->dataPtr->scale;
  const double steps = this->dataPtr->occlusionSteps;
  const double stepSize = halfSize * 2 / steps;
  const double cx = _imgPos.X();
  const double cy = _imgPos.Y();
  const double startx = cx - halfSize;
  const double starty = cy - halfSize;
  const double endx = cx + halfSize;
  const double endy = cy + halfSize;
  // do sparse ray cast occlusion check
  for (double i = starty; i < endy; i += stepSize)
  {
    for (double j = startx; j < endx; j += stepSize)
    {
      if (wideAngleCamera)
      {
        this->dataPtr->rayQuery->SetFromCamera(wideAngleCamera, _faceIdx,
                                               math::Vector2d(j, i));
      }
      else
      {
        this->dataPtr->rayQuery->SetFromCamera(this->dataPtr->currentCamera,
                                               math::Vector2d(j, i));
      }
      RayQueryResult result = this->dataPtr->rayQuery->ClosestPoint(false);
      bool intersect = result.distance >= 0.0;
      if (intersect &&
          (result.point.SquaredLength() < lightWorldPos.SquaredLength()))
      {
        occluded++;
      }

      rays++;
    }
  }
  double s = static_cast<double>(rays - occluded) / static_cast<double>(rays);
  return s * this->dataPtr->scale;
}

//////////////////////////////////////////////////
void Ogre2LensFlarePassWorkspaceListenerPrivate::passPreExecute(
  Ogre::CompositorPass *_pass)
{
  if (!this->owner.enabled)
    return;

  const Ogre::CompositorPassDef *passDef = _pass->getDefinition();
  const uint32_t identifier = passDef->mIdentifier;
  if (identifier != kLensFlareNodePassQuadId)
    return;

  using namespace Ogre;

  GZ_ASSERT(dynamic_cast<CompositorPassQuad *>(_pass),
            "Impossible! Corrupted memory? lens_flare.compositor out of sync?");

  CompositorPassQuad *passQuad = static_cast<CompositorPassQuad *>(_pass);

  Ogre::Camera *camera = passQuad->getCamera();

  Ogre2WideAngleCameraPtr wideAngleCamera =
    std::dynamic_pointer_cast<Ogre2WideAngleCamera>(
      this->owner.dataPtr->currentCamera);

  const Ogre::Radian oldFov(camera->getFOVy());
  const Ogre::Real oldAr(camera->getAspectRatio());

  if (this->owner.WideAngleCameraAfterStitching() && wideAngleCamera)
  {
    camera->setFOVy(Ogre::Degree(90));
    camera->setAspectRatio(Ogre::Real(1.0f));
  }

  Pass *pass = passQuad->getPass();

  const Matrix4 viewProj =
    camera->getProjectionMatrix() * camera->getViewMatrix();
  const Vector4 pos =
    viewProj *
    Vector4(Ogre2Conversions::Convert(this->owner.dataPtr->lightWorldPos));

  // normalize x and y, keep z for visibility test
  Vector3 lightPos;
  lightPos.x = pos.x / pos.w;
  lightPos.y = pos.y / pos.w;
  // Make lightPos.z > 0 mean we're in front of near plane
  // since pos.z is in range [-|pos.w|; |pos.w|]
  lightPos.z = pos.z + std::abs(pos.w);

  double lensFlareScale = 1.0;
  if (lightPos.z >= 0.0)
  {
    uint32_t currentFaceIdx = this->owner.dataPtr->currentFaceIdx;
    if (this->owner.WideAngleCameraAfterStitching())
    {
      currentFaceIdx = 4u;
    }

    lensFlareScale = this->owner.OcclusionScale(
      Ogre2Conversions::Convert(lightPos), currentFaceIdx);
  }

  ++this->owner.dataPtr->currentFaceIdx;

  GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();

  psParams->setNamedConstant("vpAspectRatio", camera->getAspectRatio());
  psParams->setNamedConstant("lightPos", lightPos);
  psParams->setNamedConstant("scale", static_cast<Real>(lensFlareScale));
  psParams->setNamedConstant(
    "color", Ogre2Conversions::Convert(this->owner.dataPtr->color));

  if (this->owner.WideAngleCameraAfterStitching() && wideAngleCamera)
  {
    camera->setFOVy(oldFov);
    camera->setAspectRatio(oldAr);
  }
}

GZ_RENDERING_REGISTER_RENDER_PASS(Ogre2LensFlarePass, LensFlarePass)
