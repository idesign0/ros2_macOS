/*
 * Copyright (C) 2025 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <memory>
#include <ostream>
#include <string>
#include <utility>

#include "gz/common/Console.hh"
#include "gz/common/StringUtils.hh"
#include "gz/common/Util.hh"

#ifndef BAZEL_DISABLE_DEM_LOADER
#include "gz/common/geospatial/Dem.hh"
#endif

#include "gz/common/geospatial/HeightmapUtil.hh"
#include "gz/common/geospatial/ImageHeightmap.hh"

namespace gz
{
namespace common
{

//////////////////////////////////////////////////
bool isSupportedImageHeightmapFileExtension(const std::string &_filename)
{
  if (_filename.empty())
    return false;

  std::string lowerFullPath = common::lowercase(_filename);
  return common::EndsWith(lowerFullPath, ".png")
       || common::EndsWith(lowerFullPath, ".jpg")
       || common::EndsWith(lowerFullPath, ".jpeg");
}

//////////////////////////////////////////////////
std::unique_ptr<HeightmapData> loadHeightmapData(
    const std::string &_filename,
    const math::SphericalCoordinates &_sphericalCoordinates)
{
  if (_filename.empty())
    return nullptr;

  std::unique_ptr<HeightmapData> data;
  // check if heightmap is an image
  if (isSupportedImageHeightmapFileExtension(_filename))
  {
    auto imgHeightmap = std::make_unique<ImageHeightmap>();
    if (imgHeightmap->Load(_filename) < 0)
    {
      gzerr << "Failed to load heightmap data from ["
             << _filename << "]" << std::endl;

      return nullptr;
    }
    data = std::move(imgHeightmap);
  }
  else
  {
// This macro is used by bazel build only. Dem classes are not built
// due to missing gdal dependency in BCR. So diable loading Dem heightmaps.
#ifndef BAZEL_DISABLE_DEM_LOADER
    // try loading as DEM
    auto dem = std::make_unique<Dem>();
    dem->SetSphericalCoordinates(_sphericalCoordinates);
    if (dem->Load(_filename) < 0)
    {
      gzerr << "Failed to load heightmap data from ["
             << _filename << "]" << std::endl;
      return nullptr;
    }
    data = std::move(dem);
#else
    (void)_sphericalCoordinates;
    gzerr << "Unable to load heightmap. DEM loading disabled."  << std::endl;
#endif
  }

  return data;
}

}
}
