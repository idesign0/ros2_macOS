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
#include "gz/rendering/ogre/OgreLensFlarePass.hh"

#include <gz/common/Util.hh>

#include "gz/rendering/RayQuery.hh"
#include "gz/rendering/RenderPassSystem.hh"
#include "gz/rendering/ogre/OgreCamera.hh"
#include "gz/rendering/ogre/OgreConversions.hh"
#include "gz/rendering/ogre/OgreLight.hh"
#include "gz/rendering/ogre/OgreScene.hh"
#include "gz/rendering/ogre/OgreWideAngleCamera.hh"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
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
class GZ_RENDERING_OGRE_HIDDEN OgreLensFlareCompositorListenerPrivate
    final : public Ogre::CompositorInstance::Listener
{
  public: gz::rendering::OgreLensFlarePass &owner;

  /// \brief constructor
  /// \param[in] _owner our creator for direct access to variables
  public: explicit OgreLensFlareCompositorListenerPrivate(
        gz::rendering::OgreLensFlarePass &_owner) :
    owner(_owner)
  {
  }

  /// \brief Callback that OGRE will invoke for us on each render call
  /// \param[in] _passID OGRE material pass ID.
  /// \param[in] _mat Pointer to OGRE material.
  public: void notifyMaterialRender(unsigned int _passId,
                                    Ogre::MaterialPtr &_mat) override;
};
}
}
}
// clang-format on

/// \brief Private data for the OgreLensFlarePass class
class gz::rendering::OgreLensFlarePass::Implementation
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

  /// \brief See OgreLensFlarePassPrivate
  public: OgreLensFlareCompositorListenerPrivate compositorListener;

  /// \brief Lens Flare compositor.
  public: Ogre::CompositorInstance *
    lensFlareInstance[kMaxOgreRenderPassCameras] = {};

  public: explicit Implementation(gz::rendering::OgreLensFlarePass &_owner) :
    compositorListener(_owner)
  {
  }
  // clang-format on
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
OgreLensFlarePass::OgreLensFlarePass() :
  dataPtr(utils::MakeUniqueImpl<Implementation>(*this))
{
  this->afterStitching = true;
}

//////////////////////////////////////////////////
OgreLensFlarePass::~OgreLensFlarePass()
{
}

//////////////////////////////////////////////////
void OgreLensFlarePass::Init(ScenePtr _scene)
{
  this->scene = std::dynamic_pointer_cast<OgreScene>(_scene);
  this->dataPtr->rayQuery = _scene->CreateRayQuery();
}

//////////////////////////////////////////////////
void OgreLensFlarePass::Destroy()
{
  for (size_t i = 0u; i < kMaxOgreRenderPassCameras; ++i)
  {
    if (this->dataPtr->lensFlareInstance[i])
    {
      this->dataPtr->lensFlareInstance[i]->setEnabled(false);
      this->dataPtr->lensFlareInstance[i]->removeListener(
        &this->dataPtr->compositorListener);
      Ogre::CompositorManager::getSingleton().removeCompositor(
        this->ogreCamera[i]->getViewport(), "RenderPass/LensFlare");

      this->dataPtr->lensFlareInstance[i] = nullptr;
    }
  }
}

//////////////////////////////////////////////////
void OgreLensFlarePass::CreateRenderPass()
{
  if (!this->ogreCamera[0])
  {
    gzerr << "No camera set for applying Lens Flare Pass" << std::endl;
    return;
  }

  if (this->dataPtr->lensFlareInstance[0])
  {
    gzwarn << "Lens Flare pass already created. " << std::endl;
    return;
  }

  for (size_t i = 0u; i < kMaxOgreRenderPassCameras; ++i)
  {
    if (this->ogreCamera[i])
    {
      // create compositor instance
      this->dataPtr->lensFlareInstance[i] =
        Ogre::CompositorManager::getSingleton().addCompositor(
          this->ogreCamera[i]->getViewport(), "RenderPass/LensFlare");
      this->dataPtr->lensFlareInstance[i]->setEnabled(this->enabled);
      this->dataPtr->lensFlareInstance[i]->addListener(
        &this->dataPtr->compositorListener);
    }
  }
}

//////////////////////////////////////////////////
void OgreLensFlarePass::PreRender(const CameraPtr &_camera)
{
  if (!this->dataPtr->lensFlareInstance[0])
    return;

  for (size_t i = 0u; i < kMaxOgreRenderPassCameras; ++i)
  {
    if (this->dataPtr->lensFlareInstance[i] &&
        this->enabled != this->dataPtr->lensFlareInstance[i]->getEnabled())
    {
      this->dataPtr->lensFlareInstance[i]->setEnabled(this->enabled);
    }
  }

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
void OgreLensFlarePass::PostRender()
{
  if (!this->enabled || this->light == nullptr)
    return;

  OgreWideAngleCameraPtr wideAngleCamera =
    std::dynamic_pointer_cast<OgreWideAngleCamera>(
      this->dataPtr->currentCamera);
  // WideAngleCamera is supposed to rendered 6 times. Nothin more, nothing less.
  // Normal cameras are supposed to be rendered 1 time.
  GZ_ASSERT((!wideAngleCamera && this->dataPtr->currentFaceIdx == 1u) ||
              (wideAngleCamera && this->WideAngleCameraAfterStitching() &&
               this->dataPtr->currentFaceIdx == 1u) ||
              (wideAngleCamera && !this->WideAngleCameraAfterStitching() &&
               this->dataPtr->currentFaceIdx == 6u),
            "The lens flare pass was done more times than expected");
}

//////////////////////////////////////////////////
void OgreLensFlarePass::SetScale(const double _scale)
{
  this->dataPtr->scale = _scale;
}

//////////////////////////////////////////////////
double OgreLensFlarePass::Scale() const
{
  return this->dataPtr->scale;
}

//////////////////////////////////////////////////
void OgreLensFlarePass::SetColor(const math::Vector3d &_color)
{
  this->dataPtr->color = _color;
}

//////////////////////////////////////////////////
const math::Vector3d &OgreLensFlarePass::Color() const
{
  return this->dataPtr->color;
}

//////////////////////////////////////////////////
void OgreLensFlarePass::SetOcclusionSteps(const uint32_t _occlusionSteps)
{
  this->dataPtr->occlusionSteps = static_cast<double>(_occlusionSteps);
}

//////////////////////////////////////////////////
uint32_t OgreLensFlarePass::OcclusionSteps() const
{
  return static_cast<uint32_t>(this->dataPtr->occlusionSteps);
}

//////////////////////////////////////////////////
double OgreLensFlarePass::OcclusionScale(const math::Vector3d &_imgPos,
                                         uint32_t _faceIdx)
{
  if (std::abs(this->dataPtr->occlusionSteps) <= 1e-7)
  {
    return this->dataPtr->scale;
  }

  OgreWideAngleCameraPtr wideAngleCamera =
    std::dynamic_pointer_cast<OgreWideAngleCamera>(
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
void OgreLensFlareCompositorListenerPrivate::notifyMaterialRender(
  unsigned int _passId, Ogre::MaterialPtr &_mat)
{
  if (!this->owner.enabled)
    return;

  using namespace Ogre;

  // These calls are setting parameters that are declared in two places:
  // 1. media/materials/scripts/lens_flare.material, in
  //    fragment_program LensFlareFS
  // 2. media/materials/scripts/lens_flare_fs.glsl
  Ogre::Technique *technique = _mat->getTechnique(0);
  GZ_ASSERT(technique, "Null OGRE material technique");
  Ogre::Pass *pass = technique->getPass(static_cast<uint16_t>(_passId));
  GZ_ASSERT(pass, "Null OGRE material pass");
  Ogre::GpuProgramParametersSharedPtr params =
    pass->getFragmentProgramParameters();

  const Ogre::Camera *camera;

  uint32_t currentFaceIdx = this->owner.dataPtr->currentFaceIdx;
  if (this->owner.WideAngleCameraAfterStitching())
  {
    currentFaceIdx = 4u;
  }

  {
    OgreWideAngleCamera *wideAngleCamera = dynamic_cast<OgreWideAngleCamera *>(
      this->owner.dataPtr->currentCamera.get());
    if (wideAngleCamera)
    {
      camera = wideAngleCamera->OgreEnvCameras()[currentFaceIdx];
    }
    else
    {
      camera =
        dynamic_cast<OgreCamera *>(this->owner.dataPtr->currentCamera.get())
          ->Camera();
    }
  }

  const Matrix4 viewProj =
    camera->getProjectionMatrix() * camera->getViewMatrix();
  const Vector4 pos =
    viewProj *
    Vector4(OgreConversions::Convert(this->owner.dataPtr->lightWorldPos));

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
    lensFlareScale = this->owner.OcclusionScale(
      OgreConversions::Convert(lightPos), currentFaceIdx);
  }

  ++this->owner.dataPtr->currentFaceIdx;

  GpuProgramParametersSharedPtr psParams = pass->getFragmentProgramParameters();

  psParams->setNamedConstant("vpAspectRatio", camera->getAspectRatio());
  psParams->setNamedConstant("lightPos", lightPos);
  psParams->setNamedConstant("scale", static_cast<Real>(lensFlareScale));
  psParams->setNamedConstant(
    "color", OgreConversions::Convert(this->owner.dataPtr->color));
}

GZ_RENDERING_REGISTER_RENDER_PASS(OgreLensFlarePass, LensFlarePass)
