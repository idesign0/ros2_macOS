/*
 * Copyright (C) 2016 Open Source Robotics Foundation
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
#include <algorithm>
#include <limits>

#include <gdal_priv.h>
#include <ogr_spatialref.h>

#include "gz/common/Console.hh"
#include "gz/common/geospatial/Dem.hh"
#include "gz/common/Util.hh"

using namespace gz;
using namespace common;

class gz::common::Dem::Implementation
{
  /// \brief A set of associated raster bands.
  public: GDALDataset *dataSet;

  /// \brief A pointer to the band.
  public: GDALRasterBand *band;

  /// \brief Real width of the world in meters.
  public: double worldWidth;

  /// \brief Real height of the world in meters.
  public: double worldHeight;

  /// \brief Terrain's side (after the padding).
  public: unsigned int side;

  /// \brief The maximum length of data to load in the X direction.
  ///        By default, load the entire raster.
  public: unsigned int maxXSize {std::numeric_limits<unsigned int>::max()};

  /// \brief The maximum length of data to load in the Y direction.
  ///        By default, load the entire raster.
  public: unsigned int maxYSize {std::numeric_limits<unsigned int>::max()};

  /// \brief The desired length of data to load in the X direction.
  ///        Internally, the implementation may use a
  //         higher value for performance.
  public: unsigned int configuredXSize;

  /// \brief The desired length of data to load in the Y direction.
  ///        Internally, the implementation may use a
  //         higher value for performance.
  public: unsigned int configuredYSize;

  /// \brief Minimum elevation in meters.
  public: double minElevation;

  /// \brief Maximum elevation in meters.
  public: double maxElevation;

  /// \brief Value used to mark padding buffer data.
  public: float bufferVal{std::numeric_limits<float>::max()};

  /// \brief DEM data converted to be OGRE-compatible.
  public: std::vector<float> demData;

  /// \brief Full filename used to load the dem.
  public: std::string filename;

  /// \brief Whether the DEM will be handled as unknown.
  /// If true, worldWidth & worldHeight = -1
  /// and GeoReference[Origin] can not be used (will return false)
  public: bool isUnknownDem = false;

  /// \brief Holds the spherical coordinates object from the world.
  public: math::SphericalCoordinates sphericalCoordinates =
           math::SphericalCoordinates();

  /// \brief Once the user configures size limits, apply that
  ///    to the internal configured size, which is limited
  ///    based on the dataset size.
  /// \return True if configured size was valid.
  public: [[nodiscard]] bool ConfigureLoadedSize();
};

//////////////////////////////////////////////////
Dem::Dem()
: dataPtr(gz::utils::MakeImpl<Implementation>())
{
  this->dataPtr->dataSet = nullptr;
  GDALAllRegister();
}

//////////////////////////////////////////////////
Dem::~Dem()
{
  this->dataPtr->demData.clear();
  if (this->dataPtr->dataSet)
    GDALClose(GDALDataset::ToHandle(this->dataPtr->dataSet));
}

//////////////////////////////////////////////////
void Dem::SetSphericalCoordinates(
    const math::SphericalCoordinates &_worldSphericalCoordinates)
{
  this->dataPtr->sphericalCoordinates = _worldSphericalCoordinates;
}

//////////////////////////////////////////////////
unsigned int Dem::RasterXSizeLimit()
{
  return this->dataPtr->maxXSize;
}

//////////////////////////////////////////////////
void Dem::SetRasterXSizeLimit(const unsigned int &_xLimit)
{
  this->dataPtr->maxXSize = _xLimit;
}

//////////////////////////////////////////////////
unsigned int Dem::RasterYSizeLimit()
{
  return this->dataPtr->maxYSize;
}

//////////////////////////////////////////////////
void Dem::SetRasterYSizeLimit(const unsigned int &_yLimit)
{
  this->dataPtr->maxYSize = _yLimit;
}

//////////////////////////////////////////////////
int Dem::Load(const std::string &_filename)
{
  double upLeftX, upLeftY, upRightX, upRightY, lowLeftX, lowLeftY;
  gz::math::Angle upLeftLat, upLeftLong, upRightLat, upRightLong;
  gz::math::Angle lowLeftLat, lowLeftLong;

  // Sanity check
  std::string fullName = _filename;
  if (!exists(findFilePath(fullName)))
    fullName = common::findFile(_filename);

  this->dataPtr->filename = fullName;

  // Create a re-usable lambda for opening a dataset.
  auto openInGdal = [this](const std::string& name) -> bool
  {
    GDALDatasetH handle = GDALOpen(name.c_str(), GA_ReadOnly);
    if (handle)
    {
      this->dataPtr->dataSet = GDALDataset::FromHandle(handle);
    }

    return this->dataPtr->dataSet != nullptr;
  };

  if (!exists(findFilePath(fullName)))
  {
    // https://github.com/gazebosim/gz-common/issues/596
    // Attempt loading anyways to support /vsicurl, /vsizip, and other
    // GDAL Virtual File Formats.
    // The "exists()" function does not handle GDAL's special formats.
    if (!openInGdal(_filename)) {
      gzerr << "Unable to read DEM file[" << _filename << "]." << std::endl;
      return -1;
    }
  } else if(!openInGdal(fullName)){
    gzerr << "Unable to open DEM file[" << fullName
           << "]. Format not recognized as a supported dataset." << std::endl;
    return -1;
  }
  assert(this->dataPtr->dataSet != nullptr);

  int nBands = this->dataPtr->dataSet->GetRasterCount();
  if (nBands != 1)
  {
    gzerr << "Unsupported number of bands in file [" << fullName + "]. Found "
          << nBands << " but only 1 is a valid value." << std::endl;
    return -1;
  }

  // Set the pointer to the band
  this->dataPtr->band = this->dataPtr->dataSet->GetRasterBand(1);

  // Validate the raster size and apply the user-configured size limits
  //  on loaded data.
  if(!this->dataPtr->ConfigureLoadedSize())
  {
    return -1;
  }

  // Raster width and height
  const int xSize = this->dataPtr->configuredXSize;
  const int ySize = this->dataPtr->configuredYSize;

  // Corner coordinates
  upLeftX = 0.0;
  upLeftY = 0.0;
  upRightX = xSize;
  upRightY = 0.0;
  lowLeftX = 0.0;
  lowLeftY = ySize;

  // Calculate the georeferenced coordinates of the terrain corners
  if (this->GeoReference(upLeftX, upLeftY, upLeftLat, upLeftLong)
      && this->GeoReference(upRightX, upRightY, upRightLat, upRightLong)
      && this->GeoReference(lowLeftX, lowLeftY, lowLeftLat, lowLeftLong))
  {
    // If successful, set the world width and height
    this->dataPtr->worldWidth =
       this->dataPtr->sphericalCoordinates.DistanceBetweenPoints(
           upLeftLat, upLeftLong, upRightLat, upRightLong);
    this->dataPtr->worldHeight =
       this->dataPtr->sphericalCoordinates.DistanceBetweenPoints(
           upLeftLat, upLeftLong, lowLeftLat, lowLeftLong);
  }
  // Assume unknown DEM.
  else
  {
    gzwarn << "Failed to automatically compute DEM size. "
            << std::endl;

    this->dataPtr->worldWidth = this->dataPtr->worldHeight = -1;
    this->dataPtr->isUnknownDem = true;
  }

  // Set the terrain's side (the terrain will be squared after the padding)

  unsigned int height;
  if (gz::math::isPowerOfTwo(ySize - 1))
    height = ySize;
  else
    height = gz::math::roundUpPowerOfTwo(ySize) + 1;

  unsigned int width;
  if (gz::math::isPowerOfTwo(xSize - 1))
    width = xSize;
  else
    width = gz::math::roundUpPowerOfTwo(xSize) + 1;

  //! @todo use by limits.
  this->dataPtr->side = std::max(width, height);

  // Preload the DEM's data
  if (this->LoadData() != 0)
    return -1;

  // Check for nodata value in dem data. This is used when computing the
  // min elevation. If nodata value is not defined, we assume it will be one
  // of the commonly used values such as -9999, -32768, etc.
  // See https://desktop.arcgis.com/en/arcmap/10.8/manage-data/raster-and-images/nodata-in-raster-datasets.htm
  // For simplicity, we will treat values <= -9999 as nodata values and
  // ignore them when computing the min elevation.
  int validNoData = 0;
  const double defaultNoDataValue = -9999;
  float noDataValue = this->dataPtr->band->GetNoDataValue(&validNoData);

  if (validNoData <= 0)
    noDataValue = defaultNoDataValue;

  double min = gz::math::MAX_D;
  double max = -gz::math::MAX_D;
  for (const auto &d : this->dataPtr->demData)
  {
    if (math::equal(d, this->dataPtr->bufferVal))
      continue;

    // All comparisons to NaN return false, so guard against NaN NoData
    if (!std::isnan(noDataValue) &&  math::equal(d, noDataValue))
      continue;

    if (!std::isfinite(d))
    {
      continue;
    }

    if (d < min)
      min = d;
    if (d > max)
      max = d;
  }
  if (gz::math::equal(min, gz::math::MAX_D) ||
      gz::math::equal(max, -gz::math::MAX_D))
  {
    gzwarn << "The DEM contains 'nodata' values!" << std::endl;
  }

  this->dataPtr->minElevation = min;
  this->dataPtr->maxElevation = max;

  // Buffer to min elevation
  for (auto &d : this->dataPtr->demData)
  {
    if (math::equal(d, this->dataPtr->bufferVal))
      d = this->dataPtr->minElevation;
  }
  return 0;
}

//////////////////////////////////////////////////
double Dem::Elevation(double _x, double _y)
{
  if (_x >= this->Width() || _y >= this->Height())
  {
    gzerr << "Illegal coordinates. You are asking for the elevation in ("
           << _x << "," << _y << ") but the terrain is ["
           << this->Width() << " x " << this->Height() << "]" << std::endl;
    return std::numeric_limits<double>::infinity();
  }

  auto idx = static_cast<unsigned int>(_y) * this->Width()
      + static_cast<unsigned int>(_x);
  return this->dataPtr->demData.at(idx);
}

//////////////////////////////////////////////////
float Dem::MinElevation() const
{
  return this->dataPtr->minElevation;
}

//////////////////////////////////////////////////
float Dem::MaxElevation() const
{
  return this->dataPtr->maxElevation;
}

//////////////////////////////////////////////////
bool Dem::GeoReference(double _x, double _y,
    gz::math::Angle &_latitude, gz::math::Angle &_longitude) const
{
  if (this->dataPtr->isUnknownDem)
  {
    gzerr << "Can not retrieve coordinates from unknown DEM."
            << std::endl;
    return false;
  }

  double geoTransf[6];
  assert(this->dataPtr->dataSet != nullptr);
  if (this->dataPtr->dataSet->GetGeoTransform(geoTransf) == CE_None)
  {
    OGRCoordinateTransformation *cT = nullptr;
    double xGeoDeg, yGeoDeg;
    OGRSpatialReference sourceCs;
    OGRSpatialReference targetCs;

    if (this->dataPtr->sphericalCoordinates.Surface() ==
        math::SphericalCoordinates::EARTH_WGS84)
    {
      sourceCs.importFromWkt(this->dataPtr->dataSet->GetProjectionRef());
      targetCs.SetWellKnownGeogCS("WGS84");
    }
    else if ((this->dataPtr->sphericalCoordinates.Surface() ==
        math::SphericalCoordinates::CUSTOM_SURFACE) ||
        (this->dataPtr->sphericalCoordinates.Surface() ==
         math::SphericalCoordinates::MOON_SCS))
    {
      sourceCs = *(this->dataPtr->dataSet->GetSpatialRef());
      targetCs = OGRSpatialReference();

      double axisEquatorial =
        this->dataPtr->sphericalCoordinates.SurfaceAxisEquatorial();
      double axisPolar =
        this->dataPtr->sphericalCoordinates.SurfaceAxisPolar();

      std::string surfaceLatLongProjStr =
        "+proj=latlong +a=" + std::to_string(axisEquatorial) +
        " +b=" + std::to_string(axisPolar);

      targetCs.importFromProj4(surfaceLatLongProjStr.c_str());
    }

    cT = OGRCreateCoordinateTransformation(&sourceCs, &targetCs);

    if (nullptr == cT)
    {
      gzerr << "Unable to transform terrain coordinate system for "
             << "coordinates (" << _x << "," << _y << ")" << std::endl;
      OCTDestroyCoordinateTransformation(cT);
      return false;
    }

    xGeoDeg = geoTransf[0] + _x * geoTransf[1] + _y * geoTransf[2];
    yGeoDeg = geoTransf[3] + _x * geoTransf[4] + _y * geoTransf[5];

    cT->Transform(1, &xGeoDeg, &yGeoDeg);

    _latitude.SetDegree(yGeoDeg);
    _longitude.SetDegree(xGeoDeg);

    OCTDestroyCoordinateTransformation(cT);
  }
  else
  {
    gzdbg << "Unable to obtain the georeferenced values for coordinates ("
           << _x << "," << _y << ")" << std::endl;
    return false;
  }
  return true;
}

//////////////////////////////////////////////////
bool Dem::GeoReferenceOrigin(gz::math::Angle &_latitude,
    gz::math::Angle &_longitude) const
{
  return this->GeoReference(0, 0, _latitude, _longitude);
}

//////////////////////////////////////////////////
unsigned int Dem::Height() const
{
  return this->dataPtr->side;
}

//////////////////////////////////////////////////
unsigned int Dem::Width() const
{
  return this->dataPtr->side;
}

//////////////////////////////////////////////////
double Dem::WorldWidth() const
{
  if (this->dataPtr->isUnknownDem)
  {
    gzwarn << "Unable to determine world width of unknown DEM."
            << std::endl;
  }
  return this->dataPtr->worldWidth;
}

//////////////////////////////////////////////////
double Dem::WorldHeight() const
{
  if (this->dataPtr->isUnknownDem)
  {
    gzwarn << "Unable to determine world height of unknown DEM."
            << std::endl;
  }
  return this->dataPtr->worldHeight;
}

//////////////////////////////////////////////////
void Dem::FillHeightMap(int _subSampling, unsigned int _vertSize,
    const gz::math::Vector3d &_size,
    const gz::math::Vector3d &_scale,
    bool _flipY, std::vector<float> &_heights) const
{
  if (_subSampling <= 0)
  {
    gzerr << "Illegal subsampling value (" << _subSampling << ")\n";
    return;
  }

  // Resize the vector to match the size of the vertices.
  _heights.resize(_vertSize * _vertSize);

  // Iterate over all the vertices
  for (unsigned int y = 0; y < _vertSize; ++y)
  {
    double yf = y / static_cast<double>(_subSampling);
    unsigned int y1 = static_cast<unsigned int>(floor(yf));
    unsigned int y2 = static_cast<unsigned int>(ceil(yf));
    if (y2 >= this->dataPtr->side)
      y2 = this->dataPtr->side - 1;
    double dy = yf - y1;

    for (unsigned int x = 0; x < _vertSize; ++x)
    {
      double xf = x / static_cast<double>(_subSampling);
      unsigned int x1 = static_cast<unsigned int>(floor(xf));
      unsigned int x2 = static_cast<unsigned int>(ceil(xf));
      if (x2 >= this->dataPtr->side)
        x2 = this->dataPtr->side - 1;
      double dx = xf - x1;

      double px1 = this->dataPtr->demData[y1 * this->dataPtr->side + x1];
      double px2 = this->dataPtr->demData[y1 * this->dataPtr->side + x2];
      float h1 = (px1 - ((px1 - px2) * dx));

      double px3 = this->dataPtr->demData[y2 * this->dataPtr->side + x1];
      double px4 = this->dataPtr->demData[y2 * this->dataPtr->side + x2];
      float h2 = (px3 - ((px3 - px4) * dx));

      float h = this->dataPtr->minElevation +
          (h1 - ((h1 - h2) * dy) - this->dataPtr->minElevation) * _scale.Z();

      // Invert pixel definition so 1=ground, 0=full height,
      // if the terrain size has a negative z component
      // this is mainly for backward compatibility
      if (_size.Z() < 0)
        h *= -1;

      // Convert to minElevation if a NODATA value is found
      if (_size.Z() >= 0 && h < this->dataPtr->minElevation)
        h = this->dataPtr->minElevation;

      // Store the height for future use
      if (!_flipY)
        _heights[y * _vertSize + x] = h;
      else
        _heights[(_vertSize - y - 1) * _vertSize + x] = h;
    }
  }
}

bool gz::common::Dem::Implementation::ConfigureLoadedSize()
{
  assert(this->dataSet != nullptr);
  const unsigned int nRasterXSize = this->dataSet->GetRasterXSize();
  const unsigned int nRasterYSize = this->dataSet->GetRasterYSize();
  if (nRasterXSize == 0 || nRasterYSize == 0)
  {
    gzerr << "Illegal raster size loading a DEM file (" << nRasterXSize << ","
          << nRasterYSize << ")\n";
    return false;
  }

  this->configuredXSize = std::min(nRasterXSize, this->maxXSize);
  this->configuredYSize = std::min(nRasterYSize, this->maxYSize);
  return true;
}

//////////////////////////////////////////////////
int Dem::LoadData()
{
  // Capture a local for re-use.
  const auto desiredXSize = this->dataPtr->configuredXSize;
  const auto desiredYSize = this->dataPtr->configuredYSize;

  // Scale the terrain keeping the same ratio between width and height
  unsigned int destWidth;
  unsigned int destHeight;
  float ratio;
  if (desiredXSize > desiredYSize)
  {
    ratio = static_cast<float>(desiredXSize) / static_cast<float>(desiredYSize);
    destWidth = this->dataPtr->side;
    // The decimal part is discarted for interpret the result as pixels
    float h = static_cast<float>(destWidth) / static_cast<float>(ratio);
    destHeight = static_cast<unsigned int>(h);
  }
  else
  {
    ratio = static_cast<float>(desiredYSize) / static_cast<float>(desiredXSize);
    destHeight = this->dataPtr->side;
    // The decimal part is discarted for interpret the result as pixels
    float w = static_cast<float>(destHeight) / static_cast<float>(ratio);
    destWidth = static_cast<unsigned int>(w);
  }

  // Read the whole raster data and convert it to a GDT_Float32 array.
  // In this step the DEM is scaled to destWidth x destHeight.

  std::vector<float> buffer;
  // Convert to uint64_t for multiplication to avoid overflow.
  // https://github.com/OSGeo/gdal/issues/9713
  buffer.resize(static_cast<uint64_t>(destWidth) * \
    static_cast<uint64_t>(destHeight));
  //! @todo Do not assume users only want to load
  //! from the origin of the dataset.
  // Instead, add a configuration to change where in the dataset to read from.
  if (this->dataPtr->band->RasterIO(GF_Read, 0, 0,
                       desiredXSize, desiredYSize, &buffer[0],
                       destWidth, destHeight, GDT_Float32, 0, 0) != CE_None)
  {
    gzerr << "Failure calling RasterIO while loading a DEM file\n";
    return -1;
  }

  // Copy and align 'buffer' into the target vector. The destination vector is
  // initialized to max() and later converted to the minimum elevation, so all
  // the points not contained in 'buffer' will be extra padding.
  this->dataPtr->demData.resize(static_cast<uint64_t>(this->Width()) * \
    static_cast<uint64_t>(this->Height()),
      this->dataPtr->bufferVal);
  for (unsigned int y = 0; y < destHeight; ++y)
  {
    std::copy(&buffer[destWidth * y], &buffer[destWidth * y] + destWidth,
              this->dataPtr->demData.begin() + this->Width() * y);
  }
  buffer.clear();

  return 0;
}

//////////////////////////////////////////////////
std::string Dem::Filename() const
{
  return this->dataPtr->filename;
}
