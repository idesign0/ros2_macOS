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
#ifndef GZ_RENDERING_GLOBALILLUMINATIONCIVCT_HH_
#define GZ_RENDERING_GLOBALILLUMINATIONCIVCT_HH_

#include "gz/rendering/GlobalIlluminationBase.hh"

#include "gz/rendering/RenderTypes.hh"

#include <gz/math/Vector3.hh>

namespace gz
{
  namespace rendering
  {
  inline namespace GZ_RENDERING_VERSION_NAMESPACE {

  class GZ_RENDERING_VISIBLE CiVctCascade
  {
    /// \brief Constructor
    public: CiVctCascade();

    /// \brief Destructor
    public: virtual ~CiVctCascade();

    /// \brief Sets whether to correctly calculate GI occlusion caused
    /// by occluders against area lights. Consumes more VRAM.
    /// This option is not needed if you're not using area lights
    /// \remarks Recommended setting is true for the first cascade, false
    /// for the rest. Or just false if you don't plan on using area lights
    /// (saves memory and performance)
    /// \param[in] _correctAreaLightShadows True to enable the feature
    public: virtual void SetCorrectAreaLightShadows(
          bool _correctAreaLightShadows) = 0;

    /// \brief Retrieve current setting. See SetCorrectAreaLightShadows
    /// \return True if the feature is enabled
    public: virtual bool CorrectAreaLightShadows() const = 0;

    /// \brief Whether we should calculate the ideal multiplier based on
    /// lights on scene to minimize quantization loss from storing all
    /// GI data in RGBA8_UNORM_sRGB (which is not enough for HDR).
    /// For more info see Ogre's VctLighting::setBakingMultiplier
    /// \param[in] _autoMultiplier True to enable the feature. Default.
    public: virtual void SetAutoMultiplier(bool _autoMultiplier) = 0;

    /// \brief Retrieve current setting. See SetAutoMultiplier
    /// \return True if the feature is enabled
    public: virtual bool AutoMultiplier() const = 0;

    /// \copydoc GlobalIlluminationVct::SetThinWallCounter
    public: virtual void SetThinWallCounter(float _thinWallCounter) = 0;

    /// \brief Value of CiVctCascade::SetThinWallCounter
    /// \return ThinWallCounter value
    public: virtual float ThinWallCounter() const = 0;

    /// \brief Resolution of the 3D Voxel. Must be multiple of 2
    /// \param[in] _resolution Resolution
    public: virtual void SetResolution(const uint32_t _resolution[3]) = 0;

    /// \brief Returns 3D Voxel resolution
    /// \return Resolution in each axis. Length of array is 3
    public: virtual const uint32_t* Resolution() const = 0;

    /// \brief Number of divisions per axis to divide the scene
    /// into multiple octants. Setting this value > 1 *may* improve
    /// voxelization performance but it may also decrease it.
    /// Profiling is required for each scene
    /// \param[in] _octants Number of octant subdivisions per axis
    public: virtual void SetOctantCount(const uint32_t _octants[3]) = 0;

    /// \brief Returns number of octant subdivisions per axis
    /// \return Octant subdivisions. Length of array is 3
    public: virtual const uint32_t* OctantCount() const = 0;

    /// \brief The area in units around the camera to voxelize
    /// Each cascade area size must be >= than the previous one
    /// i.e.
    ///   assert( cascade[i]->AreaHalfSize >= cascade[i-1]->AreaHalfSize );
    ///
    /// For best results, area half sizes from each cascade should be multiples
    /// of each other
    /// \param[in] _areaHalfSize
    public: virtual void SetAreaHalfSize(
          const gz::math::Vector3d &_areaHalfSize) = 0;

    /// \brief Returns area in units around the camera to voxelize
    /// \return area in units around the camera to voxelize
    public: virtual gz::math::Vector3d AreaHalfSize() const = 0;

    /// \brief How much we let the camera move before updating the cascade
    /// Value is in range [1; inf)
    ///
    /// Camera is evaluated in quantized steps. i.e.
    ///     stepSize = cameraStepSize * 2.0 * areaHalfSize / resolution
    ///     stepSize = cameraStepSize * getVoxelCellSize()
    ///
    /// If cameraStepSize = 1, after the camera moves stepSize units,
    /// we will move & update the cascades
    /// If cameraStepSize = 2, after the camera moves 2 * stepSize units,
    /// we will move & update the cascades
    ///
    /// Small step sizes may cause too much brightness jumping as VCT may not
    /// be stable.
    /// Very big step sizes may cause periodic performance spikes or sudden
    /// changes in brightness.
    /// \param[in] _stepSize Step size in units
    public: virtual void SetCameraStepSize(
          const gz::math::Vector3d &_stepSize) = 0;

    /// \brief Returns the camera step size
    /// \return Camera step size
    public: virtual gz::math::Vector3d CameraStepSize() const = 0;
  };

  /// \class GlobalIlluminationCiVct GlobalIlluminationCiVct.hh
  /// gz/rendering/GlobalIlluminationCiVct.hh
  /// \brief Implements Global Illumination using CIVCT
  /// (Cascaded Image Voxel Cone Tracing)
  class GZ_RENDERING_VISIBLE GlobalIlluminationCiVct :
        public GlobalIlluminationBase
  {
    /// \enum DebugVisualizationMode
    /// \brief Debug visualization modes for
    /// GlobalIlluminationCiVct::SetDebugVisualization
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
    public: GlobalIlluminationCiVct();

    /// \brief Destructor
    public: virtual ~GlobalIlluminationCiVct();

    /// \brief Tells how many times AddCascade will be called.
    /// You can call it fewer times (i.e. some kb of RAM will be wasted)
    /// but not more.
    /// \remarks You can't call this function after adding cascades
    /// The semantics are similar to std::vector::reserve
    /// \param[in] _maxCascades Number of times AddCascade will be called
    public: virtual void SetMaxCascades(const uint32_t _maxCascades) = 0;

    /// \brief Adds a new cascade
    /// \remark Cannot be called anymore after Start() has been called
    /// \param[in] _ref Can be nullptr. If you already have a cascade,
    /// we will clone its settings.
    /// \return New Cascade to control its settings.
    public: virtual CiVctCascadePtr AddCascade(const CiVctCascade *_ref) = 0;

    /// \brief Removes the last added cascade.
    /// \remark Do NOT try to call any of the functions of the CiVctCascadePtr
    /// returned by AddCascade after this call.
    /// \remark Cannot be called anymore after Start() has been called
    public: virtual void PopCascade() = 0;

    /// \brief Alters each cascade's step size.
    /// The last cascade is set to stepSize.
    /// The rest of the cascades are set to step sizes that are >= stepSize
    /// automatically.
    /// Should be called after adding all cascades.
    /// \param[in] _stepSize See CiVctCascade::SetCameraStepSize
    public: virtual void AutoCalculateStepSizes(
        const gz::math::Vector3d &_stepSize) = 0;

    /// \brief Selects how we determine when we need to (partially) rebuild
    /// the voxels based on camera movement.
    /// \remarks
    /// If camera movement is deterministic, output is always deterministic
    /// regardless of this setting
    /// \param[in] _consistentCascadeSteps
    /// True:
    /// Results can feel very determnistic because it is predictable.
    ///
    /// Camera position is quantized in voxelCellSize *
    /// cascade.cameraStepSize.
    /// This means the camera position is in a 'cell' or 'grid'.
    ///
    /// When the camera position moves onto another grid, we partially
    /// revoxelize results.
    ///
    /// Advantage: A camera at pos XYZ will always have the same results
    ///
    /// Disadvantage: if the camera jumps back and forth between voxels,
    /// revoxelizations will trigger frequently causing obvious frequent
    /// jumps in brightness
    ///
    /// False:
    /// Revoxelization happens after the camera has travelled
    /// cascade.cameraStepSize cells away from the last point of
    /// voxelization.
    ///
    /// Advantage: Infrequent revoxelizations. If camera movement is
    /// restricted around the last voxelization point, we will never
    /// revoxelize.
    ///
    /// Disadvantage: Taking a picture at pos XYZ, then going away, and
    /// taking another picture at same pos XYZ may not result in the same
    /// brightness / image; which can make it feel unpredictable or
    /// non-deterministic. This can be workarounded by temporarily setting
    /// SetConsistentCascadeSteps(true) then back false after taking the
    /// picture
    public: virtual void SetConsistentCascadeSteps(
        bool _consistentCascadeSteps) = 0;

    /// \brief Retrieve current setting. See SetConsistentCascadeSteps
    /// \return True if the feature is enabled
    public: virtual bool ConsistentCascadeSteps() const = 0;

    /// \brief Returns true if Start() has already been called.
    /// Will return false again if Reset() is called.
    /// \return true if Start() has already been called
    public: virtual bool Started() const = 0;

    /// \brief Call this function after adding all cascades.
    /// You can no longer add cascades after this
    /// \param[in] _bounceCount Number of bounces for cascade 0.
    /// The rest of the cascades are autocalculated to maintain even
    /// brightness levels.
    ///
    /// Range is [0; inf) but a value of 0 is strongly discouraged if you
    /// have more than 1 cascade as you can end up with very uneven
    /// brightness levels between cascades
    /// For more info see Ogre's VctLighting::update
    /// \param[in] _anisotropic Whether we should use anisotropic VCT
    /// (higher quality, but consumes more VRAM).
    /// See GlobalIlluminationVct::SetAnisotropic
    public: virtual void Start(uint32_t _bounceCount, bool _anisotropic) = 0;

    /// \brief Sets new settings after having called Init.
    /// \param[in] _bounceCount See Start
    /// \param[in] _anisotropic See Start
    public: virtual void NewSettings(
        uint32_t _bounceCount, bool _anisotropic) = 0;

    /// \brief Disables this GI and undoes the internal work performed by
    /// Start(). This makes it possible to edit cascades again.
    public: virtual void Reset() = 0;

    /// \brief CIVCT relies on having multiple cascades around a central
    /// position, typically a camera. This function binds a camera for us
    /// to monitor and will act as the center of those cascades
    /// \remarks If no camera is bound, the active camera will be
    /// used, which can cause performance problems if multiple sensors
    /// are present
    /// \param[in] _camera Camera to bind. Nullptr to unbind.
    public: virtual void Bind(const CameraPtr &_camera) = 0;

    /// \brief Whether anisotropic setting is on
    /// \return Anisotropy setting
    public: virtual bool Anisotropic() const = 0;

    /// \brief True for high quality (slower). VRAM consumption
    /// stays the same. Ogre2 will use 6 cones instead of 4
    /// \param[in] _highQuality Quality setting
    public: virtual void SetHighQuality(bool _highQuality) = 0;

    /// \brief Whether we're using 6 (true) or 4 (false) cones
    /// \return High Quality setting
    public: virtual bool HighQuality() const = 0;

    /// \brief Draws the voxels on screen for inspection and understand what
    /// is going on with GI. You should be looking at a minecraft-like world
    /// \param[in] _dvm What component to visualize
    public: virtual void SetDebugVisualization(DebugVisualizationMode _dvm) = 0;

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
