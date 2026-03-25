/*
 * Copyright (C) 2017 Open Source Robotics Foundation
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
#include "gz/fuel_tools/Model.hh"

using namespace gz;
using namespace fuel_tools;

/////////////////////////////////////////////////
/// \brief Nothing crashes
TEST(Model, ConstructorDoesNotCrash)
{
  const Model constModel;
  EXPECT_FALSE(constModel);

  Model model;
  EXPECT_FALSE(model);

  Result result = model.Fetch();
  EXPECT_EQ(ResultType::UNKNOWN, result.Type());

  std::string path = model.PathToModel();
  EXPECT_TRUE(path.empty());
}
