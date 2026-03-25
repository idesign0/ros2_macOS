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
#ifndef GZ_RENDERING_OGRE2_OGRE2GZHLMSSHAREDPRIVATE_HH_
#define GZ_RENDERING_OGRE2_OGRE2GZHLMSSHAREDPRIVATE_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"
#include "gz/rendering/ogre2/Ogre2GzOgreRenderingMode.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <OgrePrerequisites.h>
#include <ogrestd/vector.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#include <vector>

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE
    {
    typedef gz::rendering::GzOgreRenderingMode GzOgreRenderingMode;
    typedef Ogre::vector<Ogre::ConstBufferPacked*>::type ConstBufferPackedVec;

    /// \brief Implements code shared across all or most of our Hlms
    /// customizations
    /// \internal
    class GZ_RENDERING_OGRE2_HIDDEN Ogre2GzHlmsShared
    {
      /// \brief Destructor
      protected: ~Ogre2GzHlmsShared();

      /// \brief Binds currPerObjectDataBuffer to the right slot.
      /// Does nothing if it's nullptr
      /// \param[in] _commandBuffer Cmd buffer to bind to
      /// \param[in] _perObjectDataBufferSlot slot to bind the buffer to
      /// \internal
      protected: void BindObjectDataBuffer(Ogre::CommandBuffer *_commandBuffer,
                                           uint16_t _perObjectDataBufferSlot);

      /// \brief Vector of buffers holding per-object data.
      /// When one runs out, we push a new one. On the next frame
      /// we reuse them all from 0
      ///
      /// It also calls BindObjectDataBuffer if a new one must be created
      /// \param[in] _instanceIdx The index of the instance to write to
      /// \param[in] _commandBuffer Command buffer to bind our new buffer
      /// if we create one
      /// \param[in] _vaoManager VaoManager to create new ConstBufferPacked
      /// \param[in] _constBuffers Reference to mConstBuffers so we can
      /// tell if we need to bind a new const buffer
      /// \param[in] _currConstBufferIdx value of mCurrentConstBuffer so we can
      /// tell if we need to bind a new const buffer
      /// \param[in] _startMappedConstBuffer value of mStartMappedConstBuffer
      /// for validation (to ensure our implementation isn't out of sync
      /// with Ogre's)
      /// \param[in] _perObjectDataBufferSlot See
      /// Ogre2GzHlmsShared::BindObjectDataBuffer
      /// \return Pointer to write data for that instance
      /// \internal
      protected: float *MapObjectDataBufferFor(uint32_t _instanceIdx,
            Ogre::CommandBuffer *_commandBuffer,
            Ogre::VaoManager *_vaoManager,
            const ConstBufferPackedVec &_constBuffers,
            uint32_t _currConstBufferIdx,
            uint32_t *_startMappedConstBuffer,
            uint16_t _perObjectDataBufferSlot);

      /// \brief Unmaps the current buffer holding per-object data from memory
      protected: void UnmapObjectDataBuffer();

      /// \brief Vector of buffers holding per-object data.
      /// When one runs out, we push a new one. On the next frame
      /// we reuse them all from 0
      /// \internal
      protected: std::vector<Ogre::ConstBufferPacked *> perObjectDataBuffers;

      /// \brief The buffer currently use. Can be nullptr
      /// It is contained in perObjectDataBuffers
      /// \internal
      protected: Ogre::ConstBufferPacked *currPerObjectDataBuffer = nullptr;

      /// \brief The last content of mCurrentConstBuffer. If it changes
      /// we need a new currPerObjectDataBuffer too (because drawId will
      /// be reset from 0)
      /// \internal
      protected: Ogre::ConstBufferPacked *lastMainConstBuffer = nullptr;

      /// \brief The mapped contents of currPerObjectDataBuffer
      /// \internal
      protected: float *currPerObjectDataPtr = nullptr;

      /// \brief Pointer to Ogre's VAO manager. Used here for destroying const
      /// buffers.
      private: Ogre::VaoManager *vaoManager = nullptr;

      /// \brief See GzOgreRenderingMode. Public variable.
      /// Modifying it takes change on the next render
      public: GzOgreRenderingMode gzOgreRenderingMode = GORM_NORMAL;
    };
    }
  }
}

#endif
