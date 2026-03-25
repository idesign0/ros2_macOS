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

#include "gz/rendering/ogre2/Ogre2GlobalIlluminationCiVct.hh"

#include "gz/rendering/ogre2/Ogre2Camera.hh"
#include "gz/rendering/ogre2/Ogre2Conversions.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <Hlms/Pbs/Vct/OgreVctCascadedVoxelizer.h>
#include <Hlms/Pbs/Vct/OgreVctImageVoxelizer.h>
#include <Hlms/Pbs/Vct/OgreVctLighting.h>
#include <OgreHlmsManager.h>
#include <OgreItem.h>
#include <OgreRoot.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

/// \brief Private data for the Ogre2CiVctCascadePrivate class
class GZ_RENDERING_OGRE2_HIDDEN gz::rendering::Ogre2CiVctCascadePrivate
{
  // clang-format off
  /// \brief Pointer to cascade setting
  public: Ogre::VctCascadeSetting *cascade = nullptr;
  // clang-format on
};

/// \brief Private data for the Ogre2GlobalIlluminationCiVct class
class GZ_RENDERING_OGRE2_HIDDEN
  gz::rendering::Ogre2GlobalIlluminationCiVctPrivate
{
  // clang-format off
  /// \brief Ogre's Voxelizer. In charge of voxelizing the scene
  /// at voxelizing phase time
  public: Ogre::VctCascadedVoxelizer *cascadedVoxelizer = nullptr;

  /// \brief Holds all cascade settings
  public: std::vector<CiVctCascadePtr> cascades;

  /// \brief See GlobalIlluminationCiVct::Bind
  public: CameraPtr bindCamera;

  /// \brief See GlobalIlluminationVct::SetParticipatingVisuals
  public: uint32_t participatingVisuals =
      GlobalIlluminationBase::ParticipatingVisualsFlags::STATIC_VISUALS |
      GlobalIlluminationBase::ParticipatingVisualsFlags::DYNAMIC_VISUALS;

  /// \brief See GlobalIlluminationCiVct::DebugVisualizationMode
  public: GlobalIlluminationCiVct::DebugVisualizationMode
    debugVisualizationMode = GlobalIlluminationCiVct::DVM_None;

  /// \brief See GlobalIlluminationCiVct::SetHighQuality
  /// This value is cached because it globally affects HlmsPbs
  public: bool highQuality = false;

  /// \brief Tracks if GlobalIlluminationCiVct::Start has been called
  public: bool started = false;
  // clang-format on
};

//////////////////////////////////////////////////
Ogre2CiVctCascade::Ogre2CiVctCascade() :
  dataPtr(std::make_unique<Ogre2CiVctCascadePrivate>())
{
}

//////////////////////////////////////////////////
void Ogre2CiVctCascade::Init(Ogre::VctCascadeSetting *_cascade,
                             const CiVctCascade *_ref)
{
  GZ_ASSERT(this->dataPtr->cascade == nullptr,
            "Calling Ogre2CiVctCascade::Init twice!");

  this->dataPtr->cascade = _cascade;

  if (_ref)
  {
    const Ogre2CiVctCascade *_refCascade =
      dynamic_cast<const Ogre2CiVctCascade *>(_ref);
    *this->dataPtr->cascade = *_refCascade->dataPtr->cascade;
  }
}

//////////////////////////////////////////////////
void Ogre2CiVctCascade::ReInit(Ogre::VctCascadeSetting *_cascade)
{
  GZ_ASSERT(this->dataPtr->cascade != nullptr,
            "Calling Ogre2CiVctCascade::ReInit without Init!");
  this->dataPtr->cascade = _cascade;
}

//////////////////////////////////////////////////
void Ogre2CiVctCascade::SetCorrectAreaLightShadows(
  bool _correctAreaLightShadows)
{
  this->dataPtr->cascade->bCorrectAreaLightShadows = _correctAreaLightShadows;
}

//////////////////////////////////////////////////
bool Ogre2CiVctCascade::CorrectAreaLightShadows() const
{
  return this->dataPtr->cascade->bCorrectAreaLightShadows;
}

//////////////////////////////////////////////////
void Ogre2CiVctCascade::SetAutoMultiplier(bool _autoMultiplier)
{
  this->dataPtr->cascade->bAutoMultiplier = _autoMultiplier;
}

//////////////////////////////////////////////////
bool Ogre2CiVctCascade::AutoMultiplier() const
{
  return this->dataPtr->cascade->bAutoMultiplier;
}

//////////////////////////////////////////////////
void Ogre2CiVctCascade::SetThinWallCounter(float _thinWallCounter)
{
  this->dataPtr->cascade->thinWallCounter = _thinWallCounter;
}

//////////////////////////////////////////////////
float Ogre2CiVctCascade::ThinWallCounter() const
{
  return this->dataPtr->cascade->thinWallCounter;
}

//////////////////////////////////////////////////
void Ogre2CiVctCascade::SetResolution(const uint32_t _resolution[3])
{
  for (int i = 0; i < 3; ++i)
    this->dataPtr->cascade->resolution[i] = _resolution[i];
}

//////////////////////////////////////////////////
const uint32_t *Ogre2CiVctCascade::Resolution() const
{
  return this->dataPtr->cascade->resolution;
}

//////////////////////////////////////////////////
void Ogre2CiVctCascade::SetOctantCount(const uint32_t _octants[3])
{
  for (int i = 0; i < 3; ++i)
    this->dataPtr->cascade->octantSubdivision[i] = _octants[i];
}

//////////////////////////////////////////////////
const uint32_t *Ogre2CiVctCascade::OctantCount() const
{
  return this->dataPtr->cascade->octantSubdivision;
}

//////////////////////////////////////////////////
void Ogre2CiVctCascade::SetAreaHalfSize(const gz::math::Vector3d &_areaHalfSize)
{
  this->dataPtr->cascade->areaHalfSize =
    Ogre2Conversions::Convert(_areaHalfSize);
}

//////////////////////////////////////////////////
gz::math::Vector3d Ogre2CiVctCascade::AreaHalfSize() const
{
  return Ogre2Conversions::Convert(this->dataPtr->cascade->areaHalfSize);
}

//////////////////////////////////////////////////
void Ogre2CiVctCascade::SetCameraStepSize(const gz::math::Vector3d &_stepSize)
{
  this->dataPtr->cascade->cameraStepSize = Ogre2Conversions::Convert(_stepSize);
}

//////////////////////////////////////////////////
gz::math::Vector3d Ogre2CiVctCascade::CameraStepSize() const
{
  return Ogre2Conversions::Convert(this->dataPtr->cascade->cameraStepSize);
}

//////////////////////////////////////////////////
Ogre2GlobalIlluminationCiVct::Ogre2GlobalIlluminationCiVct() :
  dataPtr(std::make_unique<Ogre2GlobalIlluminationCiVctPrivate>())
{
}

//////////////////////////////////////////////////
Ogre2GlobalIlluminationCiVct::~Ogre2GlobalIlluminationCiVct()
{
  this->Destroy();
}

//////////////////////////////////////////////////
Ogre2CiVctCascade::~Ogre2CiVctCascade()
{
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::Init()
{
  this->dataPtr->cascadedVoxelizer = new Ogre::VctCascadedVoxelizer();
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::Destroy()
{
  if (this->Enabled())
    this->SetEnabled(false);

  if (this->dataPtr->cascadedVoxelizer)
  {
    delete this->dataPtr->cascadedVoxelizer;
    this->dataPtr->cascadedVoxelizer = nullptr;
  }
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::SetMaxCascades(const uint32_t _maxCascades)
{
  if (!this->dataPtr->cascades.empty())
  {
    gzerr << "Can't call SetMaxCascades after AddCascade\n";
    return;
  }
  this->dataPtr->cascadedVoxelizer->reserveNumCascades(_maxCascades);
  this->dataPtr->cascades.reserve(_maxCascades);
}

//////////////////////////////////////////////////
CiVctCascadePtr Ogre2GlobalIlluminationCiVct::AddCascade(
  const CiVctCascade *_ref)
{
  const size_t cascadeIdx = this->dataPtr->cascades.size();

  // Add cascade to Ogre
  Ogre::VctCascadeSetting cascadeSetting;
  this->dataPtr->cascadedVoxelizer->addCascade(cascadeSetting);

  // Add cascade controller
  Ogre2CiVctCascade *newCascade = new Ogre2CiVctCascade();
  this->dataPtr->cascades.push_back(CiVctCascadePtr(newCascade));
  newCascade->Init(&this->dataPtr->cascadedVoxelizer->getCascade(cascadeIdx),
                   _ref);
  return this->dataPtr->cascades.back();
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::PopCascade()
{
  if (!this->dataPtr->cascades.empty())
  {
    this->dataPtr->cascadedVoxelizer->popCascade();
    this->dataPtr->cascades.pop_back();
  }
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::AutoCalculateStepSizes(
  const gz::math::Vector3d &_stepSize)
{
  this->dataPtr->cascadedVoxelizer->autoCalculateStepSizes(
    Ogre2Conversions::Convert(_stepSize));
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::SetConsistentCascadeSteps(
  bool _consistentCascadeSteps)
{
  this->dataPtr->cascadedVoxelizer->setConsistentCascadeSteps(
    _consistentCascadeSteps);
}

//////////////////////////////////////////////////
bool Ogre2GlobalIlluminationCiVct::ConsistentCascadeSteps() const
{
  return this->dataPtr->cascadedVoxelizer->getConsistentCascadeSteps();
}

//////////////////////////////////////////////////
bool Ogre2GlobalIlluminationCiVct::Started() const
{
  return this->dataPtr->started;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::Start(uint32_t _bounceCount,
                                         bool _anisotropic)
{
  Ogre::Root *ogreRoot = Ogre2RenderEngine::Instance()->OgreRoot();
  this->dataPtr->started = true;
  this->dataPtr->cascadedVoxelizer->init(ogreRoot->getRenderSystem(),
                                         ogreRoot->getHlmsManager(),
                                         _bounceCount, _anisotropic);
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::NewSettings(uint32_t _bounceCount,
                                               bool _anisotropic)
{
  this->dataPtr->cascadedVoxelizer->setNewSettings(_bounceCount, _anisotropic);
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::Reset()
{
  if (this->Enabled())
  {
    this->scene->SetActiveGlobalIllumination(nullptr);
  }

  Ogre::VctCascadedVoxelizer *oldCascadedVoxelizer =
    this->dataPtr->cascadedVoxelizer;
  Ogre::VctCascadedVoxelizer *newCascadedVoxelizer =
    new Ogre::VctCascadedVoxelizer();
  this->dataPtr->cascadedVoxelizer = newCascadedVoxelizer;

  const size_t numCascades = this->dataPtr->cascades.size();

  for (size_t cascadeIdx = 0u; cascadeIdx < numCascades; ++cascadeIdx)
  {
    // Copy all settings from old reference. We set ref.voxelizer to nullptr
    // to avoid Ogre asserting on us (that assert is meant to warn against
    // calling addCascade on an already-started system. Here's it's ok because
    // we are copying from a started system to an unstarted one)
    Ogre::VctCascadeSetting ref = oldCascadedVoxelizer->getCascade(cascadeIdx);
    ref.voxelizer = nullptr;
    newCascadedVoxelizer->addCascade(ref);
    Ogre2CiVctCascade *cascade = dynamic_cast<Ogre2CiVctCascade *>(
      this->dataPtr->cascades[cascadeIdx].get());
    cascade->ReInit(&newCascadedVoxelizer->getCascade(cascadeIdx));
  }

  delete oldCascadedVoxelizer;
  this->dataPtr->started = false;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::Bind(const CameraPtr &_camera)
{
  this->dataPtr->bindCamera = _camera;
}

//////////////////////////////////////////////////
bool Ogre2GlobalIlluminationCiVct::Anisotropic() const
{
  return this->dataPtr->cascadedVoxelizer->isAnisotropic();
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::SetHighQuality(bool _highQuality)
{
  if (this->Enabled())
  {
    Ogre::HlmsPbs *hlmsPbs = this->HlmsPbs();
    hlmsPbs->setVctFullConeCount(_highQuality);
  }
  this->dataPtr->highQuality = _highQuality;
}

//////////////////////////////////////////////////
bool Ogre2GlobalIlluminationCiVct::HighQuality() const
{
  return this->dataPtr->highQuality;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::Build()
{
  Ogre::SceneManager *sceneManager = this->scene->OgreSceneManager();
  sceneManager->updateSceneGraph();

  Ogre::VctCascadedVoxelizer *cascadedVoxelizer =
    this->dataPtr->cascadedVoxelizer;

  cascadedVoxelizer->removeAllItems();

  // Add all static Item from Ogre
  if (this->dataPtr->participatingVisuals & STATIC_VISUALS)
    cascadedVoxelizer->addAllItems(sceneManager, 0xffffffff, true);
  if (this->dataPtr->participatingVisuals & DYNAMIC_VISUALS)
    cascadedVoxelizer->addAllItems(sceneManager, 0xffffffff, false);

  sceneManager->updateSceneGraph();
  cascadedVoxelizer->setCameraPosition(
    Ogre2Conversions::Convert(this->dataPtr->bindCamera->WorldPosition()));
  cascadedVoxelizer->update(sceneManager);

  this->LightingChanged();
  this->SyncModeVisualizationMode();
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::UpdateLighting()
{
  this->LightingChanged();
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::UpdateCamera()
{
  Ogre::SceneManager *sceneManager = this->scene->OgreSceneManager();
  this->dataPtr->cascadedVoxelizer->setCameraPosition(
    Ogre2Conversions::Convert(this->dataPtr->bindCamera->WorldPosition()));
  this->dataPtr->cascadedVoxelizer->update(sceneManager);
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::SetEnabled(bool _enabled)
{
  Ogre::HlmsPbs *hlmsPbs = this->HlmsPbs();
  if (_enabled)
  {
    if (this->dataPtr->cascadedVoxelizer->getNumCascades() == 0u ||
        this->dataPtr->cascadedVoxelizer->getVctLighting(0u) == nullptr)
    {
      gzwarn
        << "GlobalIlluminationCiVct::Start not yet called. Cannot enable.\n";
      return;
    }

    GZ_ASSERT(hlmsPbs->getVctLighting() == nullptr ||
                hlmsPbs->getVctLighting() ==
                  this->dataPtr->cascadedVoxelizer->getVctLighting(0u),
              "There's already an active GI solution!");

    hlmsPbs->setVctLighting(
      this->dataPtr->cascadedVoxelizer->getVctLighting(0u));
    hlmsPbs->setVctFullConeCount(this->dataPtr->highQuality);
  }
  else
  {
    GZ_ASSERT(hlmsPbs->getVctLighting() == nullptr ||
                hlmsPbs->getVctLighting() ==
                  this->dataPtr->cascadedVoxelizer->getVctLighting(0u),
              "There's an active GI solution but isn't us!");
    hlmsPbs->setVctLighting(nullptr);
  }
}

//////////////////////////////////////////////////
bool Ogre2GlobalIlluminationCiVct::Enabled() const
{
  if (!this->dataPtr->cascadedVoxelizer)
    return false;

  if (this->dataPtr->cascadedVoxelizer->getNumCascades() == 0u ||
      this->dataPtr->cascadedVoxelizer->getVctLighting(0u) == nullptr)
  {
    return false;
  }

  Ogre::HlmsPbs *hlmsPbs = this->HlmsPbs();
  return hlmsPbs->getVctLighting() ==
         this->dataPtr->cascadedVoxelizer->getVctLighting(0u);
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::SetDebugVisualization(
  DebugVisualizationMode _dvm)
{
  this->dataPtr->debugVisualizationMode = _dvm;
  this->SyncModeVisualizationMode();
}

//////////////////////////////////////////////////
GlobalIlluminationCiVct::DebugVisualizationMode
Ogre2GlobalIlluminationCiVct::DebugVisualization() const
{
  return this->dataPtr->debugVisualizationMode;
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::LightingChanged()
{
  GZ_ASSERT(this->dataPtr->cascadedVoxelizer->getNumCascades() != 0u &&
              this->dataPtr->cascadedVoxelizer->getVctLighting(0u) != nullptr,
            "Incorrect usage detected. Did you call Build?");
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::SetBounceCount(uint32_t _bounceCount)
{
  this->NewSettings(_bounceCount, this->Anisotropic());
}

//////////////////////////////////////////////////
uint32_t Ogre2GlobalIlluminationCiVct::BounceCount() const
{
  return this->dataPtr->cascadedVoxelizer->getNumBounces();
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::SetParticipatingVisuals(uint32_t _mask)
{
  this->dataPtr->participatingVisuals = _mask;
}

//////////////////////////////////////////////////
uint32_t Ogre2GlobalIlluminationCiVct::ParticipatingVisuals() const
{
  return this->dataPtr->participatingVisuals;
}

//////////////////////////////////////////////////
Ogre::HlmsPbs *Ogre2GlobalIlluminationCiVct::HlmsPbs() const
{
  Ogre::Root *ogreRoot = Ogre2RenderEngine::Instance()->OgreRoot();
  Ogre::HlmsManager *hlmsManager = ogreRoot->getHlmsManager();

  Ogre::Hlms *hlms = hlmsManager->getHlms(Ogre::HLMS_PBS);
  GZ_ASSERT(dynamic_cast<Ogre::HlmsPbs *>(hlms),
            "Corruption or incorrect setup detected");
  return static_cast<Ogre::HlmsPbs *>(hlms);
}

//////////////////////////////////////////////////
void Ogre2GlobalIlluminationCiVct::SyncModeVisualizationMode()
{
  if (this->dataPtr->cascadedVoxelizer->getNumCascades() > 0u)
  {
    if (this->dataPtr->debugVisualizationMode <= DVM_Emissive)
    {
      this->dataPtr->cascadedVoxelizer->getCascade(0u)
        .voxelizer->setDebugVisualization(
          static_cast<Ogre::VctVoxelizerSourceBase::DebugVisualizationMode>(
            this->dataPtr->debugVisualizationMode),
          this->scene->OgreSceneManager());
    }
    else
    {
      this->dataPtr->cascadedVoxelizer->getCascade(0u)
        .voxelizer->setDebugVisualization(
          Ogre::VctVoxelizerSourceBase::DebugVisualizationNone,
          this->scene->OgreSceneManager());
    }

    if (this->dataPtr->cascadedVoxelizer->getVctLighting(0u))
    {
      this->dataPtr->cascadedVoxelizer->getVctLighting(0u)
        ->setDebugVisualization(
          this->dataPtr->debugVisualizationMode == DVM_Lighting,
          this->scene->OgreSceneManager());
    }
  }
}
