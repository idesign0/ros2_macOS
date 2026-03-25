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
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include <gz/common/Image.hh>

#include "gz/common/testing/AutoLogFixture.hh"
#include "gz/common/testing/TestPaths.hh"

using namespace gz;

class ImageTest : public common::testing::AutoLogFixture { };

const std::string kTestDataGazeboJpeg =  // NOLINT(*)
    common::testing::TestFile("data", "gazebo_logo.jpeg");
const std::string kTestDataGazeboBmp =  // NOLINT(*)
    common::testing::TestFile("data", "gazebo_logo.bmp");
const std::string kTestData =  // NOLINT(*)
    common::testing::TestFile("data", "red_blue_colors.png");
const std::string kTestDataRGB16 =  // NOLINT(*)
    common::testing::TestFile("data", "rgb_16bit.png");
const std::string kTestDataRGBA16 =  // NOLINT(*)
    common::testing::TestFile("data", "rgba_16bit.png");

const auto kWidth = 121u;
const auto kHeight = 81u;

const auto kBpp_RGB = 24u;
const int kPitch_RGB = kWidth * kBpp_RGB/8;
const unsigned kSize_RGB = kPitch_RGB * kHeight;

const auto kBpp_RGBA = 32u;
const int kPitch_RGBA = kWidth * kBpp_RGBA/8;
const unsigned kSize_RGBA = kPitch_RGBA * kHeight;

const auto kAvgColor = math::Color(0.661157f, 0, 0.338843f, 1);
const auto kMaxColor = math::Color::Red;

/////////////////////////////////////////////////
void CheckImageRGB(const common::Image &_img)
{
  ASSERT_TRUE(_img.Valid());
  ASSERT_EQ(kWidth, _img.Width());
  ASSERT_EQ(kHeight, _img.Height());
  ASSERT_EQ(kBpp_RGB, _img.BPP());
  ASSERT_EQ(kPitch_RGB, _img.Pitch());
  ASSERT_EQ(common::Image::PixelFormatType::RGB_INT8, _img.PixelFormat());
}

/////////////////////////////////////////////////
void CheckImageRGBA(const common::Image &_img)
{
  ASSERT_TRUE(_img.Valid());
  ASSERT_EQ(kWidth, _img.Width());
  ASSERT_EQ(kHeight, _img.Height());
  ASSERT_EQ(kBpp_RGBA, _img.BPP());
  ASSERT_EQ(kPitch_RGBA, _img.Pitch());
  ASSERT_EQ(common::Image::PixelFormatType::RGBA_INT8, _img.PixelFormat());
}

/////////////////////////////////////////////////
TEST_F(ImageTest, InvalidImage)
{
  common::Image img;
  ASSERT_EQ(-1, img.Load("/file/shouldn/never/exist.png"));
}

/////////////////////////////////////////////////
TEST_F(ImageTest, ImageConstructorProperties)
{
  common::Image imgInvalid("invalid");
  ASSERT_FALSE(imgInvalid.Valid());

  common::Image img(kTestData);
  CheckImageRGBA(img);

  ASSERT_EQ(img.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(kAvgColor, img.AvgColor());
  ASSERT_EQ(kMaxColor, img.MaxColor());

  ASSERT_TRUE(img.Filename().find("red_blue_colors.png") !=
      std::string::npos);
}

/////////////////////////////////////////////////
TEST_F(ImageTest, ImageConstructorPropertiesDifferentFormats)
{
  common::Image imgBmp(kTestDataGazeboBmp);

  ASSERT_EQ(554u, imgBmp.Width());
  ASSERT_EQ(234u, imgBmp.Height());
  ASSERT_EQ(24u, imgBmp.BPP());
  ASSERT_EQ(1662, imgBmp.Pitch());
  ASSERT_EQ(common::Image::PixelFormatType::RGB_INT8, imgBmp.PixelFormat());

  ASSERT_TRUE(imgBmp.Filename().find("gazebo_logo.bmp") !=
      std::string::npos);

  common::Image imgJpeg(kTestDataGazeboJpeg);

  ASSERT_EQ(554u, imgJpeg.Width());
  ASSERT_EQ(234u, imgJpeg.Height());
  ASSERT_EQ(24u, imgJpeg.BPP());
  ASSERT_EQ(1662, imgJpeg.Pitch());
  ASSERT_EQ(common::Image::PixelFormatType::RGB_INT8, imgJpeg.PixelFormat());

  ASSERT_TRUE(imgJpeg.Filename().find("gazebo_logo.jpeg") !=
      std::string::npos);
}

/////////////////////////////////////////////////
TEST_F(ImageTest, ImageProperties)
{
  common::Image img;
  ASSERT_EQ(-1, img.Load("/file/shouldn/never/exist.png"));

  // load image and test colors
  ASSERT_EQ(0, img.Load(kTestData));

  CheckImageRGBA(img);

  ASSERT_EQ(img.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(kAvgColor, img.AvgColor());
  ASSERT_EQ(kMaxColor, img.MaxColor());

  ASSERT_TRUE(img.Filename().find("red_blue_colors.png") !=
      std::string::npos);
}

TEST_F(ImageTest, ImageSavePng)
{
  std::string pathOut = common::testing::TempPath();
  common::createDirectories(pathOut);

  std::string imagePath = common::joinPaths(pathOut, "image.png");

  common::Image imgJpeg(kTestDataGazeboJpeg);
  imgJpeg.SavePNG(imagePath);
  common::Image img(imagePath);
  ASSERT_TRUE(img.Valid());

  std::vector<unsigned char> values;
  img.SavePNGToBuffer(values);
  EXPECT_LT(0u, values.size());
}

TEST_F(ImageTest, ImageGetterInvalid)
{
  common::Image img;
  EXPECT_EQ(0u, img.Width());
  EXPECT_EQ(0u, img.Height());
  EXPECT_EQ(0u, img.BPP());
  EXPECT_EQ(math::Color(), img.Pixel(0, 0));
  EXPECT_EQ(math::Color(), img.MaxColor());

  common::Image imgBmp(kTestDataGazeboBmp);
  EXPECT_EQ(math::Color(), imgBmp.Pixel(2000, 0));

  unsigned int width = imgBmp.Width();
  unsigned int height = imgBmp.Height();
  imgBmp.Rescale(static_cast<int>(width / 2), static_cast<int>(height / 2));

  EXPECT_EQ(static_cast<unsigned int>(width / 2), imgBmp.Width());
  EXPECT_EQ(static_cast<unsigned int>(height / 2), imgBmp.Height());
}

/////////////////////////////////////////////////
TEST_F(ImageTest, RGBData)
{
  // load image and test colors
  common::Image img;
  ASSERT_EQ(0, img.Load(kTestData));
  ASSERT_TRUE(img.Valid());

  CheckImageRGBA(img);

  // Check RGB data
  auto data = img.RGBData();
  ASSERT_EQ(kSize_RGB, data.size());

  auto channels = 3u;
  auto step = img.Width() * channels;
  for (auto i = 0u; i < img.Height(); ++i)
  {
    for (auto j = 0u; j < step; j += channels)
    {
      unsigned int idx = i * step + j;
      unsigned int r = data[idx];
      unsigned int g = data[idx+1];
      unsigned int b = data[idx+2];

      ASSERT_EQ(0u, g) << i << "  " << j;
      if (j / channels < 80)
      {
        ASSERT_EQ(255u, r) << i << "  " << j / channels;
        ASSERT_EQ(0u, b) << i << "  " << j / channels;
      }
      else
      {
        ASSERT_EQ(0u, r) << i << "  " << j / channels;
        ASSERT_EQ(255u, b) << i << "  " << j / channels;
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_F(ImageTest, Data)
{
  // load image and test colors
  common::Image img;
  ASSERT_EQ(0, img.Load(kTestData));
  ASSERT_TRUE(img.Valid());

  CheckImageRGBA(img);

  // Check RGBA data
  auto data = img.Data();
  ASSERT_EQ(kSize_RGBA, data.size());

  auto channels = 4u;
  auto step = img.Width() * channels;
  for (auto i = 0u; i < img.Height(); ++i)
  {
    for (auto j = 0u; j < step; j += channels)
    {
      unsigned int idx = i * step + j;
      unsigned int r = data[idx];
      unsigned int g = data[idx+1];
      unsigned int b = data[idx+2];
      unsigned int a = data[idx+3];

      ASSERT_EQ(0u, g) << i << "  " << j;
      ASSERT_EQ(255u, a) << i << "  " << j;
      if (j / channels < 80)
      {
        ASSERT_EQ(255u, r) << i << "  " << j / channels;
        ASSERT_EQ(0u, b) << i << "  " << j / channels;
      }
      else
      {
        ASSERT_EQ(0u, r) << i << "  " << j / channels;
        ASSERT_EQ(255u, b) << i << "  " << j / channels;
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_F(ImageTest, SetFromData)
{
  // load image and test colors
  common::Image img;
  ASSERT_EQ(0, img.Load(kTestData));
  ASSERT_TRUE(img.Valid());

  auto data = img.Data();
  ASSERT_EQ(39204u, data.size());

  common::Image img2;
  img2.SetFromData(&data[0], img.Width(), img.Height(), img.PixelFormat());
  ASSERT_TRUE(img2.Valid());
  ASSERT_EQ(common::Image::PixelFormatType::RGBA_INT8, img2.PixelFormat());
  ASSERT_EQ(121u, img2.Width());
  ASSERT_EQ(81u, img2.Height());
  ASSERT_EQ(32u, img2.BPP());
  ASSERT_EQ(484, img2.Pitch());
  ASSERT_EQ(img2.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img2.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(img2.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  ASSERT_EQ(img2.MaxColor(), math::Color::Red);
}

TEST_F(ImageTest, SetFromCompressedData)
{
  // Open file and move to end
  std::ifstream ifs(kTestData, std::ios::binary | std::ios::ate);
  std::ifstream::pos_type fileEnd = ifs.tellg();
  std::vector<unsigned char> fileData(fileEnd);

  // Rewind to beginning of file and read data
  ifs.seekg(0);
  ifs.read(reinterpret_cast<char *>(&fileData[0]), fileEnd);

  common::Image img;
  img.SetFromCompressedData(&fileData[0], fileEnd,
      common::Image::PixelFormatType::COMPRESSED_PNG);
  ASSERT_TRUE(img.Valid());

  auto data = img.RGBAData();
  ASSERT_EQ(39204u, data.size());

  ASSERT_EQ(common::Image::PixelFormatType::RGBA_INT8, img.PixelFormat());
  ASSERT_EQ(121u, img.Width());
  ASSERT_EQ(81u, img.Height());
  ASSERT_EQ(32u, img.BPP());
  ASSERT_EQ(484, img.Pitch());
  ASSERT_EQ(img.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(img.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  ASSERT_EQ(img.MaxColor(), math::Color::Red);

  // Reloading the image should not cause any leaks.
  img.SetFromCompressedData(&fileData[0], fileEnd,
      common::Image::PixelFormatType::COMPRESSED_PNG);
  ASSERT_TRUE(img.Valid());

  // Loading from unsupported pixel format fails.
  img.SetFromCompressedData(&fileData[0], fileEnd,
      common::Image::PixelFormatType::UNKNOWN_PIXEL_FORMAT);
  ASSERT_FALSE(img.Valid());
}

/*
  // save image then reload and test colors
  std::string testSaveImage =
    common::testing::TempPath("test_red_blue_save.png");
  img.SavePNG(testSaveImage);

  common::Image img2;  common::Image img;

  img2.Load(testSaveImage);
  ASSERT_TRUE(img2.Valid());
  ASSERT_EQ(common::Image::PixelFormatType::RGB_INT8, img2.PixelFormat());
  ASSERT_EQ(121u, img2.Width());
  ASSERT_EQ(81u, img2.Height());
  ASSERT_EQ(24u, img2.BPP());
  ASSERT_EQ(363, img2.Pitch());
  ASSERT_EQ(img2.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img2.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(img2.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  ASSERT_EQ(img2.MaxColor(), math::Color::Red);

  // Check data
  data = nullptr;
  size = 0;
  img2.Data(&data, size);
  ASSERT_EQ(29403u, size);
  ASSERT_NE(nullptr, data);

  channels = 3u;
  step = img2.Width() * channels;
  for (auto i = 0u; i < img2.Height(); ++i)
  {
    for (auto j = 0u; j < step; j += channels)
    {
      unsigned int idx = i * step + j;
      unsigned int r = data[idx];
      unsigned int g = data[idx+1];
      unsigned int b = data[idx+2];

      ASSERT_EQ(0u, g) << i << "  " << j;
      if (j / channels < 80)
      {
        ASSERT_EQ(255u, r) << i << "  " << j / channels;
        ASSERT_EQ(0u, b) << i << "  " << j / channels;
      }
      else
      {
        ASSERT_EQ(0u, r) << i << "  " << j / channels;
        ASSERT_EQ(255u, b) << i << "  " << j / channels;
      }
    }
  }

  common::Image img3;
  img3.SetFromData(data, img3.Width(), img3.Height(), img3.PixelFormat());
  ASSERT_TRUE(img3.Valid());

  ASSERT_EQ(common::Image::PixelFormatType::RGB_INT8, img3.PixelFormat());
  ASSERT_EQ(121u, img3.Width());
  ASSERT_EQ(81u, img3.Height());
  ASSERT_EQ(24u, img3.BPP());
  ASSERT_EQ(363, img3.Pitch());
  ASSERT_EQ(img3.Pixel(0, 0), math::Color::Red);
  ASSERT_EQ(img3.Pixel(85, 0), math::Color::Blue);
  ASSERT_EQ(img3.AvgColor(), math::Color(0.661157f, 0, 0.338843f, 1));
  ASSERT_EQ(img3.MaxColor(), math::Color::Red);
}
*/

/////////////////////////////////////////////////
TEST_F(ImageTest, ConvertPixelFormat)
{
  using Image = gz::common::Image;
  EXPECT_EQ(Image::PixelFormatType::UNKNOWN_PIXEL_FORMAT,
         Image::ConvertPixelFormat("fake"));
  EXPECT_EQ(Image::PixelFormatType::UNKNOWN_PIXEL_FORMAT,
         Image::ConvertPixelFormat("unknown"));
  EXPECT_EQ(Image::PixelFormatType::UNKNOWN_PIXEL_FORMAT,
         Image::ConvertPixelFormat("UNKNOWN_PIXEL_FORMAT"));
  EXPECT_EQ(Image::PixelFormatType::L_INT8,
         Image::ConvertPixelFormat("L_INT8"));
  EXPECT_EQ(Image::PixelFormatType::L_INT16,
         Image::ConvertPixelFormat("L_INT16"));
  EXPECT_EQ(Image::PixelFormatType::RGB_INT8,
         Image::ConvertPixelFormat("RGB_INT8"));
  EXPECT_EQ(Image::PixelFormatType::RGBA_INT8,
         Image::ConvertPixelFormat("RGBA_INT8"));
  EXPECT_EQ(Image::PixelFormatType::RGB_INT16,
         Image::ConvertPixelFormat("RGB_INT16"));
  EXPECT_EQ(Image::PixelFormatType::RGB_INT32,
         Image::ConvertPixelFormat("RGB_INT32"));
  EXPECT_EQ(Image::PixelFormatType::BGR_INT8,
         Image::ConvertPixelFormat("BGR_INT8"));
  EXPECT_EQ(Image::PixelFormatType::BGRA_INT8,
         Image::ConvertPixelFormat("BGRA_INT8"));
  EXPECT_EQ(Image::PixelFormatType::BGR_INT16,
         Image::ConvertPixelFormat("BGR_INT16"));
  EXPECT_EQ(Image::PixelFormatType::BGR_INT32,
         Image::ConvertPixelFormat("BGR_INT32"));
  EXPECT_EQ(Image::PixelFormatType::R_FLOAT16,
         Image::ConvertPixelFormat("R_FLOAT16"));
  EXPECT_EQ(Image::PixelFormatType::R_FLOAT32,
         Image::ConvertPixelFormat("R_FLOAT32"));
  EXPECT_EQ(Image::PixelFormatType::RGB_FLOAT16,
         Image::ConvertPixelFormat("RGB_FLOAT16"));
  EXPECT_EQ(Image::PixelFormatType::RGB_FLOAT32,
         Image::ConvertPixelFormat("RGB_FLOAT32"));
  EXPECT_EQ(Image::PixelFormatType::BAYER_RGGB8,
         Image::ConvertPixelFormat("BAYER_RGGB8"));
  EXPECT_EQ(Image::PixelFormatType::BAYER_GBRG8,
         Image::ConvertPixelFormat("BAYER_GBRG8"));
  EXPECT_EQ(Image::PixelFormatType::BAYER_GRBG8,
         Image::ConvertPixelFormat("BAYER_GRBG8"));
  EXPECT_EQ(Image::PixelFormatType::BAYER_BGGR8,
         Image::ConvertPixelFormat("BAYER_BGGR8"));
  EXPECT_EQ(Image::PixelFormatType::COMPRESSED_PNG,
         Image::ConvertPixelFormat("COMPRESSED_PNG"));
}

/////////////////////////////////////////////////
TEST_F(ImageTest, ConvertToRGBImage)
{
  unsigned int width = 8;
  unsigned int height = 8;
  unsigned int size = width * height;

  // test L_INT8 format
  {
    // create sample image data for testing
    // the image is divided into 4 sections from top to bottom
    // The values in the sections are 10, 20, 30, 40
    auto buffer = std::vector<uint8_t>(size);
    for (unsigned int i = 0; i < height; ++i)
    {
      uint8_t v = 10 * static_cast<int>(i / (width/ 4.0) + 1);
      for (unsigned int j = 0; j < width; ++j)
      {
        buffer[i*width + j] = v;
      }
    }

    common::Image output;
    common::Image::ConvertToRGBImage<uint8_t>(
        buffer.data(), width, height, output);

    // Check RGBA data
    auto data = output.Data();
    EXPECT_EQ(size * 3, data.size());

    for (unsigned int i = 0u; i < height; ++i)
    {
      for (unsigned int j = 0u; j < width; ++j)
      {
        unsigned int r = data[i * width * 3 + j * 3];
        unsigned int g = data[i * width * 3 + j * 3 + 1];
        unsigned int b = data[i * width * 3 + j * 3 + 2];
        EXPECT_EQ(r, g);
        EXPECT_EQ(r, b);
        if (i < (height / 4.0))
          EXPECT_EQ(0u, r);
        else if (i >= (height / 4.0) && i < (height / 2.0))
          EXPECT_EQ(static_cast<unsigned int>(255 / 3), r);
        else if (i >= (height / 2.0) && i < (height / 4.0 * 3.0))
          EXPECT_EQ(static_cast<unsigned int>(255 / 3 * 2), r);
        else
          EXPECT_EQ(255u, r);
      }
    }
  }

  // test L_INT16 format
  {
    // create sample image data for testing
    // the image is divided into 4 sections from top to bottom
    // The values in the sections are 100, 200, 300, 400
    auto buffer = std::vector<uint16_t>(size);
    for (unsigned int i = 0; i < height; ++i)
    {
      uint16_t v = 100 * static_cast<int>(i / (height / 4.0) + 1);
      for (unsigned int j = 0; j < width; ++j)
      {
        buffer[i*width + j] = v;
      }
    }

    common::Image output;
    common::Image::ConvertToRGBImage<uint16_t>(
        buffer.data(), width, height, output);

    // Check RGB data
    auto data = output.Data();
    EXPECT_EQ(size * 3, data.size());

    for (unsigned int i = 0u; i < height; ++i)
    {
      for (unsigned int j = 0u; j < width; ++j)
      {
        unsigned int r = data[i * width * 3 + j * 3];
        unsigned int g = data[i * width * 3 + j * 3 + 1];
        unsigned int b = data[i * width * 3 + j * 3 + 2];
        EXPECT_EQ(r, g);
        EXPECT_EQ(r, b);

        if (i < (height / 4.0))
          EXPECT_EQ(0u, r);
        else if (i >= (height / 4.0) && i < (height / 2.0))
          EXPECT_EQ(static_cast<unsigned int>(255 / 3), r);
        else if (i >= (height / 2.0) && i < (height / 4.0 * 3.0))
          EXPECT_EQ(static_cast<unsigned int>(255 / 3 * 2), r);
        else
          EXPECT_EQ(255u, r);
      }
    }
  }

  // test R_FLOAT32 format
  {
    // create sample image data for testing
    // the image is divided into 4 sections from top to bottom
    // The values in the sections are 0.5, 1.0, 1.5, 2.0
    auto buffer = std::vector<float>(size);
    for (unsigned int i = 0; i < height; ++i)
    {
      float v = 0.5f * static_cast<int>(i / (height / 4.0) + 1);
      for (unsigned int j = 0; j < width; ++j)
      {
        buffer[i*width + j] = v;
      }
    }

    common::Image output;
    common::Image::ConvertToRGBImage<float>(
        buffer.data(), width, height, output);

    // Check RGB data
    auto data = output.Data();
    EXPECT_EQ(size * 3, data.size());

    for (unsigned int i = 0u; i < height; ++i)
    {
      for (unsigned int j = 0u; j < width; ++j)
      {
        unsigned int r = data[i * width * 3 + j * 3];
        unsigned int g = data[i * width * 3 + j * 3 + 1];
        unsigned int b = data[i * width * 3 + j * 3 + 2];
        EXPECT_EQ(r, g);
        EXPECT_EQ(r, b);

        if (i < (height / 4.0))
          EXPECT_EQ(0u, r);
        else if (i >= (height / 4.0) && i < (height / 2.0))
          EXPECT_EQ(static_cast<unsigned int>(255 / 3), r);
        else if (i >= (height / 2.0) && i < (height / 4.0 * 3.0))
          EXPECT_EQ(static_cast<unsigned int>(255 / 3 * 2), r);
        else
          EXPECT_EQ(255u, r);
      }
    }
  }

  // test R_FLOAT32 format with min, max, and flip values set
  {
    // create sample image data for testing
    // the image is divided into 4 sections from top to bottom
    // The values in the sections are 0.5, 1.0, 1.5, 2.0
    auto buffer = std::vector<float>(size);
    for (unsigned int i = 0; i < height; ++i)
    {
      float v = 0.5f * static_cast<int>(i / (height / 4.0) + 1);
      for (unsigned int j = 0; j < width; ++j)
      {
        buffer[i*width + j] = v;
      }
    }

    float min = 0.0f;
    float max = 5.0f;
    common::Image output;
    common::Image::ConvertToRGBImage<float>(
        buffer.data(), width, height, output, min, max, true);

    // Check RGB data
    auto data = output.Data();
    EXPECT_EQ(size * 3, data.size());

    for (unsigned int i = 0u; i < height; ++i)
    {
      for (unsigned int j = 0u; j < width; ++j)
      {
        unsigned int r = data[i * width * 3 + j * 3];
        unsigned int g = data[i * width * 3 + j * 3 + 1];
        unsigned int b = data[i * width * 3 + j * 3 + 2];
        EXPECT_EQ(r, g);
        EXPECT_EQ(r, b);

        // values should be normalized by min, max and flipped
        float v = 0.5f * static_cast<int>(i / (height / 4.0) + 1);
        unsigned int expectedValue = static_cast<unsigned int>(
            (1.0f - ((v - min) / (max - min))) * 255);
        EXPECT_EQ(expectedValue, r);
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_F(ImageTest, Grayscale)
{
  {
    common::Image img;
    std::string fileName = common::testing::TestFile("data",
        "grayscale_8bit.png");
    EXPECT_EQ(0, img.Load(fileName));
    unsigned int width = 4u;
    unsigned int height = 4u;
    unsigned int bits = 8u;
    EXPECT_TRUE(img.Valid());
    EXPECT_EQ(width, img.Width());
    EXPECT_EQ(height, img.Height());
    EXPECT_EQ(bits, img.BPP());
    EXPECT_EQ(width * bits / 8u, img.Pitch());
    EXPECT_EQ(common::Image::PixelFormatType::L_INT8, img.PixelFormat());
    math::Color maxColor(0.847f, 0.847f, 0.847f);
    EXPECT_NEAR(maxColor.R(), img.MaxColor().R(), 1e-3);
    EXPECT_NEAR(maxColor.G(), img.MaxColor().G(), 1e-3);
    EXPECT_NEAR(maxColor.B(), img.MaxColor().B(), 1e-3);
  }

  {
    common::Image img;
    std::string fileName = common::testing::TestFile("data",
        "grayscale_16bit.png");
    EXPECT_EQ(0, img.Load(fileName));
    unsigned int width = 4u;
    unsigned int height = 4u;
    unsigned int bits = 16u;
    EXPECT_TRUE(img.Valid());
    EXPECT_EQ(width, img.Width());
    EXPECT_EQ(height, img.Height());
    EXPECT_EQ(bits, img.BPP());
    EXPECT_EQ(width * bits / 8u, img.Pitch());
    EXPECT_EQ(common::Image::PixelFormatType::L_INT16, img.PixelFormat());
    math::Color maxColor(0.847f, 0.847f, 0.847f);
    EXPECT_NEAR(maxColor.R(), img.MaxColor().R(), 1e-3);
    EXPECT_NEAR(maxColor.G(), img.MaxColor().G(), 1e-3);
    EXPECT_NEAR(maxColor.B(), img.MaxColor().B(), 1e-3);
  }
}


/////////////////////////////////////////////////
TEST_F(ImageTest, Color16bit)
{
  {
    common::Image img;
    std::string fileName = common::testing::TestFile("data",
        "rgb_16bit.png");
    EXPECT_EQ(0, img.Load(fileName));
    const unsigned int width = 4u;
    const unsigned int height = 4u;
    const unsigned int channels = 3u;
    const unsigned int bpp = channels * 16u;
    EXPECT_TRUE(img.Valid());
    EXPECT_EQ(width, img.Width());
    EXPECT_EQ(height, img.Height());
    EXPECT_EQ(bpp, img.BPP());
    EXPECT_EQ(width * bpp / 8u, img.Pitch());
    EXPECT_EQ(common::Image::PixelFormatType::RGB_INT16, img.PixelFormat());
    math::Color maxColor(0.0f, 0.0f, 0.847f);
    EXPECT_NEAR(maxColor.R(), img.MaxColor().R(), 1e-3);
    EXPECT_NEAR(maxColor.G(), img.MaxColor().G(), 1e-3);
    EXPECT_NEAR(maxColor.B(), img.MaxColor().B(), 1e-3);
    math::Color pixelWithMaxColor = img.Pixel(3, 1);
    EXPECT_NEAR(maxColor.R(), pixelWithMaxColor.R(), 1e-3);
    EXPECT_NEAR(maxColor.G(), pixelWithMaxColor.G(), 1e-3);
    EXPECT_NEAR(maxColor.B(), pixelWithMaxColor.B(), 1e-3);
  }
  {
    common::Image img;
    std::string fileName = common::testing::TestFile("data",
        "rgba_16bit.png");
    EXPECT_EQ(0, img.Load(fileName));
    const unsigned int width = 4u;
    const unsigned int height = 4u;
    const unsigned int channels = 4u;
    const unsigned int bpp = channels * 16u;
    EXPECT_TRUE(img.Valid());
    EXPECT_EQ(width, img.Width());
    EXPECT_EQ(height, img.Height());
    EXPECT_EQ(bpp, img.BPP());
    EXPECT_EQ(width * bpp / 8u, img.Pitch());
    math::Color maxColor(0.0f, 0.0f, 0.847f, 0.5f);
    EXPECT_NEAR(maxColor.R(), img.MaxColor().R(), 1e-3);
    EXPECT_NEAR(maxColor.G(), img.MaxColor().G(), 1e-3);
    EXPECT_NEAR(maxColor.B(), img.MaxColor().B(), 1e-3);
    EXPECT_NEAR(maxColor.A(), img.MaxColor().A(), 1e-3);
    math::Color pixelWithMaxColor = img.Pixel(3, 1);
    EXPECT_NEAR(maxColor.R(), pixelWithMaxColor.R(), 1e-3);
    EXPECT_NEAR(maxColor.G(), pixelWithMaxColor.G(), 1e-3);
    EXPECT_NEAR(maxColor.B(), pixelWithMaxColor.B(), 1e-3);
    EXPECT_NEAR(maxColor.A(), pixelWithMaxColor.A(), 1e-3);
  }
}

/////////////////////////////////////////////////
TEST_F(ImageTest, ChannelData)
{
  // load image, extract data from each channel and verify values
  common::Image img;
  ASSERT_EQ(0, img.Load(kTestData));
  ASSERT_TRUE(img.Valid());

  std::vector<unsigned char> red =
      img.ChannelData(common::Image::Channel::RED);
  std::vector<unsigned char> green =
      img.ChannelData(common::Image::Channel::GREEN);
  std::vector<unsigned char> blue =
      img.ChannelData(common::Image::Channel::BLUE);
  std::vector<unsigned char> alpha =
      img.ChannelData(common::Image::Channel::ALPHA);

  EXPECT_FALSE(red.empty());
  ASSERT_EQ(img.Width() * img.Height(), red.size());
  ASSERT_EQ(red.size(), green.size());
  ASSERT_EQ(red.size(), blue.size());
  ASSERT_EQ(red.size(), alpha.size());

  for (auto i = 0u; i < img.Height(); ++i)
  {
    for (auto j = 0u; j < img.Width(); ++j)
    {
      unsigned int idx = i * img.Width() + j;
      unsigned int r = red[idx];
      unsigned int g = green[idx];
      unsigned int b = blue[idx];
      unsigned int a = alpha[idx];

      ASSERT_EQ(0u, g);
      ASSERT_EQ(255u, a);
      if (j < 80)
      {
        ASSERT_EQ(255u, r);
        ASSERT_EQ(0u, b);
      }
      else
      {
        ASSERT_EQ(0u, r);
        ASSERT_EQ(255u, b);
      }
    }
  }
}

/////////////////////////////////////////////////
TEST_F(ImageTest, ChannelData16bit)
{
  {
    // load load 16 bit RGBA image
    // extract data from each channel and verify values
    common::Image img;
    ASSERT_EQ(0, img.Load(kTestDataRGBA16));
    ASSERT_TRUE(img.Valid());

    std::vector<unsigned char> red =
        img.ChannelData(common::Image::Channel::RED);
    std::vector<unsigned char> green =
        img.ChannelData(common::Image::Channel::GREEN);
    std::vector<unsigned char> blue =
        img.ChannelData(common::Image::Channel::BLUE);
    std::vector<unsigned char> alpha =
        img.ChannelData(common::Image::Channel::ALPHA);

    EXPECT_FALSE(red.empty());
    ASSERT_EQ(img.Width() * img.Height(), red.size());
    ASSERT_EQ(red.size(), green.size());
    ASSERT_EQ(red.size(), blue.size());
    ASSERT_EQ(red.size(), alpha.size());

    for (auto i = 0u; i < img.Height(); ++i)
    {
      for (auto j = 0u; j < img.Width(); ++j)
      {
        unsigned int idx = i * img.Width() + j;
        unsigned int r = red[idx];
        unsigned int g = green[idx];
        unsigned int b = blue[idx];
        unsigned int a = alpha[idx];

        EXPECT_EQ(0u, r);
        EXPECT_EQ(0u, g);
        EXPECT_LT(0u, b);
        EXPECT_EQ(128u, a);
      }
    }
  }

  {
    // load 16 bit RGB image (no alpha channel)
    // extra data from each channel and verify values
    common::Image img;
    ASSERT_EQ(0, img.Load(kTestDataRGB16));
    ASSERT_TRUE(img.Valid());

    std::vector<unsigned char> red =
        img.ChannelData(common::Image::Channel::RED);
    std::vector<unsigned char> green =
        img.ChannelData(common::Image::Channel::GREEN);
    std::vector<unsigned char> blue =
        img.ChannelData(common::Image::Channel::BLUE);
    std::vector<unsigned char> alpha =
        img.ChannelData(common::Image::Channel::ALPHA);

    EXPECT_FALSE(red.empty());
    ASSERT_EQ(img.Width() * img.Height(), red.size());
    ASSERT_EQ(red.size(), green.size());
    ASSERT_EQ(red.size(), blue.size());
    ASSERT_EQ(0u, alpha.size());

    for (auto i = 0u; i < img.Height(); ++i)
    {
      for (auto j = 0u; j < img.Width(); ++j)
      {
        unsigned int idx = i * img.Width() + j;
        unsigned int r = red[idx];
        unsigned int g = green[idx];
        unsigned int b = blue[idx];

        EXPECT_EQ(0u, r);
        EXPECT_EQ(0u, g);
        EXPECT_LT(0u, b);
      }
    }
  }
}

using string_int2 = std::tuple<const char *, unsigned int, unsigned int>;

class ImagePerformanceTest : public ImageTest,
                             public ::testing::WithParamInterface<string_int2>
{
  /// \brief Loads an image specified by _filename and checks its max color.
  /// \param[in] _filePath Relative path to the image file to check.
  /// \param[in] _width Width of the image in pixels.
  /// \param[in] _height Height of the image in pixels.
  public: void MaxColor(const std::string &_filePath,
                        const unsigned int _width,
                        const unsigned int _height);
};

TEST_P(ImagePerformanceTest, MaxColorFlatHeightmap)
{
  MaxColor(std::get<0>(GetParam()), std::get<1>(GetParam()),
           std::get<2>(GetParam()));
}

void ImagePerformanceTest::MaxColor(const std::string &_filePath,
                                    const unsigned int _width,
                                    const unsigned int _height)
{
  std::string fileName = common::testing::TestFile("data", _filePath);

  common::Image img;
  EXPECT_EQ(0, img.Load(fileName));
  EXPECT_EQ(_width, img.Width());
  EXPECT_EQ(_height, img.Height());
  EXPECT_EQ(gz::math::Color(0., 0., 0., 0), img.MaxColor());
  EXPECT_TRUE(img.Valid());
}

INSTANTIATE_TEST_SUITE_P(FlatHeightmaps, ImagePerformanceTest,
  ::testing::Values(
    std::make_tuple("heightmap_flat_129x129.png", 129u, 129u),
    std::make_tuple("heightmap_flat_257x257.png", 257u, 257u),
    std::make_tuple("heightmap_flat_513x513.png", 513u, 513u),
    std::make_tuple("heightmap_flat_1025x1025.png", 1025u, 1025u)));
