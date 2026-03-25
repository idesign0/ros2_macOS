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
#include <gtest/gtest.h>

#include "gz/common/Skeleton.hh"
#include "gz/common/SkeletonAnimation.hh"

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;
class SkeletonAnimation : public common::testing::AutoLogFixture { };

/////////////////////////////////////////////////
TEST_F(SkeletonAnimation, CheckNoXDisplacement)
{
  common::SkeletonAnimation* skelAnim =
    new common::SkeletonAnimation("emptyAnimation");
  auto xDisplacement = skelAnim->XDisplacement();
  ASSERT_FALSE(xDisplacement);

  delete skelAnim;
}
