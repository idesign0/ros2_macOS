/*
 * Copyright (C) 2025 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_GEOSPATIAL_HEIGHTMAPUTIL_HH_
#define GZ_COMMON_GEOSPATIAL_HEIGHTMAPUTIL_HH_

#include <memory>
#include <string>

#include <gz/math/SphericalCoordinates.hh>

#include <gz/common/geospatial/Export.hh>
#include "gz/common/geospatial/HeightmapData.hh"

namespace gz
{
  namespace common
  {
      /// \brief Load a heightmap from file
      /// \param[in] _filename Path to heightmap file
      /// \param[in] _sphericalCoordinates The spherical coordinates
      /// object contained in the world. This is used if the underlying
      /// heightmap type is DEM.
      /// \return heightmap
      std::unique_ptr<HeightmapData>
      GZ_COMMON_GEOSPATIAL_VISIBLE loadHeightmapData(
                  const std::string &_filename,
                  const math::SphericalCoordinates &_sphericalCoordinates =
                  math::SphericalCoordinates());

      /// \brief Check if input file has a file extension that can be
      /// loaded as an ImageHeightmap
      /// \param[in] _filename Path to heightmap file
      /// \return True if the filename has a file extension that is supported.
      bool GZ_COMMON_GEOSPATIAL_VISIBLE isSupportedImageHeightmapFileExtension(
                  const std::string &_filename);
  }
}
#endif
