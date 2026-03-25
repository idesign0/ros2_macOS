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
#ifndef GZ_RENDERING_OGRE2_OGRE2GZHLMSPBSPRIVATE_HH_
#define GZ_RENDERING_OGRE2_OGRE2GZHLMSPBSPRIVATE_HH_

#include "gz/rendering/config.hh"
#include "gz/rendering/ogre2/Export.hh"

#include "Ogre2GzHlmsSphericalClipMinDistance.hh"
#include "Ogre2GzHlmsSharedPrivate.hh"

#ifdef _MSC_VER
  #pragma warning(push, 0)
#endif
#include <Hlms/Pbs/OgreHlmsPbs.h>
#include <OgreHlmsListener.h>
#ifdef _MSC_VER
  #pragma warning(pop)
#endif

#include <vector>

namespace Ogre
{
  class HlmsPbsTerraShadows;

  /// \brief Controls custom shader snippets of Hlms:
  ///
  ///   - Toggles them on/off
  ///   - Sends relevant data to the GPU buffers for shaders to use
  ///
  /// This listener requires Hlms to have been created with the piece data
  /// files in ogre2/src/media/Hlms/Gz registered
  ///
  /// We need to derive from HlmsPbs/HlmsUnlit (rather than just using
  /// HlmsListener) when we need to use code that sends data
  /// *per object*
  ///
  /// For performance reasons Ogre does not allow passing per-object
  /// data via listeners; so we override Hlms implementations.
  ///
  /// Use GetDefaultPaths to get them
  ///
  /// \internal
  /// \remark Public variables take effect immediately (i.e. for the
  /// next render)
  class GZ_RENDERING_OGRE2_HIDDEN Ogre2GzHlmsPbs final
    : public HlmsPbs,
      public HlmsListener,
      public gz::rendering::Ogre2GzHlmsShared
  {
    /// \brief Constructor. Asks for modular listeners so we can add
    /// them in the proper order
    public: Ogre2GzHlmsPbs(Archive *dataFolder, ArchiveVec *libraryFolders,
                            gz::rendering::
                            Ogre2GzHlmsSphericalClipMinDistance
                            *_sphericalClipMinDistance,
                            Ogre::HlmsPbsTerraShadows *terraShadows);

    /// \brief Destructor. Virtual to silence warnings
    public: virtual ~Ogre2GzHlmsPbs() override = default;

    /// \brief Override so listeners can inform Hlms the number of
    /// extra textures they need to make room for.
    ///
    /// \remarks We cannot read 'this' state. All state data
    /// must come from _properties. Otherwise we can't use
    /// HlmsDiskCache
    ///
    /// \param[in] _properties Properties to read state from
    /// \param[in] _casterPass Whether this is a caster pass
    /// \return Extra number of per-pass textures Hlms needs to account for
    public: virtual uint16 getNumExtraPassTextures(
        const HlmsPropertyVec &_properties, bool _casterPass) const override;

    /// \brief Override so listeners can set extra properties after
    /// a renderable is assigned an HlmsDatablock
    ///
    /// \param[in, out] _hlms Hlms to modify
    /// \param[in] _passCache Current properties at pass level
    /// \param[in] _renderableCacheProperties properties at renderable level
    /// \param[in] _renderableCachePieces Custom pieces for this renderable
    /// \param[in] _properties Properties defined so far (amalgamated)
    /// \param[in] _queuedRenderable Renderable being affected
    public: virtual void propertiesMergedPreGenerationStep(
        Hlms *_hlms,
        const HlmsCache &_passCache,
        const HlmsPropertyVec &_renderableCacheProperties,
        const PiecesMap _renderableCachePieces[NumShaderTypes],
        const HlmsPropertyVec &_properties,
        const QueuedRenderable &_queuedRenderable) override;

    // Documentation inherited
    public: using HlmsPbs::preparePassHash;

    /// \brief Override HlmsListener to add customizations.
    /// We can't override HlmsPbs because adding properties before
    /// calling it will be cleared. And adding it afterwards is too late.
    /// The listener gets called right in the middle
    ///
    /// \param[in] _shadowNode see base class
    /// \param[in] _casterPass see base class
    /// \param[in] _dualParaboloid see base class
    /// \param[in] _sceneManager see base class
    /// \param[in] _hlms see base class
    public: virtual void preparePassHash(
        const CompositorShadowNode *_shadowNode,
        bool _casterPass,
        bool _dualParaboloid,
        SceneManager *_sceneManager,
        Hlms *_hlms) override;

    /// \brief Tells Ogre the buffer data sent to GPU should be a little
    /// bigger to fit our data we need to send
    /// \param[in] _shadowNode see base class
    /// \param[in] _casterPass see base class
    /// \param[in] _dualParaboloid see base class
    /// \param[in] _sceneManager see base class
    /// \return Size in bytes of how bigger the buffer should be
    public: virtual uint32 getPassBufferSize(
        const Ogre::CompositorShadowNode *_shadowNode,
        bool _casterPass, bool _dualParaboloid,
        Ogre::SceneManager *_sceneManager) const override;

    /// \brief Sends our custom data to GPU buffers that our
    /// pieces activated in preparePassHash will need.
    ///
    /// Bytes written must not exceed what we informed in getPassBufferSize
    /// \param[in] _shadowNode see base class
    /// \param[in] _casterPass see base class
    /// \param[in] _dualParaboloid see base class
    /// \param[in] _sceneManager see base class
    /// \param[in] _passBufferPtr see base class
    /// \return The pointer where Ogre should continue appending more data
    private: virtual float* preparePassBuffer(
          const Ogre::CompositorShadowNode *_shadowNode,
          bool _casterPass, bool _dualParaboloid,
          Ogre::SceneManager *_sceneManager,
          float *_passBufferPtr) override;

    /// \brief Fix warning about setupRootLayout overload hiding each other
    protected: using HlmsPbs::setupRootLayout;

    /// \brief Override so listeners can alter the root layout
    /// if they need to add buffers to shaders. Applies to
    /// a few APIs like Vulkan
    ///
    /// \remarks We cannot read 'this' state. All state data
    /// must come from _properties. Otherwise we can't use
    /// HlmsDiskCache
    ///
    /// \param[in,out] _rootLayout Root Layout to modify
    /// \param[in] _properties Properties the PSO is being compiled with
    public: virtual void setupRootLayout(
        RootLayout &_rootLayout,
        const HlmsPropertyVec &_properties) const override;

    /// \brief See HlmsListener::shaderCacheEntryCreated
    public: virtual void shaderCacheEntryCreated(
        const String &_shaderProfile, const HlmsCache *_hlmsCacheEntry,
        const HlmsCache &_passCache, const HlmsPropertyVec &_properties,
        const QueuedRenderable &_queuedRenderable) override;

      /// \brief Override to calculate which slots are used
    public: virtual void notifyPropertiesMergedPreGenerationStep() override;

    /// \brief Override HlmsListener::hlmsTypeChanged so we can
    /// bind buffers which carry per-object data when in GORM_SOLID_COLOR
    /// \param[in] _casterPass true if this is a caster pass
    /// \param[in] _commandBuffer command buffer so we can add commands
    /// \param[in] _datablock material of the object that caused
    /// Ogre2GzHlmsPbs to be bound again
    public: virtual void hlmsTypeChanged(
        bool _casterPass,
        CommandBuffer *_commandBuffer,
        const HlmsDatablock *_datablock,
        size_t _texUnit) override;

    // Documentation inherited
    public: virtual uint32 fillBuffersForV1(
      const HlmsCache *_cache,
      const QueuedRenderable &_queuedRenderable,
      bool _casterPass, uint32 _lastCacheHash,
      CommandBuffer *_commandBuffer ) override;

    // Documentation inherited
    public: virtual uint32 fillBuffersForV2(
      const HlmsCache *_cache,
      const QueuedRenderable &_queuedRenderable,
      bool _casterPass, uint32 _lastCacheHash,
      CommandBuffer *_commandBuffer ) override;

    // Documentation inherited
    public: virtual void preCommandBufferExecution(
        CommandBuffer *_commandBuffer) override;

    // Documentation inherited
    public: virtual void frameEnded() override;

    //// \brief Same as HlmsPbs::getDefaultPaths, but we also append
    /// our own paths with customizations
    /// \param[out] _outDataFolderPath Path (as a String) used for
    /// creating the "dataFolder" Archive the constructor will need
    /// \param[out] _outLibraryFoldersPaths
    /// Vector of String used for creating the ArchiveVector "libraryFolders"
    /// the constructor will need. Our own stuff is appended here
    public: static void GetDefaultPaths(String &_outDataFolderPath,
                                        StringVector &_outLibraryFoldersPaths);

    /// \brief Contains additional customizations that are modular and
    /// implemented as listener-only
    private: std::vector<Ogre::HlmsListener*> customizations;
  };
}  // namespace Ogre

#endif
