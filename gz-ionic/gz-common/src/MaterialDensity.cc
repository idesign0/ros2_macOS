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
#include <cmath>
#include <limits>
#include "gz/common/EnumIface.hh"
#include "gz/common/MaterialDensity.hh"

using namespace gz;
using namespace common;

// Initialize enum iterator, and string converter
GZ_ENUM(materialDensityIface, MaterialDensity::Type,
  MaterialDensity::Type::BEGIN,
  MaterialDensity::Type::END,
  "Styrofoam",
  "Pine",
  "Wood",
  "Oak",
  "Ice",
  "Water",
  "Plastic",
  "Concrete",
  "Aluminum",
  "Steel, Alloy",
  "Steel, Stainless",
  "Iron",
  "Brass",
  "Copper",
  "Tungsten",
  "end"
)

// Initialize the materials
std::map<MaterialDensity::Type, double> MaterialDensity::materials =
{
  {MaterialDensity::Type::STYROFOAM, 75.0},
  {MaterialDensity::Type::PINE, 373.0},
  {MaterialDensity::Type::WOOD, 700.0},
  {MaterialDensity::Type::OAK, 710.0},
  {MaterialDensity::Type::ICE, 916.0},
  {MaterialDensity::Type::WATER, 1000.0},
  {MaterialDensity::Type::PLASTIC, 1175.0},
  {MaterialDensity::Type::CONCRETE, 2000.0},
  {MaterialDensity::Type::ALUMINUM, 2700.0},
  {MaterialDensity::Type::STEEL_ALLOY, 7600.0},
  {MaterialDensity::Type::STEEL_STAINLESS, 7800.0},
  {MaterialDensity::Type::IRON, 7870.0},
  {MaterialDensity::Type::BRASS, 8600.0},
  {MaterialDensity::Type::COPPER, 8940.0},
  {MaterialDensity::Type::TUNGSTEN, 19300.0}
};

/////////////////////////////////////////////////
const std::map<MaterialDensity::Type, double> &MaterialDensity::Materials()
{
  return materials;
}

/////////////////////////////////////////////////
double MaterialDensity::Density(const std::string &_material)
{
  MaterialDensity::Type type = MaterialDensity::Type::END;
  materialDensityIface.Set(type, _material);

  if (type != MaterialDensity::Type::END)
    return materials[type];
  else
    return -1;
}

/////////////////////////////////////////////////
double MaterialDensity::Density(const MaterialDensity::Type _material)
{
  return materials[_material];
}

/////////////////////////////////////////////////
std::tuple<MaterialDensity::Type, double> MaterialDensity::Nearest(
    const double _value, const double _epsilon)
{
  double min = std::numeric_limits<double>::max();
  std::tuple<MaterialDensity::Type, double> result
  {
    MaterialDensity::Type::END, -1.0
  };

  for (auto const &mat : materials)
  {
    double diff = std::fabs(mat.second - _value);
    if (diff < min && diff < _epsilon)
    {
      min = diff;
      result = mat;
    }
  }

  return result;
}

/////////////////////////////////////////////////
MaterialDensity::Type MaterialDensity::NearestMaterial(const double _value,
    const double _epsilon)
{
  return std::get<0>(Nearest(_value, _epsilon));
}
