/*
 * Copyright 2016 Open Source Robotics Foundation
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
#ifndef GZ_COMMON_VIDEOENCODER_HH_
#define GZ_COMMON_VIDEOENCODER_HH_

#include <chrono>
#include <string>
#include <optional>

#include <gz/common/FlagSet.hh>
#include <gz/common/av/Export.hh>
#include <gz/common/HWVideo.hh>
#include <gz/utils/ImplPtr.hh>

// Default bitrate (0) indicates that a bitrate should be calculated when
// Start is called.
#define VIDEO_ENCODER_BITRATE_DEFAULT 0
#define VIDEO_ENCODER_WIDTH_DEFAULT 1280
#define VIDEO_ENCODER_HEIGHT_DEFAULT 720
#define VIDEO_ENCODER_FPS_DEFAULT 25
#define VIDEO_ENCODER_FORMAT_DEFAULT "mp4"

namespace gz
{
  namespace common
  {
    /// \brief The VideoEncoder class supports encoding a series of images
    /// to a video format, and then writing the video to disk.
    class GZ_COMMON_AV_VISIBLE VideoEncoder
    {
      /// \brief Constructor
      public: VideoEncoder();

      /// \brief Destructor
      public: virtual ~VideoEncoder();

      /// \brief Start the encoder. This should be called once. Add new
      /// frames to the video using the AddFrame function. Use SaveToFile
      /// when the video is complete.
      /// \param[in] _width Width in pixels of the output video.
      /// \param[in] _height Height in pixels of the output video.
      /// \param[in] _format String that represents the video type.
      /// Supported types include: "avi", "ogv", mp4", "v4l2". If using
      /// "v4l2", you must also specify a _filename.
      /// \param[in] _bitRate Bit rate to encode the video. A value of zero
      /// will cause this function to automatically compute a bitrate.
      /// \param[in] _filename Name of the file that stores the video while it
      /// is being created. This is a temporary file when recording to
      /// disk, or a video4linux loopback device like /dev/video0 when
      /// the _format is "v4l2". If blank, a default temporary file is used.
      /// However, the "v4l2" _format must be accompanied with a video
      /// loopback device filename.
      /// \return True on success. Do NOT ignore the return value. If Start()
      /// failed, encoding will not work. If Start() function failed, you can
      /// try running the overload with _allowHwAccel=false to see if the
      /// failure isn't a result of faulty HW encoding (e.g. when NVENC sessions
      /// are exhausted).
      /// \note This will automatically select a HW-accelerated encoder based
      /// on the values of environment variables GZ_VIDEO_ALLOWED_ENCODERS,
      /// GZ_VIDEO_ENCODER_DEVICE and GZ_VIDEO_ENCODER_USE_HW_SURFACE.
      /// To completely avoid trying to set up HW accelerated encoding,
      /// set GZ_VIDEO_ALLOWED_ENCODERS to value NONE or leave it empty or
      /// unset.
      /// The meaning of these variables is the following:
      /// - GZ_VIDEO_ALLOWED_ENCODERS is a colon-separated list of values of
      /// HWEncoderType enum, or ALL to allow all encoders. Default is NONE.
      /// - GZ_VIDEO_ENCODER_DEVICE optionally specifies the HW device
      /// to use for encoding (used only when a matching encoder is found
      /// first). By default, an empty string is used, which means to use
      /// whatever device is found to work first.
      /// - GZ_VIDEO_USE_HW_SURFACE specifies whether the encoder should use
      /// an explicit GPU buffer for video frames. Some codecs do this
      /// implicitly, and then this setting has no meaning (setting it to 1 can
      /// actually decrease performance). For codecs that need to set this
      /// explicitly, this variable is checked for three values: 1, 0 and empty.
      /// Empty value means that the implementation should guess whether
      /// explicit GPU buffers are worth it or not.
      public: bool Start(
                const std::string &_format = VIDEO_ENCODER_FORMAT_DEFAULT,
                const std::string &_filename = "",
                const unsigned int _width = VIDEO_ENCODER_WIDTH_DEFAULT,
                const unsigned int _height = VIDEO_ENCODER_HEIGHT_DEFAULT,
                const unsigned int _fps = VIDEO_ENCODER_FPS_DEFAULT,
                const unsigned int _bitRate = VIDEO_ENCODER_BITRATE_DEFAULT);

      /// \brief Start the encoder. This should be called once. Add new
      /// frames to the video using the AddFrame function. Use SaveToFile
      /// when the video is complete.
      /// \param[in] _width Width in pixels of the output video.
      /// \param[in] _height Height in pixels of the output video.
      /// \param[in] _format String that represents the video type.
      /// Supported types include: "avi", "ogv", mp4", "v4l2". If using
      /// "v4l2", you must also specify a _filename.
      /// \param[in] _bitRate Bit rate to encode the video. A value of zero
      /// will cause this function to automatically compute a bitrate.
      /// \param[in] _filename Name of the file that stores the video while it
      /// is being created. This is a temporary file when recording to
      /// disk, or a video4linux loopback device like /dev/video0 when
      /// the _format is "v4l2". If blank, a default temporary file is used.
      /// However, the "v4l2" _format must be accompanied with a video
      /// loopback device filename.
      /// \param[in] _allowHwAccel If true, HW acceleration settings are read
      /// from environment variables (see the other Start() overload for the
      /// loading mechanism description). If set to false, only SW encoding will
      /// be done.
      /// \return True on success. Do NOT ignore the return value. If Start()
      /// failed, encoding will not work. If Start() function failed and you ran
      /// it with allowed HW acceleration, you can try running it once more
      /// without it (this can help in case the HW encoder can be found and
      /// detected, but it fails to start, e.g. when NVENC sessions are
      /// exhausted).
      public: bool Start(
                const std::string &_format,
                const std::string &_filename,
                const unsigned int _width,
                const unsigned int _height,
                const unsigned int _fps,
                const unsigned int _bitRate,
                const bool _allowHwAccel);

      /// \brief Start the encoder. This should be called once. Add new
      /// frames to the video using the AddFrame function. Use SaveToFile
      /// when the video is complete.
      /// \param[in] _width Width in pixels of the output video.
      /// \param[in] _height Height in pixels of the output video.
      /// \param[in] _format String that represents the video type.
      /// Supported types include: "avi", "ogv", mp4", "v4l2". If using
      /// "v4l2", you must also specify a _filename.
      /// \param[in] _bitRate Bit rate to encode the video. A value of zero
      /// will cause this function to automatically compute a bitrate.
      /// \param[in] _filename Name of the file that stores the video while it
      /// is being created. This is a temporary file when recording to
      /// disk, or a video4linux loopback device like /dev/video0 when
      /// the _format is "v4l2". If blank, a default temporary file is used.
      /// However, the "v4l2" _format must be accompanied with a video
      /// loopback device filename.
      /// \param[in] _allowedHwAccel Allowed HW acceleration frameworks to
      /// probe (as a bitmask of values of HWAccelerationDevice enum).
      /// Set to HWAccelerationDevice::NONE to force software encoding only.
      /// \param[in] _hwAccelDevice If nonempty, specifies the HW device to use
      /// for encoding. If empty, the framework search method will look for some
      /// default devices.
      /// \return True on success. Do NOT ignore the return value. If Start()
      /// failed, encoding will not work. If Start() function failed and you ran
      /// it with some HW acceleration, you can try running it once more
      /// with a different acceleration config (this can help in case the HW
      /// encoder can be found and detected, but it fails to start, e.g. when
      /// NVENC sessions are exhausted).
      public: bool Start(
                const std::string &_format,
                const std::string &_filename,
                unsigned int _width,
                unsigned int _height,
                unsigned int _fps,
                unsigned int _bitRate,
                const FlagSet<HWEncoderType>& _allowedHwAccel,
                const std::string& _hwAccelDevice = "",
                std::optional<bool> _useHwSurface = {});

      /// \brief Stop the encoder. The SaveToFile function also calls this
      /// function.
      /// \return True on success.
      public: bool Stop();

      /// \brief True if the enoder has been started, false otherwise
      /// \return True if Start has been called.
      public: bool IsEncoding() const;

      /// \brief Add a single frame to be encoded
      /// \param[in] _frame Image buffer to be encoded
      /// \param[in] _width Input frame width
      /// \param[in] _height Input frame height
      /// \return True on success
      public: bool AddFrame(const unsigned char *_frame,
                            const unsigned int _width,
                            const unsigned int _height);

      /// \brief Add a single timestamped frame to be encoded
      /// \param[in] _frame Image buffer to be encoded
      /// \param[in] _width Input frame width
      /// \param[in] _height Input frame height
      /// \param[in] _timestamp Timestamp of the image frame
      /// \return True on success.
      public: bool AddFrame(const unsigned char *_frame,
                  const unsigned int _width,
                  const unsigned int _height,
                  const std::chrono::steady_clock::time_point &_timestamp);

      /// \brief Write the video to disk
      /// param[in] _filename File in which to save the encoded data
      /// \return True on success.
      public: bool SaveToFile(const std::string &_filename);

      /// \brief Get the encoding format
      /// \return Encoding format
      public: std::string Format() const;

      /// \brief Get the bit rate
      /// \return Bit rate
      public: unsigned int BitRate() const;

      /// \brief Reset to default video properties and clean up allocated
      /// memory. This will also delete any temporary files.
      public: void Reset();

      /// \brief Private data pointer
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
  }
}
#endif
