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
#ifndef GZ_COMMON_GEOSPATIAL_HEIGHTMAPDATA_HH_
#define GZ_COMMON_GEOSPATIAL_HEIGHTMAPDATA_HH_

#include <string>
#include <vector>
#include <gz/math/Vector3.hh>
#include <gz/common/geospatial/Export.hh>

namespace gz
{
  namespace common
  {
    /// \brief Encapsulates a generic heightmap data file.
    class GZ_COMMON_GEOSPATIAL_VISIBLE HeightmapData
    {
      /// \brief Destructor.
      public: virtual ~HeightmapData() = default;

      /// \brief Create a lookup table of the terrain's height.
      /// \param[in] _subsampling Multiplier used to increase the resolution.
      /// Ex: A subsampling of 2 in a terrain of 129x129 means that the height
      /// vector will be 257 * 257.
      /// \param[in] _vertSize Number of points per row.
      /// \param[in] _size Real dimensions of the terrain.
      /// \param[in] _scale Vector3 used to scale the height.
      /// \param[in] _flipY If true, it inverts the order in which the vector
      /// is filled.
      /// \param[out] _heights Vector containing the terrain heights.
      public: virtual void FillHeightMap(int _subSampling,
          unsigned int _vertSize, const math::Vector3d &_size,
          const math::Vector3d &_scale, bool _flipY,
          std::vector<float> &_heights) const = 0;

      /// \brief Get the terrain's height.
      /// \return The terrain's height [pixels].
      public: virtual unsigned int Height() const = 0;

      /// \brief Get the terrain's width.
      /// \return The terrain's width [pixels].
      public: virtual unsigned int Width() const = 0;

      /// \brief Get the maximum terrain's elevation.
      /// \return The maximum terrain's elevation.
      public: virtual float MaxElevation() const = 0;

      /// \brief Get the min terrain's elevation.
      /// \return The min terrain's elevation.
      public: virtual float MinElevation() const
              {
                return 0.0f;
              }

      /// \brief Get the full filename of loaded heightmap image/dem
      /// \return The filename used to load the heightmap image/dem
      public: virtual std::string Filename() const = 0;
    };
  }
}
#endif
