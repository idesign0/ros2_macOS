/*
 * Copyright (C) 2015 Open Source Robotics Foundation
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

#include <memory>

#include "gz/rendering/Image.hh"

/// \brief Shared pointer to raw image buffer
typedef std::shared_ptr<unsigned char> DataPtr;

/// \brief Private fields of Image
class gz::rendering::Image::Implementation
{
  /// \brief Image width in pixels
  public: unsigned int width = 0;

  /// \brief Image height in pixels
  public: unsigned int height = 0;

  /// \brief Image pixel format
  public: PixelFormat format = PF_UNKNOWN;

  /// \brief Pointer to the image data
  public: DataPtr data = nullptr;
};

using namespace gz;
using namespace rendering;

//////////////////////////////////////////////////
template <class T>
struct ArrayDeleter
{
  void operator () (T const * p)
  {
    delete [] p;
  }
};

//////////////////////////////////////////////////
Image::Image()
  : dataPtr(utils::MakeImpl<Implementation>())
{
}

//////////////////////////////////////////////////
Image::Image(unsigned int _width, unsigned int _height,
  PixelFormat _format)
  : dataPtr(utils::MakeImpl<Implementation>())
{
  this->dataPtr->width = _width;
  this->dataPtr->height = _height;
  this->dataPtr->format = PixelUtil::Sanitize(_format);
  unsigned int size = this->MemorySize();
  this->dataPtr->data =
      DataPtr(new unsigned char[size], ArrayDeleter<unsigned char>());
}

//////////////////////////////////////////////////
Image::~Image() = default;

//////////////////////////////////////////////////
unsigned int Image::Width() const
{
  return this->dataPtr->width;
}

//////////////////////////////////////////////////
unsigned int Image::Height() const
{
  return this->dataPtr->height;
}

//////////////////////////////////////////////////
PixelFormat Image::Format() const
{
  return this->dataPtr->format;
}

//////////////////////////////////////////////////
unsigned int Image::Depth() const
{
  return PixelUtil::ChannelCount(this->dataPtr->format);
}

//////////////////////////////////////////////////
unsigned int Image::MemorySize() const
{
  return PixelUtil::MemorySize(this->dataPtr->format, this->dataPtr->width,
      this->dataPtr->height);
}

//////////////////////////////////////////////////
const void *Image::Data() const
{
  return this->dataPtr->data.get();
}

//////////////////////////////////////////////////
void *Image::Data()
{
  return this->dataPtr->data.get();
}
