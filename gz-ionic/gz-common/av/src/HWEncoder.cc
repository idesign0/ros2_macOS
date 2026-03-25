/*
 * Copyright 2020 Martin Pecka
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
 * The hardware acceleration code is inspired by
 * https://stackoverflow.com/questions/59666753
*/

#include <map>
#include <utility>

#include "gz/common/ffmpeg_inc.hh"
#include "gz/common/HWEncoder.hh"
#include "gz/common/StringUtils.hh"
#include "gz/common/Console.hh"

using namespace gz;
using namespace common;
using namespace std;

class common::HWEncoder::Implementation
{
  /// \brief Device reference for HW-accelerated encoding.
  public: AVBufferRef* hwDevice = nullptr;

  /// \brief Name of the device the encoder is using (empty means default).
  public: std::string hwDeviceName;

  /// \brief Type of the HW encoder in use.
  public: HWEncoderType hwEncoderType = HWEncoderType::NONE;

  /// \brief libav output HW video frame.
  public: AVFrame *avOutHwFrame = nullptr;

  /// \brief Specifies whether the HW encoder should explicitly upload frames to
  /// the GPU, or whether that is left for the internals of the codec.
  public: bool useHwSurface = false;

  /// \brief The set of HW encoders that were given to the constructor.
  public: FlagSet<HWEncoderType> initHwEncoders =
    FlagSet<HWEncoderType>::AllSet();

  /// \brief The HW device that was given to the constructor.
  public: std::string initHwDevice;

  /// \brief The hint for using HW surfaces that were given to the constructor.
  public: std::optional<bool> initUseHwSurface = std::nullopt;

  /// \brief Try to configure a hardware pixel surface for the given context.
  /// \param[in] _encoderContext The encoder context to use. It has to already
  /// have hw_device_ctx set and configured.
  /// \return True in case the surface was correctly set and initialized.
  /// \note This function assumes that _encoderContext->pix_fmt is a hardware
  /// pixel format and that _encoderContext->sw_pix_fmt is the pixel format
  /// of the CPU (software) input buffer.
  public: bool ConfigHWSurface(AVCodecContext* _encoderContext);
};

// the bool in key means whether the string is prefix (true) or suffix (false)
const map<pair<string, bool>, HWEncoderType> HW_ENCODER_MATCHERS = {
    {make_pair("_cuda", false), HWEncoderType::NVENC},
    {make_pair("nvenc", true), HWEncoderType::NVENC},
    {make_pair("_nvenc", false), HWEncoderType::NVENC},
    {make_pair("_cuvid", false), HWEncoderType::NVENC},
    {make_pair("_vaapi", false), HWEncoderType::VAAPI},
    {make_pair("_vdpau", false), HWEncoderType::VDPAU},
    {make_pair("_amf", false), HWEncoderType::AMF},
    {make_pair("_videotoolbox", false), HWEncoderType::VIDEOTOOLBOX},
    {make_pair("_qsv", false), HWEncoderType::QSV},
    {make_pair("_omx", false), HWEncoderType::OMX},
    {make_pair("_v4l2m2m", false), HWEncoderType::V4L2M2M},
    {make_pair("dxva2_", true), HWEncoderType::DXVA2},
};

/////////////////////////////////////////////////
/// Detect the type of HW encoder the given codec uses.
/// \param[in] _codecName The codec to examine.
/// \return The HW encoder type, or NONE for software encoders.
HWEncoderType DetectHWEncoderType(const std::string& _codecName)
{
  for (const auto& accel : HW_ENCODER_MATCHERS)
  {
    const auto matchPrefix = accel.first.second;
    const auto& matchString = accel.first.first;
    const auto hwAccel = accel.second;

    bool matches;
    if (matchPrefix)
      matches = StartsWith(_codecName, matchString);
    else
      matches = EndsWith(_codecName, matchString);

    if (matches)
      return hwAccel;
  }
  return HWEncoderType::NONE;
}

/////////////////////////////////////////////////
/// \brief Try if the given device can be opened as the given HW encoder type.
/// \param _deviceName File name of the device, if it has any, or a pretty name.
/// \param _deviceType Libav type of the acceleration device to try.
/// \param _encoderType Type of HW encoder to try (just for pretty-printing).
/// \param _checkDeviceFileExistence If true, check whether _deviceName is a
/// valid file before trying to open the encoder.
/// \param _warnIfNotFile Print a warning if _deviceName is not a file.
/// \param _libavDeviceName The device name to be passed to
/// av_hwdevice_ctx_create(). Usually will be the same as _deviceName.
/// \param _opts An optional dictionary of options to pass to
/// av_hwdevice_ctx_create().
/// \return True if the parameters specify a HW encoder for which a device can
/// be initialized.
/// \note This function does not keep the encoder context open, you have to
/// open and initialize it yourself.
bool ProbeDevice(const std::string& _deviceName,
                 const AVHWDeviceType _deviceType,
                 const HWEncoderType _encoderType,
                 const bool _checkDeviceFileExistence,
                 const bool _warnIfNotFile,
                 const std::string& _libavDeviceName,
                 AVDictionary* _opts = nullptr)
{
  if (_checkDeviceFileExistence && !_deviceName.empty())
  {
    auto* device = fopen(_deviceName.c_str(), "r+");
    if (device != nullptr)
    {
      // device exists, we can proceed
      fclose(device);
    }
    else
    {
      if (_warnIfNotFile)
        gzwarn << "Device " << _deviceName << " can't be used with "
                << HWEncoderTypeParser.Str(_encoderType) <<" (aren't write "
                << "permissions for the device missing?)" << std::endl;
      return false;
    }
  }

  const auto deviceStr =
    _deviceName.empty() ? "default device" : "device " + _deviceName;

  AVBufferRef* tmpHwDevice;
  // we own tmpHwDevice, so it needs to be unreferenced later
  const auto ret = av_hwdevice_ctx_create(  // lavu 55.13.0
    &tmpHwDevice, _deviceType, _libavDeviceName.c_str(), _opts, 0);

  if (ret < 0)
  {
    gzerr << "Could not initialize " << deviceStr << " for "
           << HWEncoderTypeParser.Str(_encoderType) << ", skipping.\n";
    return false;
  }

  av_buffer_unref(&tmpHwDevice);

  gzmsg << "Initialized " << HWEncoderTypeParser.Str(_encoderType)
         << " on " << deviceStr << std::endl;

  return true;
}

/////////////////////////////////////////////////
bool ProbeDevice(const std::string& _deviceName,
                 const AVHWDeviceType _deviceType,
                 const HWEncoderType _encoderType,
                 const bool _checkDeviceFileExistence,
                 const bool _warnIfNoMatch = false)
{
  return ProbeDevice(
    _deviceName, _deviceType, _encoderType, _checkDeviceFileExistence,
    _warnIfNoMatch, _deviceName, nullptr);
}

/////////////////////////////////////////////////
AVCodec* HWEncoder::FindEncoder(AVCodecID _codecId)
{
  AVCodec* foundEncoder = nullptr;
  HWEncoderType foundEncoderType = HWEncoderType::NONE;
  std::string foundDevice;

  const AVCodec* codec = nullptr;  // av_codec_iterate needs const
#if LIBAVFORMAT_VERSION_INT > AV_VERSION_INT(58, 10, 100)
  void* iter = nullptr;
  // since lavf 58.10.100
  while ((codec = av_codec_iterate(&iter)) && foundEncoder == nullptr)
  {
#else
  // until lavf 58.8.100
  while ((codec = av_codec_next(codec)) && foundEncoder == nullptr)
  {
#endif
    if (codec->id != _codecId || !av_codec_is_encoder(codec))  // lavc 54.8.100
      continue;

    const auto hwEncoderType = DetectHWEncoderType(codec->name);
    if (hwEncoderType == HWEncoderType::NONE)
    {
      // if we only search for software encoders, happily return the first one
      if (this->dataPtr->initHwEncoders == HWEncoderType::NONE)
      {
        gzmsg << "Compatible SW encoder: " << codec->name << std::endl;
        foundEncoder = const_cast<AVCodec*>(codec);
        break;
      }
      // if we're searching for hw encoders, skip the sw ones, and run a
      // second recursive search for sw-only encoders in case we do not find
      // any hw ones
    }
    else
    {
      gzmsg << "Found known HW encoder: " << codec->name << std::endl;

      if (!this->dataPtr->initHwEncoders[hwEncoderType])
      {
        gzmsg << "Encoder " << codec->name << " is not allowed, skipping.\n";
        continue;
      }

      const auto& initHwDevice = this->dataPtr->initHwDevice;

      switch (hwEncoderType)
      {
        case HWEncoderType::NVENC:
        {
          std::string deviceName = "/dev/nvidia0";
          if (StartsWith(initHwDevice, "/dev/nvidia"))
            deviceName = initHwDevice;
          else if (!initHwDevice.empty())
            break;

          const auto checkFileExistence =
#if __linux__
            true;
#else
          // on Windows, there is no file /dev/nvidia*
          false;
#endif

          const char devNum =
            (deviceName.empty() ? '0' : deviceName[deviceName.length() - 1]);
          if (ProbeDevice(deviceName,
                          AV_HWDEVICE_TYPE_CUDA,  // lavu 55.6.0
                          hwEncoderType, checkFileExistence,
                          !initHwDevice.empty(), &devNum))
          {
            foundEncoder = const_cast<AVCodec*>(codec);
            foundEncoderType = hwEncoderType;
            foundDevice = devNum;
          }

          break;
        }

#ifdef __linux__
        case HWEncoderType::VAAPI:
        {
          // 129 tends to be the discrete GPU, so prefer it
          std::vector<std::string> devices = {
            "/dev/dri/renderD129", "/dev/dri/renderD128", ":0"
          };
          std::string display;
          if (common::env("DISPLAY", display) && display != ":0")
            devices.push_back(display);

          // DRM device or local X display
          if (StartsWith(initHwDevice, "/dev/dri/renderD") ||
              StartsWith(initHwDevice, ":"))
          {
            devices = {initHwDevice};
          }
          else if (!initHwDevice.empty())
          {
            const auto parts = Split(initHwDevice, ':');
            // check if device is of form <host>:<display>
            if (parts.size() != 2)
              break;
            // check that <display> is of form <number>[.<sub>]
            if (parts[1].find_first_not_of("0123456789.") != std::string::npos)
              break;
            devices = {initHwDevice};
          }

          for (const auto& deviceName : devices)
          {
            if (ProbeDevice(deviceName, AV_HWDEVICE_TYPE_VAAPI,  // lavu 55.9.0
                            HWEncoderType::VAAPI,
                            !deviceName.empty() && deviceName[0] == '/',
                            !initHwDevice.empty()))
            {
              foundEncoder = const_cast<AVCodec*>(codec);
              foundEncoderType = hwEncoderType;
              foundDevice = deviceName;
              break;
            }
          }
          break;
        }
#endif

        case HWEncoderType::QSV:
        {
#ifdef __linux__
          // on Linux, QSV accesses the GPU via VAAPI, so we actually need to
          // check whether VAAPI support works; if it does and QSV is compiled
          // in, it should work, too
          const auto origInitHwEncoders = this->dataPtr->initHwEncoders;
          this->dataPtr->initHwEncoders = HWEncoderType::VAAPI;
          const auto vaapiEncoder = FindEncoder(_codecId);
          this->dataPtr->initHwEncoders = origInitHwEncoders;
          if (vaapiEncoder != nullptr &&
            this->dataPtr->hwEncoderType == HWEncoderType::VAAPI)
          {
            foundEncoder = const_cast<AVCodec*>(codec);
            foundEncoderType = HWEncoderType::QSV;
            foundDevice = this->dataPtr->hwDeviceName;
          }
          break;
#elif defined(_WIN32)
          // on Windows, GPUs are named just by their number (in what order?),
          // so valid devices are 0-based GPU indices (passed in text form)
          if (initHwDevice.length() > 1)
            break;

          // Try to parse as number; we cannot use the return value of
          // strtol because it returns the same for parse error and for
          // parsing "0"; however, endptr should be set to the last parsed
          // position, so when nothing parsed, it would point to the
          // start of _initHwDevice.
          // The use of strtol here is intentional - it mimics the behavior
          // of h264_qsv codec. Do NOT replace it with other number parsing
          // methods.
          char* endptr;
          strtol(initHwDevice.c_str(), &endptr, 0);
          if (!initHwDevice.empty() && endptr == initHwDevice.c_str())
            break;

          // QSV uses child devices to do the actual work, so we need to pass
          // the device name in the options
          AVDictionary* opts = nullptr;
          av_dict_set(&opts, "child_device", initHwDevice.c_str(), 0);
          // from libavutil/hwcontext_qsv.c, selects MFX_IMPL_HARDWARE_ANY
          const auto qsvMfxImpl = "hw_any";

          if (ProbeDevice(initHwDevice,
                          AV_HWDEVICE_TYPE_QSV,  // lavu 55.32.100
                          HWEncoderType::QSV, false, !initHwDevice.empty(),
                          qsvMfxImpl, opts))
          {
            foundEncoder = const_cast<AVCodec*>(codec);
            foundEncoderType = hwEncoderType;
            foundDevice = initHwDevice;
          }
          av_dict_free(&opts);
          break;
#else
          // QSV is only supported on Linux and Windows
          break;
#endif
        }

        default:
        {
          // This is an encoder that is compiled into libavcodec, but we do
          // not (yet) support it.
          gzmsg << "Encoder " << codec->name << " is not yet supported.\n";
          break;
        }
      }
    }
  }

  // if no hardware-accelerated encoder has been found, return SW encoder
  if (foundEncoder == nullptr &&
    this->dataPtr->initHwEncoders != HWEncoderType::NONE)
  {
    gzwarn << "No hardware-accelerated encoder found, falling back to "
               "software encoders" << std::endl;
    this->dataPtr->initHwEncoders = HWEncoderType::NONE;
    return this->FindEncoder(_codecId);
  }

  this->dataPtr->hwEncoderType = foundEncoderType;
  this->dataPtr->hwDeviceName = foundDevice;
  return foundEncoder;
}

// These functions are used by _encoderContext when using HW surfaces. It
// would be really nice to pass a lambda using GetFormat() to _encoderContext,
// however it is not possible to get lambdas with C linkage.
extern "C" {

/////////////////////////////////////////////////
static enum AVPixelFormat GetFormat(AVCodecContext*,
  const enum AVPixelFormat* _formats, const enum AVPixelFormat _format)
{
  const enum AVPixelFormat* p;
  for (p = _formats; *p != AV_PIX_FMT_NONE; p++) {
    if (*p == _format)
      return *p;
  }
  fprintf(stderr, "Failed to get HW surface format.\n");
  return AV_PIX_FMT_NONE;
}

/////////////////////////////////////////////////
static enum AVPixelFormat GetFormatVAAPI(
    AVCodecContext* _codec, const enum AVPixelFormat *_formats)
{
  return GetFormat(_codec, _formats, AV_PIX_FMT_VAAPI);  // lavu54.31.100
}

/////////////////////////////////////////////////
static enum AVPixelFormat GetFormatQSV(
    AVCodecContext* _codec, const enum AVPixelFormat *_formats)
{
  return GetFormat(_codec, _formats, AV_PIX_FMT_QSV);  // lavu 54.19.100
}

/////////////////////////////////////////////////
static enum AVPixelFormat GetFormatCUDA(
    AVCodecContext* _codec, const enum AVPixelFormat *_formats)
{
  return GetFormat(_codec, _formats, AV_PIX_FMT_CUDA);  // lavu 55.6.0
}

}

/////////////////////////////////////////////////
void HWEncoder::ConfigHWAccel(AVCodecContext* _encoderContext)
{
  int ret;  // used for libav return codes

  const auto& fallbackGetFormat = _encoderContext->get_format;
  AVPixelFormat fallbackPixelFormat = _encoderContext->pix_fmt;

  switch (this->dataPtr->hwEncoderType)
  {
    case HWEncoderType::NVENC:
    {
      ret = av_hwdevice_ctx_create(  // lavu 55.13.0
        &this->dataPtr->hwDevice, AV_HWDEVICE_TYPE_CUDA,  // lavu 55.6.0
        this->dataPtr->hwDeviceName.c_str(), nullptr, 0);

      if (ret < 0)
      {
        gzerr << "Could not initialize HW encoding device using "
               << HWEncoderTypeParser.Str(this->dataPtr->hwEncoderType) << ". "
               << av_err2str_cpp(ret)
               << ". Video encoding will use a software encoder instead.\n";
        return;
      }

      // set to variable bitrate so that the configured bitrate is achieved
      // lavu 51.21.0
      ret = av_opt_set(_encoderContext->priv_data, "rc", "vbr",
                       AV_OPT_SEARCH_CHILDREN);
      if (ret == AVERROR_OPTION_NOT_FOUND)
        gzerr << "Codec " << _encoderContext->codec->name << " does not "
               << "support option 'rc' which is required for setting bitrate "
               << "of the encoded video. The video will have some default "
               << "bitrate.\n";
      else if (ret == AVERROR(EINVAL))
        gzerr << "Codec " << _encoderContext->codec->name << " does not "
               << "support 'vbr' mode for option 'rc' which is required for "
               << "setting bitrate of the encoded video. The video will have "
               << "some default bitrate.\n";

      // default to not using HW surface
      this->dataPtr->useHwSurface = this->dataPtr->initUseHwSurface.has_value()
        && *this->dataPtr->initUseHwSurface;

      // could also use NV12, but YUV yields better performance
      // we misuse this field a bit, as docs say it is unused in encoders
      _encoderContext->sw_pix_fmt = AV_PIX_FMT_YUV420P;  // lavu 51.42.0
      if (this->dataPtr->useHwSurface)
      {
        _encoderContext->pix_fmt = AV_PIX_FMT_CUDA;  // lavu 55.6.0
        _encoderContext->get_format = GetFormatCUDA;
      }

      break;
    }

#ifdef __linux__
    case HWEncoderType::VAAPI:
    {
      ret = av_hwdevice_ctx_create(  // lavu 55.13.0
        &this->dataPtr->hwDevice, AV_HWDEVICE_TYPE_VAAPI,  // lavu 55.9.0
        this->dataPtr->hwDeviceName.c_str(), nullptr, 0);
      if (ret < 0)
      {
        gzerr << "Could not initialize HW encoding device using "
               << HWEncoderTypeParser.Str(this->dataPtr->hwEncoderType) << ". "
               << av_err2str_cpp(ret)
               << ". Video encoding will use a software encoder instead.\n";
        return;
      }

      // default to using HW surface
      this->dataPtr->useHwSurface = !this->dataPtr->initUseHwSurface.has_value()
        || *this->dataPtr->initUseHwSurface;

      // tutorials suggest NV12, but we have RGB24 input and this shows to be
      // the fastest format it can be converted into
      // we misuse this field a bit, as docs say it is unused in encoders
      _encoderContext->sw_pix_fmt = AV_PIX_FMT_RGB0;  // // lavu 51.42.0
      // lavu 51.42.0
      _encoderContext->pix_fmt = fallbackPixelFormat = AV_PIX_FMT_YUV420P;
      if (this->dataPtr->useHwSurface)
      {
        _encoderContext->pix_fmt = AV_PIX_FMT_VAAPI;  // lavu 54.31.100
        _encoderContext->get_format = GetFormatVAAPI;
      }

      break;
    }
#endif

    case HWEncoderType::QSV:
    {
      // QSV uses child devices to do the actual work - VAAPI on Linux,
      // DXVA2 on Windows; so we need to pass the device name in the options
      AVDictionary* opts = nullptr;
      av_dict_set(&opts, "child_device",
                  this->dataPtr->hwDeviceName.c_str(), 0);
      // from libavutil/hwcontext_qsv.c, selects MFX_IMPL_HARDWARE_ANY
      const auto qsvMfxImpl = "hw_any";

      ret = av_hwdevice_ctx_create(  // lavu 55.13.0
        &this->dataPtr->hwDevice, AV_HWDEVICE_TYPE_QSV,  // lavu 55.32.100
        qsvMfxImpl, opts, 0);
      av_dict_free(&opts);

      if (ret < 0)
      {
        gzerr << "Could not initialize HW encoding device using "
               << HWEncoderTypeParser.Str(this->dataPtr->hwEncoderType) << ". "
               << av_err2str_cpp(ret)
               << ". Video encoding will use a software encoder instead.\n";
        return;
      }

      // default to not using HW surface
      this->dataPtr->useHwSurface = this->dataPtr->initUseHwSurface.has_value()
        && *this->dataPtr->initUseHwSurface;

      // we misuse this field a bit, as docs say it is unused in encoders
      _encoderContext->sw_pix_fmt = AV_PIX_FMT_NV12;  // lavu 51.42.0
      _encoderContext->pix_fmt = fallbackPixelFormat = AV_PIX_FMT_NV12;
      if (this->dataPtr->useHwSurface)
      {
        _encoderContext->pix_fmt = AV_PIX_FMT_QSV;  // lavu 54.19.100
        _encoderContext->get_format = GetFormatQSV;
      }

      break;
    }

    default:
      return;
  }

  // this should not happen, but to be sure
  if (this->dataPtr->hwDevice == nullptr)
    return;

  // the reference is freed by libavcodec
  _encoderContext->hw_device_ctx = av_buffer_ref(this->dataPtr->hwDevice);

  if (this->dataPtr->useHwSurface)
  {
    const auto success = this->dataPtr->ConfigHWSurface(_encoderContext);
    if (!success)
    {
      // if HW surface initialization failed, fall back to software surface
      this->dataPtr->useHwSurface = false;
      _encoderContext->pix_fmt = fallbackPixelFormat;
      _encoderContext->get_format = fallbackGetFormat;
    }
  }
}

/////////////////////////////////////////////////
bool HWEncoder::Implementation::ConfigHWSurface(AVCodecContext* _encoderContext)
{
  int ret;

  auto* hw_frames_ref = av_hwframe_ctx_alloc(this->hwDevice);  // lavu 55.6.0
  if (hw_frames_ref == nullptr)
  {
    gzerr << "Could not allocate hardware surface for format "
           << av_get_pix_fmt_name(_encoderContext->pix_fmt)  // lavu 51.3.0
           << " and HW encoder " << HWEncoderTypeParser.Str(this->hwEncoderType)
           << ". Encoder will use a software surface.\n";
    return false;
  }

  auto* frames_ctx = reinterpret_cast<AVHWFramesContext*>(hw_frames_ref->data);
  frames_ctx->format = _encoderContext->pix_fmt;
  frames_ctx->sw_format = _encoderContext->sw_pix_fmt;
  frames_ctx->width = _encoderContext->width;
  frames_ctx->height = _encoderContext->height;
  frames_ctx->initial_pool_size = 20;

  ret = av_hwframe_ctx_init(hw_frames_ref);  // lavu 55.6.0
  if (ret < 0)
  {
    av_buffer_unref(&hw_frames_ref);
    gzerr << "Could not initialize hardware surface for format "
           << av_get_pix_fmt_name(_encoderContext->pix_fmt)  // lavu 51.3.0
           << " and HW encoder "
           << HWEncoderTypeParser.Str(this->hwEncoderType)
           << ". Encoder will use a software surface. The reason is: "
           << av_err2str_cpp(ret) << endl;
    return false;
  }

  // the reference is freed by libavcodec
  _encoderContext->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
  av_buffer_unref(&hw_frames_ref);  // we no longer need our reference

  this->avOutHwFrame = av_frame_alloc();  // lavc 55.45.101
  if (this->avOutHwFrame == nullptr)
  {
    av_buffer_unref(&_encoderContext->hw_frames_ctx);

    gzerr << "Could not allocate frame for format "
           << av_get_pix_fmt_name(_encoderContext->pix_fmt)  // lavu 51.3.0
           << " and HW encoder " << HWEncoderTypeParser.Str(this->hwEncoderType)
           << ". Encoder will use a software surface.\n";
    return false;
  }

  ret = av_hwframe_get_buffer(_encoderContext->hw_frames_ctx,  // lavu 55.6.0
                              this->avOutHwFrame, 0);
  if (ret < 0)
  {
    av_frame_free(&this->avOutHwFrame);  // lavc 55.45.101
    av_buffer_unref(&_encoderContext->hw_frames_ctx);

    gzerr << "Could not initialize hardware frame for format "
           << av_get_pix_fmt_name(_encoderContext->pix_fmt)  // lavu 51.3.0
           << " and HW encoder " << HWEncoderTypeParser.Str(this->hwEncoderType)
           << ". Encoder will use a software surface. The reason is: "
           << av_err2str_cpp(ret) << std::endl;
    return false;
  }

  gzmsg << "Using HW surface for the encoder." << std::endl;
  return true;
}

/////////////////////////////////////////////////
AVFrame* HWEncoder::GetFrameForEncoder(AVFrame* _inFrame)
{
  auto result = _inFrame;
  if (this->dataPtr->useHwSurface && this->dataPtr->avOutHwFrame) {
    // lavu 55.6.0
    av_hwframe_transfer_data(this->dataPtr->avOutHwFrame, _inFrame, 0);
    result = this->dataPtr->avOutHwFrame;
  }
  return result;
}

/////////////////////////////////////////////////
HWEncoder::HWEncoder(const FlagSet<HWEncoderType> _allowedHwEncoders,
                     const std::string& _hwAccelDevice,
                     std::optional<bool> _useHwSurface)
  : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
  this->dataPtr->initHwEncoders = _allowedHwEncoders;
  this->dataPtr->initHwDevice = _hwAccelDevice;
  this->dataPtr->initUseHwSurface = _useHwSurface;
}

/////////////////////////////////////////////////
HWEncoder::~HWEncoder()
{
    this->dataPtr->initHwEncoders.Set();
    this->dataPtr->initHwDevice = "";
    this->dataPtr->initUseHwSurface.reset();

    if (this->dataPtr->hwDevice)
      av_buffer_unref(&this->dataPtr->hwDevice);
    if (this->dataPtr->avOutHwFrame)
      av_frame_free(&this->dataPtr->avOutHwFrame);  // lavc 55.45.101
}

/////////////////////////////////////////////////
HWEncoderType HWEncoder::GetEncoderType() const
{
  return this->dataPtr->hwEncoderType;
}
