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

#include "Ogre2GzHlmsTerraPrivate.hh"

#include <gz/common/Console.hh>
#include <gz/common/Filesystem.hh>
#include <gz/common/Util.hh>

#include "Terra/Terra.h"

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreRenderQueue.h>
#include <OgreRootLayout.h>
#include <Vao/OgreConstBufferPacked.h>
#include <Vao/OgreVaoManager.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

using namespace gz;
using namespace rendering;

namespace Ogre
{
  /// \brief The slot where to bind currPerObjectDataBuffer
  /// HlmsPbs might consume slot 3, so we always use slot 4 for simplicity
  /// \internal
  static const uint16 kPerObjectDataBufferSlot = 4u;

  Ogre2GzHlmsTerra::Ogre2GzHlmsTerra(
    Archive *dataFolder, ArchiveVec *libraryFolders,
    Ogre2GzHlmsSphericalClipMinDistance *_sphericalClipMinDistance) :
    HlmsTerra(dataFolder, libraryFolders)
  {
    this->customizations.push_back(_sphericalClipMinDistance);
  }

  /////////////////////////////////////////////////
  uint16 Ogre2GzHlmsTerra::getNumExtraPassTextures(
    const HlmsPropertyVec &_properties, bool _casterPass) const
  {
    uint16 numExtraTextures = 0u;

    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      numExtraTextures +=
        listener->getNumExtraPassTextures(_properties, _casterPass);
    }

    return numExtraTextures;
  }

  /////////////////////////////////////////////////
  void Ogre2GzHlmsTerra::propertiesMergedPreGenerationStep(
    Hlms *_hlms,  //
    const HlmsCache &_passCache,
    const HlmsPropertyVec &_renderableCacheProperties,
    const PiecesMap _renderableCachePieces[NumShaderTypes],
    const HlmsPropertyVec &_properties,
    const QueuedRenderable &_queuedRenderable)
  {
    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      listener->propertiesMergedPreGenerationStep(
        _hlms, _passCache, _renderableCacheProperties, _renderableCachePieces,
        _properties, _queuedRenderable);
    }
  }

  /////////////////////////////////////////////////
  void Ogre2GzHlmsTerra::preparePassHash(
    const CompositorShadowNode *_shadowNode, bool _casterPass,
    bool _dualParaboloid, SceneManager *_sceneManager, Hlms *_hlms)
  {
    if (!_casterPass &&
        (this->gzOgreRenderingMode == GORM_SOLID_COLOR ||
         this->gzOgreRenderingMode == GORM_SOLID_THERMAL_COLOR_TEXTURED))
    {
      _hlms->_setProperty("gz_render_solid_color", 1);

      if (this->gzOgreRenderingMode == GORM_SOLID_THERMAL_COLOR_TEXTURED)
        _hlms->_setProperty("gz_render_solid_color_textured", 1);
    }

    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      listener->preparePassHash(_shadowNode, _casterPass, _dualParaboloid,
                                _sceneManager, _hlms);
    }
  }

  /////////////////////////////////////////////////
  uint32 Ogre2GzHlmsTerra::getPassBufferSize(
    const Ogre::CompositorShadowNode *_shadowNode, bool _casterPass,
    bool _dualParaboloid, Ogre::SceneManager *_sceneManager) const
  {
    uint32 bufferSize = 0u;

    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      bufferSize += listener->getPassBufferSize(_shadowNode, _casterPass,
                                                _dualParaboloid, _sceneManager);
    }
    return bufferSize;
  }

  /////////////////////////////////////////////////
  float *Ogre2GzHlmsTerra::preparePassBuffer(
    const Ogre::CompositorShadowNode *_shadowNode, bool _casterPass,
    bool _dualParaboloid, Ogre::SceneManager *_sceneManager,
    float *_passBufferPtr)
  {
    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      _passBufferPtr =
        listener->preparePassBuffer(_shadowNode, _casterPass, _dualParaboloid,
                                    _sceneManager, _passBufferPtr);
    }
    return _passBufferPtr;
  }

  /////////////////////////////////////////////////
  void Ogre2GzHlmsTerra::setupRootLayout(
    RootLayout &_rootLayout, const HlmsPropertyVec &_properties) const
  {
    if (this->getProperty(_properties, "gz_render_solid_color") != 0)
    {
      // Account for the extra buffer bound at kPerObjectDataBufferSlot
      // It should be the last buffer to be set, so kPerObjectDataBufferSlot + 1
      _rootLayout.mDescBindingRanges[0][DescBindingTypes::ConstBuffer].end =
        kPerObjectDataBufferSlot + 1u;
    }

    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      listener->setupRootLayout(_rootLayout, _properties);
    }
  }

  /////////////////////////////////////////////////
  void Ogre2GzHlmsTerra::shaderCacheEntryCreated(
    const String &_shaderProfile, const HlmsCache *_hlmsCacheEntry,
    const HlmsCache &_passCache, const HlmsPropertyVec &_properties,
    const QueuedRenderable &_queuedRenderable)
  {
    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      listener->shaderCacheEntryCreated(_shaderProfile, _hlmsCacheEntry,
                                        _passCache, _properties,
                                        _queuedRenderable);
    }
  }

  /////////////////////////////////////////////////
  void Ogre2GzHlmsTerra::notifyPropertiesMergedPreGenerationStep()
  {
    HlmsTerra::notifyPropertiesMergedPreGenerationStep();

    setProperty("GzPerObjectDataSlot", kPerObjectDataBufferSlot);
  }

  /////////////////////////////////////////////////
  void Ogre2GzHlmsTerra::hlmsTypeChanged(bool _casterPass,
                                         CommandBuffer *_commandBuffer,
                                         const HlmsDatablock *_datablock,
                                         size_t _texUnit)
  {
    // Allow additional listener-only customizations to inject their stuff
    for (Ogre::HlmsListener *listener : this->customizations)
    {
      listener->hlmsTypeChanged(_casterPass, _commandBuffer, _datablock,
                                _texUnit);
    }

    if (_casterPass ||
        (this->gzOgreRenderingMode != GORM_SOLID_COLOR &&
         this->gzOgreRenderingMode != GORM_SOLID_THERMAL_COLOR_TEXTURED))
    {
      return;
    }

    this->BindObjectDataBuffer(_commandBuffer, kPerObjectDataBufferSlot);
  }

  /////////////////////////////////////////////////
  uint32 Ogre2GzHlmsTerra::fillBuffersForV1(
    const HlmsCache *_cache, const QueuedRenderable &_queuedRenderable,
    bool _casterPass, uint32 _lastCacheHash, CommandBuffer *_commandBuffer)
  {
    const uint32 instanceIdx = HlmsTerra::fillBuffersForV1(
      _cache, _queuedRenderable, _casterPass, _lastCacheHash, _commandBuffer);

    if ((this->gzOgreRenderingMode == GORM_SOLID_COLOR ||
         this->gzOgreRenderingMode == GORM_SOLID_THERMAL_COLOR_TEXTURED) &&
        !_casterPass)
    {
      const Ogre::Terra *terra =
        static_cast<const Ogre::Terra *>(_queuedRenderable.movableObject);

      Vector4 customParam;
      try
      {
        customParam = terra->SolidColor(1u);
      }
      catch (ItemIdentityException &)
      {
        // This error can trigger for two reasons:
        //
        //  1. We forgot to call setSolidColor(1u, ...)
        //  2. This object should not be rendered and we should've called
        //     movableObject->setVisible(false) or use RenderQueue IDs
        //     or visibility flags to prevent rendering it
        gzerr << "A module is trying to render an object without "
                 "specifying a parameter. Please report this bug at "
                 "https://github.com/gazebosim/gz-rendering/issues\n";
        throw;
      }
      float *dataPtr = this->MapObjectDataBufferFor(
        instanceIdx, _commandBuffer, this->mVaoManager, this->mConstBuffers,
        this->mCurrentConstBuffer, this->mStartMappedConstBuffer,
        kPerObjectDataBufferSlot);
      dataPtr[0] = customParam.x;
      dataPtr[1] = customParam.y;
      dataPtr[2] = customParam.z;

      if (this->gzOgreRenderingMode == GORM_SOLID_THERMAL_COLOR_TEXTURED &&
          terra->HasSolidColor(2u))
      {
        GZ_ASSERT(customParam.w >= 0.0f,
                  "customParam.w can't be negative for "
                  "GORM_SOLID_THERMAL_COLOR_TEXTURED");

        // Negate customParam.w to tell the shader we wish to multiply
        // against the diffuse texture. We substract 0.5f to avoid -0.0 = 0.0
        dataPtr[3] = -customParam.w - 0.5f;
      }
      else
      {
        dataPtr[3] = customParam.w;
      }
    }

    return instanceIdx;
  }

  /////////////////////////////////////////////////
  uint32 Ogre2GzHlmsTerra::fillBuffersForV2(
    const HlmsCache *_cache, const QueuedRenderable &_queuedRenderable,
    bool _casterPass, uint32 _lastCacheHash, CommandBuffer *_commandBuffer)
  {
    const uint32 instanceIdx = HlmsTerra::fillBuffersForV2(
      _cache, _queuedRenderable, _casterPass, _lastCacheHash, _commandBuffer);

    if ((this->gzOgreRenderingMode == GORM_SOLID_COLOR ||
         this->gzOgreRenderingMode == GORM_SOLID_THERMAL_COLOR_TEXTURED) &&
        !_casterPass)
    {
      const Ogre::Terra *terra =
        static_cast<const Ogre::Terra *>(_queuedRenderable.movableObject);

      Vector4 customParam;
      try
      {
        customParam = terra->SolidColor(1u);
      }
      catch (ItemIdentityException &)
      {
        // This error can trigger for two reasons:
        //
        //  1. We forgot to call setSolidColor(1u, ...)
        //  2. This object should not be rendered and we should've called
        //     movableObject->setVisible(false) or use RenderQueue IDs
        //     or visibility flags to prevent rendering it
        gzerr << "A module is trying to render an object without "
                 "specifying a parameter. Please report this bug at "
                 "https://github.com/gazebosim/gz-rendering/issues\n";
        throw;
      }
      float *dataPtr = this->MapObjectDataBufferFor(
        instanceIdx, _commandBuffer, this->mVaoManager, this->mConstBuffers,
        this->mCurrentConstBuffer, this->mStartMappedConstBuffer,
        kPerObjectDataBufferSlot);
      dataPtr[0] = customParam.x;
      dataPtr[1] = customParam.y;
      dataPtr[2] = customParam.z;
      dataPtr[3] = customParam.w;

      if (this->gzOgreRenderingMode == GORM_SOLID_THERMAL_COLOR_TEXTURED &&
          terra->HasSolidColor(2u))
      {
        GZ_ASSERT(customParam.w >= 0.0f,
                  "customParam.w can't be negative for "
                  "GORM_SOLID_THERMAL_COLOR_TEXTURED");

        // Negate customParam.w to tell the shader we wish to multiply
        // against the diffuse texture. We substract 0.5f to avoid -0.0 = 0.0
        dataPtr[3] = -customParam.w - 0.5f;
      }
      else
      {
        dataPtr[3] = customParam.w;
      }
    }

    return instanceIdx;
  }

  /////////////////////////////////////////////////
  void Ogre2GzHlmsTerra::preCommandBufferExecution(
    CommandBuffer *_commandBuffer)
  {
    this->UnmapObjectDataBuffer();
    HlmsTerra::preCommandBufferExecution(_commandBuffer);
  }

  /////////////////////////////////////////////////
  void Ogre2GzHlmsTerra::frameEnded()
  {
    HlmsTerra::frameEnded();

    this->currPerObjectDataBuffer = nullptr;
    this->lastMainConstBuffer = nullptr;
    this->currPerObjectDataPtr = nullptr;
  }

  /////////////////////////////////////////////////
  void Ogre2GzHlmsTerra::GetDefaultPaths(String &_outDataFolderPath,
                                         StringVector &_outLibraryFoldersPaths)
  {
    HlmsTerra::getDefaultPaths(_outDataFolderPath, _outLibraryFoldersPaths);

    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Gz", "SolidColor"));
    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Gz", "SphericalClipMinDistance"));
    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Terra", "gz"));
    _outLibraryFoldersPaths.push_back(
      common::joinPaths("Hlms", "Gz", "Pbs"));
  }
}  // namespace Ogre
