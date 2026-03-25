/*
 * Copyright (C) 2020 Martin Pecka
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
*/
#ifndef GZ_COMMON_HWENCODER_HH
#define GZ_COMMON_HWENCODER_HH

#include <memory>
#include <string>
#include <optional>

#include <gz/common/FlagSet.hh>
#include <gz/common/av/Export.hh>
#include <gz/common/HWVideo.hh>
#include <gz/common/ffmpeg_inc.hh>
#include <gz/utils/ImplPtr.hh>

/// This is an internal-use only class encapsulating HW video encoding. All
/// symbols defined here are hidden from the public API.
namespace gz::common
{
  /// \brief Representation of a GPU video encoder and its configuration.
  /// \note This class is intentionally hidden as it provides highly customized
  /// behavior tailored just for the use with VideoEncoder.
  class GZ_COMMON_AV_HIDDEN HWEncoder
  {
    /// \brief Set up the HW encoder configurator.
    /// \param[in] _allowedHwEncoders HW encoders to try.
    /// \param[in] _hwAccelDevice Optional name of the device that should be
    /// used. It doesn't affect unrelated HW encoders, but if the name is a
    /// match for an encoder, it will limit its search to only this device.
    /// However, if another encoder is tried earlier and succeeds, it will
    /// return with its default device.
    /// \param[in] _useHwSurface. If set, specifies whether the HW encoder
    /// should explicitly upload frames to the GPU, or whether that is left for
    /// the internals of the codec. If the value is unset, a HW-specific
    /// default is used that was experimentally found to be faster.
    public: HWEncoder(FlagSet<HWEncoderType> _allowedHwEncoders,
                      const std::string& _hwAccelDevice,
                      std::optional<bool> _useHwSurface);
    public: virtual ~HWEncoder();

    /// Find a suitable encoder, preferring HW encoders.
    /// \param[in] _codecId ID of the codec we seek the encoder for.
    /// \return Name of the encoder to be used.
    /// \note The class expects that the returned encoder will be passed to
    /// ConfigHWAccel, and no other.
    public: AVCodec* FindEncoder(AVCodecID _codecId);

    /// \brief Configure hardware acceleration. Should be called after the codec
    /// context parameters have been set, but before calling avcodec_open2().
    /// \param[in] _encoderContext Context of the encoder for which hardware
    /// acceleration should be set up.
    /// \note If the configuration fails, the codec will be left configured for
    /// software encoding and an error will be written to gzerr describing what
    /// failed.
    public: void ConfigHWAccel(AVCodecContext* _encoderContext);

    /// \brief Get a pointer to the frame that contains the encoder input. This
    /// mainly serves for uploading the frame to GPU buffer if requested.
    /// \param[in] _inFrame The input frame to be encoded (in CPU buffer).
    /// \return Pointer to either this->avOutFrame or this->avOutHwFrame.
    /// \note If GPU buffers are used, this function uploads the frame from the
    /// CPU buffer to the GPU before returning.
    public: AVFrame* GetFrameForEncoder(AVFrame* _inFrame);

    /// \brief Returns the type of encoder that is currently being used.
    /// The value will be NONE until ConfigHWAccel() successfully finishes.
    public: HWEncoderType GetEncoderType() const;

    /// \brief Private data pointer
    GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
  };
}

#endif
