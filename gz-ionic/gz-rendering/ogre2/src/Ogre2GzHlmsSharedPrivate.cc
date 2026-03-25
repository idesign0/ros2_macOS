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

#include "Ogre2GzHlmsSharedPrivate.hh"

#include <gz/common/Util.hh>

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif
#include <CommandBuffer/OgreCbShaderBuffer.h>
#include <CommandBuffer/OgreCommandBuffer.h>
#include <OgreRenderQueue.h>
#include <Vao/OgreConstBufferPacked.h>
#include <Vao/OgreVaoManager.h>
#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#include <algorithm>

namespace gz
{
  namespace rendering
  {
    /////////////////////////////////////////////////
    Ogre2GzHlmsShared::~Ogre2GzHlmsShared()
    {
      if (!this->vaoManager)
        return;

      for (auto & buffer : this->perObjectDataBuffers)
        this->vaoManager->destroyConstBuffer(buffer);

      this->perObjectDataBuffers.clear();
    }

    /////////////////////////////////////////////////
    void Ogre2GzHlmsShared::BindObjectDataBuffer(
      Ogre::CommandBuffer *_commandBuffer, uint16_t _perObjectDataBufferSlot)
    {
      if (this->currPerObjectDataBuffer)
      {
        *_commandBuffer->addCommand<Ogre::CbShaderBuffer>() =
          Ogre::CbShaderBuffer(
            Ogre::VertexShader, _perObjectDataBufferSlot,
            this->currPerObjectDataBuffer, 0u,
            static_cast<uint32_t>(
              this->currPerObjectDataBuffer->getTotalSizeBytes()));
      }
    }

    /////////////////////////////////////////////////
    float *Ogre2GzHlmsShared::MapObjectDataBufferFor(
      uint32_t _instanceIdx, Ogre::CommandBuffer *_commandBuffer,
      Ogre::VaoManager *_vaoManager, const ConstBufferPackedVec &_constBuffers,
      uint32_t _currConstBufferIdx, uint32_t *_startMappedConstBuffer,
      uint16_t _perObjectDataBufferSlot)
    {
      // This argument is used only in the MACRO, and it generating a warning.
      // This should void the warning
      (void)_startMappedConstBuffer;
      const uint32_t numFloatsPerObject = 4u;

      if (!this->currPerObjectDataBuffer ||
          this->lastMainConstBuffer != _constBuffers[_currConstBufferIdx])
      {
        // mConstBuffers[this->mCurrentConstBuffer] changed, which means
        // gl_InstanceId / drawId will be reset to 0. We must create a new
        // buffer and bind that one

        this->UnmapObjectDataBuffer();

        Ogre::ConstBufferPacked *constBuffer = nullptr;
        if (_currConstBufferIdx >= this->perObjectDataBuffers.size())
        {
          this->vaoManager = _vaoManager;
          const size_t bufferSize =
            std::min<size_t>(65536, _vaoManager->getConstBufferMaxSize());
          constBuffer = _vaoManager->createConstBuffer(
            bufferSize, Ogre::BT_DYNAMIC_PERSISTENT, nullptr, false);
          this->perObjectDataBuffers.push_back(constBuffer);
        }
        else
        {
          constBuffer = this->perObjectDataBuffers[_currConstBufferIdx];
        }

        this->currPerObjectDataBuffer = constBuffer;
        this->currPerObjectDataPtr = reinterpret_cast<float *>(
          constBuffer->map(0u, constBuffer->getNumElements()));

        GZ_ASSERT(
          _currConstBufferIdx <= _constBuffers.size() &&
            _startMappedConstBuffer != nullptr,
          "This should not happen. Base class must've bound something");

        this->lastMainConstBuffer = _constBuffers[_currConstBufferIdx];

        this->BindObjectDataBuffer(_commandBuffer, _perObjectDataBufferSlot);
      }

      const size_t offset = _instanceIdx * numFloatsPerObject;

      // This assert triggering either means:
      //  - This class got modified and we're packing more data into
      //    currPerObjectDataBuffer, so it must be bigger
      //    (use a TexBufferPacked if we're past limits)
      //  - There is a bug and currPerObjectDataBuffer got out of sync
      //    with mCurrentConstBuffer
      GZ_ASSERT((offset + numFloatsPerObject) * sizeof(float) <=
                   this->currPerObjectDataBuffer->getTotalSizeBytes(),
                 "Out of bounds!");

      return this->currPerObjectDataPtr + offset;
    }

    /////////////////////////////////////////////////
    void Ogre2GzHlmsShared::UnmapObjectDataBuffer()
    {
      if (this->currPerObjectDataBuffer)
      {
        this->currPerObjectDataBuffer->unmap(
          Ogre::UO_KEEP_PERSISTENT, 0u,
          this->currPerObjectDataBuffer->getNumElements());
        this->currPerObjectDataPtr = 0;
        this->currPerObjectDataBuffer = 0;
        this->lastMainConstBuffer = 0;
      }
    }
  }  // namespace rendering
}  // namespace gz
