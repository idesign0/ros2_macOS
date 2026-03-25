\page hw-encoding Hardware-accelerated Video Encoding

When recording video using the `gz::common::VideoEncoder` class, you can
opt-in to use hardware (HW) acceleration for the encoding process. By default, only software
encoders are used. This tutorial will show how to configure the encoder for HW
acceleration and will present ready-made commandlines for some typical use-cases.

You can either use the `VideoEncoder` class directly, or you can "meet it" in the
video recorder plugin in Gazebo. In both cases, HW-accelerated encoding
can be set up.

HW acceleration should provide you with higher encoding performance, potentially
leaving more CPU power to the rest of your program/simulation, while taking a bit of
GPU memory (video encoding uses different chips than 3D graphics or CUDA computations,
so performance-wise, the rest of the GPU should be unaffected).

# Configuring hardware acceleration

## What needs to be configured

In order to get HW accelerated encoding working, you need to get 3 things right:

1. The encoder type
2. The HW device to be used
3. Whether to use a specialized HW surface

## 1. Encoder types

The support for HW-accelerated encoding is based on what the local installation of
`libavcodec` (and your hardware) offers. If the libavcodec/FFMpeg your system has doesn't support HW
acceleration, you're out of luck until you get a version that supports some.
Some information about various aspects of the acceleration support by FFMpeg
can be found on their [HWAccelIntro wiki page](https://trac.ffmpeg.org/wiki/HWAccelIntro).

If FFMpeg is correctly installed, you can see the available HW encoders by calling
``ffmpeg -hide_banner -encoders | grep 264``.

Here are some sample outputs:

```
(Windows) $ ffmpeg -hide_banner -encoders | grep 264
 V..... libx264              libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264)
 V..... libx264rgb           libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 RGB (codec h264)
 V..... h264_amf             AMD AMF H.264 Encoder (codec h264)
 V..... h264_nvenc           NVIDIA NVENC H.264 encoder (codec h264)
 V..... h264_qsv             H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (Intel Quick Sync Video acceleration) (codec h264)
 V..... nvenc                NVIDIA NVENC H.264 encoder (codec h264)
 V..... nvenc_h264           NVIDIA NVENC H.264 encoder (codec h264)
```

```
(Linux) $ ffmpeg -hide_banner -encoders | grep 264
 V..... libx264              libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264)
 V..... libx264rgb           libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 RGB (codec h264)
 V..... h264_nvenc           NVIDIA NVENC H.264 encoder (codec h264)
 V..... h264_omx             OpenMAX IL H.264 video encoder (codec h264)
 V..... h264_v4l2m2m         V4L2 mem2mem H.264 encoder wrapper (codec h264)
 V..... h264_vaapi           H.264/AVC (VAAPI) (codec h264)
 V..... nvenc                NVIDIA NVENC H.264 encoder (codec h264)
 V..... nvenc_h264           NVIDIA NVENC H.264 encoder (codec h264)
```

Gazebo Common so far supports:

| Encoder | Technology | Platforms | GPU vendors | Gazebo Common name |
| ------- | ---------- | --------- | ----------- | -------------------- |
| `h264_nvenc` | [NVidia NVEnc](https://en.wikipedia.org/wiki/Nvidia_NVENC) | Windows + x86 Linux | NVidia GPUs | NVENC |
| `h264_qsv` | [Intel QuickSync](https://en.wikipedia.org/wiki/Intel_Quick_Sync_Video) | Windows + x86 Linux | Intel GPUs | QSV |
| `h264_vaapi` | [VA-API](https://en.wikipedia.org/wiki/Video_Acceleration_API) | x86 Linux | most GPUs | VAAPI |

Adding support for more is possible and should not be a big problem, but needs
someone with the right hardware to verify the implementation.

The acceleration libraries may also need some drivers for them to work:

- NVEnc: `libnvidia-encode-*` or just CUDA runtime
- VA-API: `libva-glx2`, `libva-drm2`, maybe also `libglvnd0`
- QSV: Unclear, usually works out of the box

## 2. Device names

If your computer has more GPUs, it is important to specify which one to use.
Here are some basic naming rules:

- NVEnc
  - Linux: Devices `/dev/nvidia0`, `/dev/nvidia1` etc. Beware that the numbers
    in names of these devices can differ from the numbers CUDA assigns the GPUs
    (CUDA orders the GPUs according to their compute capability, whereas numbering
    of these devices is probably by the order on PCI bus).
  - Windows: Devices are called the same as on Linux, even though such files do
    not exist in the system.
- QSV
  - Windows: The devices are called just `0`, `1` and so on.
  - Linux: When using QSV on Linux, you can use VA-API device names.
- VA-API
  - Linux:
    - DRM: The devices are called `/dev/dri/renderD128`, `/dev/dri/renderD129`
      etc. DRM stands for Direct Rendering Manager, not Digital Rights Management
      To use these devices, make sure your user has write permissions to the
      device files. Ubuntu usually doesn't give write access to everybody, but just
      to members of the `video` group.
    - GLX: You can also pass an X-server string like `:0`, which means the encoder
      should use the GPU on which this X server is running (it needs to support
      the GLX extension). On headless machines, you should use DRM.

## 3. Using HW surface

The last thing you need to decide is whether the selected encoder should use
a HW surface (pixel buffer) or the default CPU-located one. With most encoders,
this is just a performance issue and they will work both with CPU and GPU surfaces.

It is best if you perform experiments with HW surface used and not used and compare
the performance. Select the one that suits your use case better.

## Putting it together - configuration via environment variables

To ease configuration of the HW-accelerated encoding, there doesn't have to
be explicit support for it in the code using `VideoEncoder`. The code may concentrate
on implementing the recording procedure itself, and completely ignore any HW
acceleration of the recording process. Users of the code can then enable the HW
acceleration just using these 3 environment variables:

### GZ_VIDEO_ALLOWED_ENCODERS

This is the main variable that allows the `VideoEncoder` to probe for supported
HW-accelerated encoders. It is a colon-separated list of names described in the
table above. Example: `GZ_VIDEO_ALLOWED_ENCODERS=NVENC:QSV`. Special value `ALL`
means that all encoders should be tried. Special value `NONE` (or empty value) means
that a SW encoder should be used.

If more values are specified, the system will probe all the allowed encoders trying
to start them up (if device is specified, then only with the given device). The
first allowed encoder that successfully finishes the probe will be used.

The probing mechanism isn't 100% reliable. But it does what's reasonable to do in
such an autodetection loop - it checks whether the required/supported device files
exist, and if they do (or if there are no device files, as on Windows), the library
tries to create an encoding context. If the context is successfully created, the
encoder is considered working. Sometimes, something can go wrong in a later stage
(e.g. insufficient GPU memory), and that is a kind of thing you have to
handle yourself.

### GZ_VIDEO_ENCODER_DEVICE

This is a name of the encoder device as specified in the "Device Names" section.
If empty, first working device will be used. This auto detection should suffice on
single-GPU systems or if you don't care which GPU will be used. If a device is
specified, only encoders accepting this device name as an argument will be probed.

### GZ_VIDEO_USE_HW_SURFACE

This variable has three possible values:

- `1`: Explicitly tell the encoder to use a GPU-located buffer.
- `0`: Explicitly tell the encoder to use a CPU-located buffer.
- empty: Let the library guess based on some pre-compiled hints.

Refer to section "Using HW surface" for more in-depth description of the
meaning of this variable. Usually, leaving it empty should be just fine.

## Configuration in code

These values can also be configured in code.

As stated earlier, if you use the standard 6-argument signature of
`VideoEncoder::Start()`, configuration via the above described environment
variables will be performed.

There are two more signatures:

```{.cpp}
public: bool Start(_format, _filename, _width, _height, _fps, _bitRate,
                   bool _allowHwAccel);
```

This signature added the `_allowHwAccel` boolean argument with which you can
explicitly allow/disallow the configuration via environment variables.

Then there is the full signature:

```{.cpp}
public: bool Start(_format, _filename, _width, _height, _fps, _bitRate,
                   const FlagSet<HWEncoderType>& _allowedHwAccel,
                   const std::string& _hwAccelDevice = "",
                   std::optional<bool> _useHwSurface = {})
```

The three added arguments correspond to the environment variables, but with this
signature you can set their values from code (and environment variables will have
no effect then). This would be useful if you want to e.g. implement a GUI chooser
for the acceleration.

The `FlagSet<HWEncoderType>` captures a set of allowed encoders. Its value may be
e.g. `gz::Common::HWEncoderType::QSV | gz::common::HWEncoderType::NVENC`.

## How do I know it's working

To make sure you configured the HW acceleration right, you may look at info-level
messages where `VideoEncoder` documents the detected encoder. It may look like:

```
[Msg] Recording started: 3078s (sim time), 42088s (real time)
[GUI] [Msg] Recording video using sim time.
[GUI] [Msg] Recording video in lockstep mode
[GUI] [Msg] Recording video using bitrate: 8000000
[GUI] [Msg] Found known HW encoder: h264_nvenc
[GUI] [Msg] Initialized NVENC on device /dev/nvidia1
[GUI] [Msg] Using encoder h264_nvenc
[Msg] Recording stopped: 120s (sim time), 42413s (real time)
```

There you can see a NVEnc encoder was successfully used on card `/dev/nvidia1` to
encode a 2-minute video clip.

If something goes wrong, you'll se a lot of error messages in the console.
Sometimes they are helpful, sometimes they are not. In the very worst case,
wrong configuration may lead to a segfault or any other kind of faulty behavior.
This is also the reason why HW-acceleration isn't turned on by default.

This is what it looks like when the system cannot use any of the found devices,
so it falls back to SW encoder:

```
[Msg] Recording started: 2950s (sim time), 52132s (real time)
[GUI] [Msg] Recording video using sim time.
[GUI] [Msg] Recording video in lockstep mode
[GUI] [Msg] Recording video using bitrate: 8000000
[GUI] [Msg] Found known HW encoder: h264_nvenc
[GUI] [Err] [Util.cc:58] ffmpeg [AVHWDeviceContext] Could not initialize the CUDA driver API

[GUI] [Err] [HWEncoder.cc:159] Could not initialize device /dev/nvidia3 for NVENC, skipping.
[GUI] [Msg] Found known HW encoder: h264_omx
[GUI] [Msg] Encoder h264_omx is not allowed, skipping.
[GUI] [Msg] Found known HW encoder: h264_v4l2m2m
[GUI] [Msg] Encoder h264_v4l2m2m is not allowed, skipping.
[GUI] [Msg] Found known HW encoder: h264_vaapi
[GUI] [Msg] Encoder h264_vaapi is not allowed, skipping.
[GUI] [Msg] Found known HW encoder: nvenc
[GUI] [Err] [Util.cc:58] ffmpeg [AVHWDeviceContext] Could not initialize the CUDA driver API

[GUI] [Err] [HWEncoder.cc:159] Could not initialize device /dev/nvidia3 for NVENC, skipping.
[GUI] [Msg] Found known HW encoder: nvenc_h264
[GUI] [Err] [Util.cc:58] ffmpeg [AVHWDeviceContext] Could not initialize the CUDA driver API

[GUI] [Err] [HWEncoder.cc:159] Could not initialize device /dev/nvidia3 for NVENC, skipping.
[GUI] [Wrn] [HWEncoder.cc:384] No hardware-accelerated encoder found, falling back to software encoders
[GUI] [Msg] Compatible SW encoder: libx264
[GUI] [Msg] Using encoder libx264
```

# Examples

Here are a few ready-made examples which might or might not work for you right-away.
Just give them a try and dig deeper in the configuration if something is wrong.

## Linux/Win + Intel GPU

    GZ_VIDEO_ALLOWED_ENCODERS=QSV

## Linux/Win + NVidia GPU

    GZ_VIDEO_ALLOWED_ENCODERS=NVENC

## Linux + Intel/NVidia GPU

    GZ_VIDEO_ALLOWED_ENCODERS=VAAPI

## Linux NVidia Multi-GPU machine

    GZ_VIDEO_ALLOWED_ENCODERS=NVENC GZ_VIDEO_ENCODER_DEVICE=/dev/nvidia2

# Caveats

## FFMpeg on Windows via Conda

The current distribution of FFMpeg via Conda for Windows does not include the h264_qsv
encoder. It only has h264_nvenc. This means that using Intel GPUs for HW acceleration is
not possible out of the box. A possible solution would be to build a custom build of
ffmpeg in the workspace with gz-common. Pull requests with the relevant tutorial are
welcome.

## NVEnc per-machine limit

If you have a multi-GPU station with desktop-class (not server-class) GPUs, you will
run into an artificial limitation from NVidia. You can only run 3 concurrent
encoding sessions on one computer, no matter how many GPUs you have. The computer
can even have 8 GPUs, but you will only be able to encode 3 videos at a time. The
exact maxima of encoding sessions are described in
[this NVidia support page](https://developer.nvidia.com/video-encode-and-decode-gpu-support-matrix-new).

To make things worse, there is no API that would tell you the number of currently
running NVEnc sessions. The only way to find out you're launching the fourth is
trying to start encoding and getting a memory allocation error. This library
catches this error and writes a lengthy description of what might have just happened
(either really low memory or this artificial limit). Unfortunately, when you start
the doomed fourth encoding session, all the three "legal" sessions will crash.
This might be really troublesome on e.g. multi-user systems when you don't even know
which jobs of the other users are using NVEnc.

There is a workaround removing this artificial limit - patching the binary blob
drivers using https://github.com/keylase/nvidia-patch . This is an unofficial
patch that is not supported by NVidia or the Gazebo developers. It is up to
you if you can and want to do that. And there is no guarantee it will work forever.
