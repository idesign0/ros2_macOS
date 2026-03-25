/*
 * Copyright (C) 2019 Open Source Robotics Foundation
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

#include "gz/msgs/image.pb.h"

/////////////////////////////////////////////////
TEST(Image, PixelFormat)
{
  // Check that pixel format enum is properly set
  gz::msgs::Image msg;
  EXPECT_EQ(
    static_cast<int>(gz::msgs::PixelFormatType::UNKNOWN_PIXEL_FORMAT),
    static_cast<int>(msg.pixel_format_type()));

  msg.set_pixel_format_type(gz::msgs::PixelFormatType::RGB_INT8);
  EXPECT_EQ(
    static_cast<int>(gz::msgs::PixelFormatType::RGB_INT8),
    static_cast<int>(msg.pixel_format_type()));
}
