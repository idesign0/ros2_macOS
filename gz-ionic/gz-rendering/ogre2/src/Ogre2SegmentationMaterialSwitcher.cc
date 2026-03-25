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

#include "Ogre2SegmentationMaterialSwitcher.hh"

#include <algorithm>
#include <utility>
#include <vector>
#include <variant>

#include <gz/common/Console.hh>

#include "gz/rendering/ogre2/Ogre2Heightmap.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/ogre2/Ogre2Visual.hh"
#include "gz/rendering/RenderTypes.hh"

#include "Terra/Terra.h"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreItem.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
Ogre2SegmentationMaterialSwitcher::Ogre2SegmentationMaterialSwitcher(
  Ogre2ScenePtr _scene, SegmentationCamera *_camera)
{
  this->scene = _scene;
  this->segmentationCamera = _camera;
}

/////////////////////////////////////////////////
Ogre2SegmentationMaterialSwitcher::~Ogre2SegmentationMaterialSwitcher()
{
  this->segmentationCamera = nullptr;
}

/////////////////////////////////////////////////
bool Ogre2SegmentationMaterialSwitcher::IsTakenColor(const math::Color &_color)
{
  // Get the int value of the 24 bit color
  // Multiply by 255 as color values are normalized
  int64_t colorId = static_cast<int64_t>((_color.R() * 255) * 256 * 256 +
    (_color.G() * 255) * 256 + (_color.B() * 255));

  if (this->takenColors.count(colorId))
  {
    return true;
  }
  else
  {
    this->takenColors.insert(colorId);
    return false;
  }
}

/////////////////////////////////////////////////
Ogre::Vector4 Ogre2SegmentationMaterialSwitcher::ColorForVisual(
  const VisualPtr &_visual, std::string &_prevParentName)
{
  // get class user data
  Variant labelAny = _visual->UserData("label");
  int label;
  if (const int* labelPtr = std::get_if<int>(&labelAny))
  {
    label = *labelPtr;
  }
  else
  {
    // items with no class are considered background
    label = this->segmentationCamera->BackgroundLabel();
  }

  // sub item custom parameter to set the pixel color material
  Ogre::Vector4 customParameter;

  // Material Switching
  if (this->segmentationCamera->Type() == SegmentationType::ST_SEMANTIC)
  {
    if (this->segmentationCamera->IsColoredMap())
    {
      // semantic material (each pixel has item's color)
      math::Color color = this->LabelToColor(label);
      customParameter = Ogre::Vector4(color.R(), color.G(), color.B(), 1.0);
    }
    else
    {
      // labels ids material (each pixel has item's label)
      float labelColor = label / 255.0f;
      customParameter = Ogre::Vector4(labelColor, labelColor, labelColor, 1.0);
    }
  }
  else if (this->segmentationCamera->Type() == SegmentationType::ST_PANOPTIC)
  {
    auto itemName = _visual->Name();
    std::string parentName = this->TopLevelModelVisual(_visual)->Name();

    auto it = this->instancesCount.find(label);
    if (it == this->instancesCount.end())
      it = this->instancesCount.insert(std::make_pair(label, 0)).first;

    // Multi link model has many links with the same first name and should
    // have the same pixels color
    bool isMultiLink = false;
    if (parentName == _prevParentName)
    {
      isMultiLink = true;
    }
    else
    {
      it->second++;
      _prevParentName = parentName;
    }

    const int instanceCount = it->second;

    if (this->segmentationCamera->IsColoredMap())
    {
      math::Color color;
      if (label == this->segmentationCamera->BackgroundLabel())
      {
        color = this->LabelToColor(label, isMultiLink);
      }
      else
      {
        // convert 24 bit number to int64
        const int compositeId = label * 256 * 256 + instanceCount;
        color = this->LabelToColor(compositeId, isMultiLink);
      }

      customParameter = Ogre::Vector4(color.R(), color.G(), color.B(), 1.0);
    }
    else
    {
      // 256 => 8 bits .. 255 => color percentage
      float labelColor = label / 255.0f;
      float instanceColor1 = (instanceCount / 256) / 255.0f;
      float instanceColor2 = (instanceCount % 256) / 255.0f;

      customParameter =
        Ogre::Vector4(instanceColor2, instanceColor1, labelColor, 1.0);
    }
  }

  return customParameter;
}

/////////////////////////////////////////////////
math::Color Ogre2SegmentationMaterialSwitcher::LabelToColor(int64_t _label,
  bool _isMultiLink)
{
  if (_label == this->segmentationCamera->BackgroundLabel())
    return this->segmentationCamera->BackgroundColor();

  // use label as seed to generate the same color for the label
  this->generator.seed(_label);
  std::uniform_int_distribution<int> distribution(0, 255);

  // random color
  int r = distribution(this->generator);
  int g = distribution(this->generator);
  int b = distribution(this->generator);

  math::Color color(
    static_cast<float>(r / 255.0f),
    static_cast<float>(g / 255.0f),
    static_cast<float>(b / 255.0f));

  // if the label is colored before return the color
  // don't check for taken colors in that case, all items
  // with the same label will have the same color
  if (this->segmentationCamera->Type() == SegmentationType::ST_SEMANTIC &&
    this->coloredLabel.count(_label))
    return color;

  if (_isMultiLink)
    return color;

  // loop recursively till finding a unique color
  if (this->IsTakenColor(color))
      return this->LabelToColor(_label);

  this->coloredLabel.insert(_label);

  // We don't multiply by 255 here as (r,g,b) are in [0-255] range
  int64_t colorId = r * 256 * 256 + g * 256 + b;
  this->colorToLabel[colorId] = _label;

  return color;
}

////////////////////////////////////////////////
VisualPtr Ogre2SegmentationMaterialSwitcher::TopLevelModelVisual(
    VisualPtr _visual) const
{
  if (!_visual)
    return _visual;
  VisualPtr p = _visual;
  while (p->Parent() && p->Parent() != _visual->Scene()->RootVisual())
    p = std::dynamic_pointer_cast<Visual>(p->Parent());
  return p;
}

////////////////////////////////////////////////
void Ogre2SegmentationMaterialSwitcher::cameraPreRenderScene(
    Ogre::Camera * /*_cam*/)
{
  this->colorToLabel.clear();
  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);

  auto engine = Ogre2RenderEngine::Instance();
  engine->SetGzOgreRenderingMode(GORM_SOLID_COLOR);

  // Used for multi-link models, where each model has many ogre items but
  // belongs to the same object, and all of them has the same parent name
  std::string prevParentName = "";

  // Store the ogre objects in a vector
  std::vector<Ogre::MovableObject *> ogreObjects;
  while (itor.hasMoreElements())
  {
    Ogre::MovableObject *object = itor.peekNext();
    ogreObjects.push_back(object);
    itor.moveNext();
  }

  // Sort the ogre objects by name
  // The algorithm of handling multi-link models depends on a sorted objects
  // by name, so all links that belongs to the same object come in order
  std::sort(ogreObjects.begin(), ogreObjects.end(),
    [] (Ogre::MovableObject * object1, Ogre::MovableObject * object2) {
      return object1->getName() > object2->getName();
  });

  this->materialMap.clear();
  this->datablockMap.clear();
  Ogre::HlmsManager *hlmsManager = engine->OgreRoot()->getHlmsManager();

  Ogre::HlmsDatablock *defaultPbs =
    hlmsManager->getHlms(Ogre::HLMS_PBS)->getDefaultDatablock();

  // Construct one now so that datablock->setBlendblock
  // each is as fast as possible
  const Ogre::HlmsBlendblock *noBlend =
    hlmsManager->getBlendblock(Ogre::HlmsBlendblock());

  for (auto *object : ogreObjects)
  {
    Ogre::Item *item = static_cast<Ogre::Item *>(object);

    // get visual from ogre item
    Ogre::Any userAny = item->getUserObjectBindings().getUserAny();

    if (!userAny.isEmpty() && userAny.getType() == typeid(unsigned int))
    {
      // get visual id for the ogre item
      auto visualId = Ogre::any_cast<unsigned int>(userAny);

      VisualPtr visual;
      try
      {
        visual = this->scene->VisualById(visualId);
      }
      catch(Ogre::Exception &e)
      {
        gzerr << "Ogre Error:" << e.getFullDescription() << "\n";
      }

      const Ogre::Vector4 customParameter =
        ColorForVisual(visual, prevParentName);

      const size_t numSubItems = item->getNumSubItems();
      for (size_t i = 0; i < numSubItems; ++i)
      {
        // Set the custom value to the sub item to render
        Ogre::SubItem *subItem = item->getSubItem(i);
        subItem->setCustomParameter(1, customParameter);

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
            if (material->getLoadingState() ==
                Ogre::Resource::LOADSTATE_UNLOADED)
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
    }
  }

  // Do the same with heightmaps / terrain
  auto heightmaps = this->scene->Heightmaps();
  for (auto h : heightmaps)
  {
    auto heightmap = h.lock();
    if (heightmap)
    {
      // TODO(anyone): Retrieve datablock and make sure it's not blending
      // like we do with Items (it should be impossible?)
      VisualPtr visual = heightmap->Parent();
      const Ogre::Vector4 customParameter =
        ColorForVisual(visual, prevParentName);
      heightmap->Terra()->SetSolidColor(1u, customParameter);
    }
  }

  // Remove the reference count on noBlend we created
  hlmsManager->destroyBlendblock(noBlend);

  // reset the count & colors tracking
  this->instancesCount.clear();
  this->takenColors.clear();
  this->coloredLabel.clear();
}

////////////////////////////////////////////////
void Ogre2SegmentationMaterialSwitcher::cameraPostRenderScene(
    Ogre::Camera * /*_cam*/)
{
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
  for (auto &subItemMat : this->materialMap)
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

////////////////////////////////////////////////
const std::unordered_map<int64_t, int64_t> &
Ogre2SegmentationMaterialSwitcher::ColorToLabel() const
{
  return this->colorToLabel;
}
