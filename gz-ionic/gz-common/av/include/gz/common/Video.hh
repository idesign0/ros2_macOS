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
#ifndef GZ_COMMON_VIDEO_HH_
#define GZ_COMMON_VIDEO_HH_

#include <string>

#include <gz/common/av/Export.hh>
#include <gz/utils/ImplPtr.hh>

struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPicture;
struct SwsContext;

namespace gz
{
  namespace common
  {
    /// \brief Handle video encoding and decoding using libavcodec
    class GZ_COMMON_AV_VISIBLE Video
    {
      /// \brief Constructor
      public: Video();

      /// \brief Destructor
      public: virtual ~Video();

      /// \brief Load a video file
      /// \param[in] _filename Full path of the video file
      /// \return false if  a video stream can't be found
      public: bool Load(const std::string &_filename);

      /// \brief Get the width of the video in pixels
      /// \return the width
      public: int Width() const;

      /// \brief Get the height of the video in pixels
      /// \return the height
      public: int Height() const;

      /// \brief Convenience type alias for duration
      /// where 1000000 is the same as AV_TIME_BASE fractional seconds
      public:
        using Length = std::chrono::duration<int64_t, std::ratio<1, 1000000>>;

      /// \brief Get the duration of the video
      /// \return the duration
      public: Length Duration() const;

      /// \brief Get the next frame of the video.
      /// \param[out] _buffer Allocated buffer in which the frame is stored
      ///                     (size has to be width * height * 3 bytes).
      /// \return false on error or end of file
      public: bool NextFrame(unsigned char **_buffer);

      /// \brief free up open Video object, close files, streams
      private: void Cleanup();

      /// \brief Private data pointer
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
  }
}
#endif
