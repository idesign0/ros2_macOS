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
#include <gtest/gtest.h>

#include "gz/common/MaterialDensity.hh"
#include "gz/common/testing/AutoLogFixture.hh"

using namespace gz;
using namespace common;

class MaterialDensityTest : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(MaterialDensityTest, Init)
{
  EXPECT_TRUE(!MaterialDensity::Materials().empty());
}

/////////////////////////////////////////////////
TEST_F(MaterialDensityTest, Accessors)
{
  {
    double density = MaterialDensity::Density("Aluminum");
    double density2 = MaterialDensity::Density(MaterialDensity::Type::ALUMINUM);

    EXPECT_DOUBLE_EQ(density, 2700);
    EXPECT_DOUBLE_EQ(density, density2);
  }
  {
    double density = MaterialDensity::Density("Notfoundium");
    EXPECT_DOUBLE_EQ(density, -1.0);
  }
  {
    MaterialDensity::Type material;
    double density;
    std::tie(material, density) = MaterialDensity::Nearest(19300.0);
    EXPECT_EQ(material, MaterialDensity::Type::TUNGSTEN);
    EXPECT_DOUBLE_EQ(density,
        MaterialDensity::Density(MaterialDensity::NearestMaterial(19300.0)));
  }
  {
    MaterialDensity::Type material;
    double density;
    std::tie(material, density) = MaterialDensity::Nearest(1001001.001, 1e-3);
    EXPECT_EQ(material, MaterialDensity::Type::END);
    EXPECT_DOUBLE_EQ(density, -1.0);
  }
  {
    MaterialDensity::Type material;
    double density;
    std::tie(material, density) = MaterialDensity::Nearest(1001001.001);
    EXPECT_EQ(material, MaterialDensity::Type::TUNGSTEN);
    EXPECT_DOUBLE_EQ(density, 19300);
  }
}
