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
#ifndef GZ_COMMON_IMAGE_HH_
#define GZ_COMMON_IMAGE_HH_

#include <cstring>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <gz/math/Color.hh>

#include <gz/common/graphics/Export.hh>

#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \brief String names for the pixel formats.
    /// \sa Image::PixelFormat.
    static std::string PixelFormatNames[] =
    {
      "UNKNOWN_PIXEL_FORMAT",
      "L_INT8",
      "L_INT16",
      "RGB_INT8",
      "RGBA_INT8",
      "BGRA_INT8",
      "RGB_INT16",
      "RGB_INT32",
      "BGR_INT8",
      "BGR_INT16",
      "BGR_INT32",
      "R_FLOAT16",
      "RGB_FLOAT16",
      "R_FLOAT32",
      "RGB_FLOAT32",
      "BAYER_RGGB8",
      "BAYER_BGGR8",
      "BAYER_GBRG8",
      "BAYER_GRBG8",
      "COMPRESSED_PNG"
    };

    /// \class Image Image.hh gz/common/common.hh
    /// \brief Encapsulates an image
    class GZ_COMMON_GRAPHICS_VISIBLE Image
    {
      /// \brief Image channel
      public: enum class Channel
              {
                /// \brief Red channel
                RED = 0,
                /// \brief Green channel
                GREEN = 1,
                /// \brief Blue channel
                BLUE = 2,
                /// \brief Alpha channel
                ALPHA = 3
              };

      /// \brief Pixel formats enumeration
      public: enum PixelFormatType
              {
                UNKNOWN_PIXEL_FORMAT = 0,
                L_INT8,
                L_INT16,
                RGB_INT8,
                RGBA_INT8,
                BGRA_INT8,
                RGB_INT16,
                RGB_INT32,
                BGR_INT8,
                BGR_INT16,
                BGR_INT32,
                R_FLOAT16,
                RGB_FLOAT16,
                R_FLOAT32,
                RGB_FLOAT32,
                BAYER_RGGB8,
                BAYER_BGGR8,
                BAYER_GBRG8,
                BAYER_GRBG8,
                COMPRESSED_PNG,
                PIXEL_FORMAT_COUNT,
                // \todo(iche033) COMPRESSED_JPEG is added at the end to
                // preserve ABI compatibility. Move this enum up when merging
                // forward to main
                COMPRESSED_JPEG
              };


      /// \brief Convert a string to a Image::PixelFormat.
      /// \param[in] _format Pixel format string. \sa Image::PixelFormatNames
      /// \return Image::PixelFormat
      public: static Image::PixelFormatType ConvertPixelFormat(
                  const std::string &_format);

      /// \brief Constructor
      /// \param[in] _filename the path to the image
      public: explicit Image(const std::string &_filename = "");

      /// \brief Destructor
      public: virtual ~Image();

      /// \brief Load an image. Return 0 on success
      /// \param[in] _filename the path to the image file
      /// \return 0 when the operation succeeds to open a file or -1 when fails.
      public: int Load(const std::string &_filename);

      /// \brief Save the image in PNG format
      /// \param[in] _filename The name of the saved image
      public: void SavePNG(const std::string &_filename);

      /// \brief Get the PNG image in a buffer
      /// \param[out] _buffer Buffer with the data
      public: void SavePNGToBuffer(std::vector<unsigned char> &_buffer);

      /// \brief Set the image from raw data
      /// \param[in] _data Pointer to the raw image data
      /// \param[in] _width Width in pixels
      /// \param[in] _height Height in pixels
      /// \param[in] _format Pixel format of the provided data
      public: void SetFromData(const unsigned char *_data,
                               unsigned int _width,
                               unsigned int _height,
                               Image::PixelFormatType _format);

      /// \brief Set the image from compressed (i.e. png) data
      /// \param[in] _data Pointer to the raw image data
      /// \param[in] _size Size of the buffer
      /// \param[in] _format Pixel format of the provided data
      public: void SetFromCompressedData(unsigned char *_data,
                                         unsigned int _size,
                                         Image::PixelFormatType _format);

      /// \brief Get the image as a data array
      /// \return The image data
      public: std::vector<unsigned char> Data() const;

      /// \brief Get only the RGB data from the image. This will drop the
      /// alpha channel if one is present.
      /// \return The image RGB data
      public: std::vector<unsigned char> RGBData() const;

      /// \brief Get the RGBA data from the image. This will add an alpha
      /// channel if one is not present.
      /// \return The image RGBA data
      public: std::vector<unsigned char> RGBAData() const;

      /// \brief Get the width
      /// \return The image width
      public: unsigned int Width() const;

      /// \brief Get the height
      /// \return The image height
      public: unsigned int Height() const;

      /// \brief Get the size of one pixel in bits
      /// \return The BPP of the image
      public: unsigned int BPP() const;

      // \brief Get the size of a row of pixel
      /// \return The pitch of the image
      public: int Pitch() const;

      /// \brief Get the full filename of the image
      /// \return The filename used to load the image
      public: std::string Filename() const;

      /// \brief Get the pixel format
      /// \return PixelFormat
      public: PixelFormatType PixelFormat() const;

      /// \brief Get a pixel color value
      /// \param[in] _x Column location in the image
      /// \param[in] _y Row location in the image
      /// \return The color of the given pixel
      public: math::Color Pixel(const unsigned int _x,
                  const unsigned int _y) const;

      /// \brief Get the average color
      /// \return The average color
      public: math::Color AvgColor() const;

      /// \brief Get the max color
      /// \return The max color
      public: math::Color MaxColor() const;

      /// \brief Rescale the image
      /// \param[in] _width New image width
      /// \param[in] _height New image height
      public: void Rescale(const int _width, const int _height);

      /// \brief Returns whether this is a valid image
      /// \return true if image has a bitmap
      public: bool Valid() const;

      /// \brief Extract a single channel (red, green, blue, or alpha) from
      /// an RGB[A] image and return a single channel 8 bit image data.
      /// \param[in] _channel Channel to extract
      /// \return 8 bit single channel image data
      public: std::vector<unsigned char> ChannelData(Channel _channel) const;

      /// \brief Convert a single channel image data buffer into an RGB image.
      /// During the conversion, the input image data are normalized to 8 bit
      /// values i.e. [0, 255]. Optionally, specify min and max values to use
      /// when normalizing the input image data. For example, if min and max
      /// are set to 1 and 10, a data value 2 will be normalized to:
      ///    (2 - 1) / (10 - 1) * 255.
      /// \param[in] _data input image data buffer
      /// \param[in] _width image width
      /// \param[in] _height image height
      /// \param[out] _output Output RGB image
      /// \param[in] _min Minimum value to be used when normalizing the input
      /// image data to RGB.
      /// \param[in] _max Maximum value to be used when normalizing the input
      /// image data to RGB.
      /// \param[in] _flip True to flip the values after normalization, i.e.
      /// lower values are converted to brigher pixels.
      public: template<typename T>
          static void ConvertToRGBImage(const void *_data,
          unsigned int _width, unsigned int _height, Image &_output,
          T _min = std::numeric_limits<T>::max(),
          T _max = std::numeric_limits<T>::lowest(), bool _flip = false)
      {
        unsigned int samples = _width * _height;
        unsigned int bufferSize = samples * sizeof(T);

        auto buffer = std::vector<T>(samples);
        memcpy(buffer.data(), _data, bufferSize);

        auto outputRgbBuffer = std::vector<uint8_t>(samples * 3);

        // use min and max values found in the data if not specified
        T min = std::numeric_limits<T>::max();
        T max = std::numeric_limits<T>::lowest();
        if (_min > max)
        {
          for (unsigned int i = 0; i < samples; ++i)
          {
            auto v = buffer[i];
            // ignore inf values when computing min/max
            // cast to float when calling isinf to avoid compile error on
            // windows
            if (v > max && !std::isinf(static_cast<float>(v)))
              max = v;
            if (v < min && !std::isinf(static_cast<float>(v)))
              min = v;
          }
        }
        min = math::equal(_min, std::numeric_limits<T>::max()) ? min : _min;
        max = math::equal(_max, std::numeric_limits<T>::lowest()) ? max : _max;

        // convert to rgb image
        // color is grayscale, i.e. r == b == g
        double range = static_cast<double>(max - min);
        if (gz::math::equal(range, 0.0))
          range = 1.0;
        unsigned int idx = 0;
        for (unsigned int j = 0; j < _height; ++j)
        {
          for (unsigned int i = 0; i < _width; ++i)
          {
            auto v = buffer[idx++];
            double t = static_cast<double>(v - min) / range;
            if (_flip)
              t = 1.0 - t;
            uint8_t r = static_cast<uint8_t>(255*t);
            unsigned int outIdx = j * _width * 3 + i * 3;
            outputRgbBuffer[outIdx] = r;
            outputRgbBuffer[outIdx + 1] = r;
            outputRgbBuffer[outIdx + 2] = r;
          }
        }
        _output.SetFromData(outputRgbBuffer.data(), _width, _height, RGB_INT8);
      }

      /// \brief Private data pointer
      GZ_UTILS_IMPL_PTR(dataPtr)
    };
  }
}
#endif
