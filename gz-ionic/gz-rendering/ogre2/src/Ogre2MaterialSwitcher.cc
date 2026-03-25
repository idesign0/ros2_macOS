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

#include "gz/common/Console.hh"

#include "gz/rendering/ogre2/Ogre2Heightmap.hh"
#include "gz/rendering/ogre2/Ogre2MaterialSwitcher.hh"
#include "gz/rendering/ogre2/Ogre2RenderEngine.hh"
#include "gz/rendering/ogre2/Ogre2Scene.hh"
#include "gz/rendering/RenderTypes.hh"

#include "Terra/Terra.h"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgreHlms.h>
#include <OgreItem.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

/////////////////////////////////////////////////
Ogre2MaterialSwitcher::Ogre2MaterialSwitcher(Ogre2ScenePtr _scene)
{
  this->currentColor = math::Color(0.0f, 0.0f, 0.1f);
  this->scene = _scene;
}

/////////////////////////////////////////////////
Ogre2MaterialSwitcher::~Ogre2MaterialSwitcher()
{
}

////////////////////////////////////////////////
void Ogre2MaterialSwitcher::cameraPreRenderScene(
    Ogre::Camera * /*_evt*/)
{
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

  auto itor = this->scene->OgreSceneManager()->getMovableObjectIterator(
      Ogre::ItemFactory::FACTORY_TYPE_NAME);
  while (itor.hasMoreElements())
  {
    this->NextColor();

    Ogre::MovableObject *object = itor.peekNext();
    Ogre::Item *item = static_cast<Ogre::Item *>(object);

    this->colorDict[this->currentColor.AsRGBA()] = item->getName();

    const Ogre::Vector4 ogreCurrentColor(this->currentColor.R(),
                                         this->currentColor.G(),
                                         this->currentColor.B(), 1.0);

    const size_t numSubItems = item->getNumSubItems();
    for (size_t i = 0; i < numSubItems; ++i)
    {
      Ogre::SubItem *subItem = item->getSubItem(i);

      subItem->setCustomParameter(1, ogreCurrentColor);

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
      this->NextColor();
      this->colorDict[this->currentColor.AsRGBA()] = heightmap->Name();

      // TODO(anyone): Retrieve datablock and make sure it's not blending
      // like we do with Items (it should be impossible?)
      heightmap->Terra()->SetSolidColor(
        1u, Ogre::Vector4(this->currentColor.R(), this->currentColor.G(),
                          this->currentColor.B(), 1.0));
    }
  }

  // Remove the reference count on noBlend we created
  hlmsManager->destroyBlendblock(noBlend);
}

/////////////////////////////////////////////////
void Ogre2MaterialSwitcher::cameraPostRenderScene(
    Ogre::Camera * /*_evt*/)
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

/////////////////////////////////////////////////
std::string Ogre2MaterialSwitcher::EntityName(
    const math::Color &_color) const
{
  auto iter = this->colorDict.find(_color.AsRGBA());

  if (iter != this->colorDict.end())
    return (*iter).second;
  else
    return std::string();
}

/////////////////////////////////////////////////
void Ogre2MaterialSwitcher::NextColor()
{
  auto color = this->currentColor.AsARGB();
  color++;
  this->currentColor.SetFromARGB(color);
}

/////////////////////////////////////////////////
void Ogre2MaterialSwitcher::Reset()
{
  this->currentColor = math::Color(
      0.0, 0.0, 0.0);
  this->colorDict.clear();
}
