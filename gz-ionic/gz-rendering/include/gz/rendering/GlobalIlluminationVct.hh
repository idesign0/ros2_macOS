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
#ifndef GZ_RENDERING_GLOBALILLUMINATIONVCT_HH_
#define GZ_RENDERING_GLOBALILLUMINATIONVCT_HH_

#include "gz/rendering/GlobalIlluminationBase.hh"

namespace gz
{
  namespace rendering
  {
    inline namespace GZ_RENDERING_VERSION_NAMESPACE {

    /// \class GlobalIlluminationVct GlobalIlluminationVct.hh
    /// gz/rendering/GlobalIlluminationVct.hh
    /// \brief Implements Global Illumination using VCT (Voxel Cone Tracing)
    class GZ_RENDERING_VISIBLE GlobalIlluminationVct :
        public GlobalIlluminationBase
    {
      /// \enum DebugVisualizationMode
      /// \brief Debug visualization modes for
      /// GlobalIlluminationVct::SetDebugVisualization
      public: enum DebugVisualizationMode
              {
                /// \brief Debug voxelized Albedo
                DVM_Albedo,
                /// \brief Debug voxelized Normals
                DVM_Normal,
                /// \brief Debug voxelized Emissive materials
                DVM_Emissive,
                /// \brief Debug VCT result (i.e. what's used by GI)
                DVM_Lighting,
                /// \brief Disable debugging
                DVM_None
              };

      /// \brief Constructor
      public: GlobalIlluminationVct();

      /// \brief Destructor
      public: virtual ~GlobalIlluminationVct();

      /// \brief Resolution of the 3D Voxel. Must be multiple of 2
      /// \remarks To avoid wasting RAM, make this function your first call
      /// if you intend to change the defaults.
      /// \param[in] _resolution Resolution
      public: virtual void SetResolution(const uint32_t _resolution[3]) = 0;

      /// \brief Returns 3D Voxel resolution
      /// \return Resolution in each axis. Length of array is 3
      public: virtual const uint32_t* Resolution() const = 0;

      /// \brief This will try to release resources that aren't needed
      /// when lights and static objects don't change.
      ///
      /// When true, every time light changes we will recreate those resources,
      /// update the GI, and release them; causing VRAM spikes.
      /// This can be bad for memory fragmentation (and performance) if
      /// VCT is updated too often.
      ///
      /// When false, those resources will stay loaded at all times
      /// \param[in] _conserveMemory True to release memory as fast as possible
      public: virtual void SetConserveMemory(bool _conserveMemory) = 0;

      /// \brief Whether we're releasing memory as fast as possible
      /// \return Conserve Memory setting
      public: virtual bool ConserveMemory() const = 0;

      /// \brief True for high quality (slower). VRAM consumption
      /// stays the same. Ogre2 will use 6 cones instead of 4
      /// \param[in] _highQuality Quality setting
      public: virtual void SetHighQuality(bool _highQuality) = 0;

      /// \brief Whether we're using 6 (true) or 4 (false) cones
      /// \return High Quality setting
      public: virtual bool HighQuality() const = 0;

      /// \brief Anisotropic VCT provides much higher quality because
      /// it better captures incoming indirect illumination from all
      /// directions.
      ///
      /// However it consumes more VRAM and is slower. Recommended: true
      /// \remark This setting has nothing to do with
      /// anistropic texture filtering in GPUs
      /// \param[in] _anisotropic Anisotropy setting
      public: virtual void SetAnisotropic(bool _anisotropic) = 0;

      /// \brief Whether anisotropic setting is on
      /// \return Anisotropy setting
      public: virtual bool Anisotropic() const = 0;

      /// \brief Shadows are calculated by raymarching towards the light
      /// source. However sometimes the ray 'may go through' a wall due to how
      /// bilinear interpolation works.
      ///
      /// Bilinear interpolation can produce nicer soft shadows, but it can
      /// also cause this light leaking from behind a wall.
      ///
      /// Increase this value (e.g. to 2.0f) to fight light leaking.
      /// This should generally (over-)darken the scene
      ///
      /// Lower values will lighten the scene and allow more light leaking
      ///
      /// Note that thinWallCounter can *not* fight all sources of light
      /// leaking, thus increasing it to ridiculous high values may not yield
      /// any benefit.
      ///
      /// \param[in] _thinWallCounter Value in range (0; inf)
      public: virtual void SetThinWallCounter(float _thinWallCounter) = 0;

      /// \brief Value of SetThinWallCounter
      /// \return ThingWallCounter value
      public: virtual float ThinWallCounter() const = 0;

      /// \brief Number of divisions per axis to divide the scene
      /// into multiple octants. Setting this value > 1 *may* improve
      /// voxelization performance when there is a lot of static Meshes;
      /// but it may also decrease it.
      /// Profiling is required for each scene
      /// \param[in] _octants Number of octant subdivisions per axis
      public: virtual void SetOctantCount(const uint32_t _octants[3]) = 0;

      /// \brief Returns number of octant subdivisions per axis
      /// \return Octant subdivisions. Length of array is 3
      public: virtual const uint32_t* OctantCount() const = 0;

      /// \brief Draws the voxels on screen for inspection and understand what
      /// is going on with GI. You should be looking at a minecraft-like world
      /// \param[in] _dvm What component to visualize
      public: virtual void SetDebugVisualization(
            DebugVisualizationMode _dvm) = 0;

      /// \brief Returns current visualization mode
      /// \return Visualization mode
      public: virtual DebugVisualizationMode DebugVisualization() const = 0;

      /// \brief Called by Scene when lighting changes so that
      /// GI can be updated
      public: virtual void LightingChanged() = 0;
    };
    }
  }
}
#endif
