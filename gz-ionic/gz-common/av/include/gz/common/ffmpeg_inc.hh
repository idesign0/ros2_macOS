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
#ifndef GZ_COMMON_FFMPEG_INC_HH_
#define GZ_COMMON_FFMPEG_INC_HH_

/// Versions of FFMPEG on Gazebo supported platforms
// v4.2.7 (Ubuntu Focal)
// libavutil      56. 31.100 / 56. 31.100
// libavcodec     58. 54.100 / 58. 54.100
// libavformat    58. 29.100 / 58. 29.100
// libavdevice    58.  8.100 / 58.  8.100
// libavfilter     7. 57.100 /  7. 57.100
// libavresample   4.  0.  0 /  4.  0.  0
// libswscale      5.  5.100 /  5.  5.100
// libswresample   3.  5.100 /  3.  5.100
// libpostproc    55.  5.100 / 55.  5.100

// v4.4.1 (Windows CI vcpkg)
// libavutil      56. 70.100 / 56. 70.100
// libavcodec     58.134.100 / 58.134.100
// libavformat    58. 76.100 / 58. 76.100
// libavdevice    58. 13.100 / 58. 13.100
// libavfilter     7.110.100 /  7.110.100
// libswscale      5.  9.100 /  5.  9.100
// libswresample   3.  9.100 /  3.  9.100
// libpostproc    55.  9.100 / 55.  9.100

// v4.4.2 (Ubuntu Jammy)
// libavutil      56. 70.100 / 56. 70.100
// libavcodec     58.134.100 / 58.134.100
// libavformat    58. 76.100 / 58. 76.100
// libavdevice    58. 13.100 / 58. 13.100
// libavfilter     7.110.100 /  7.110.100
// libswscale      5.  9.100 /  5.  9.100
// libswresample   3.  9.100 /  3.  9.100
// libpostproc    55.  9.100 / 55.  9.100

// v5.1 (homebrew)
//  libavutil      57. 28.100 / 57. 28.100
//  libavcodec     59. 37.100 / 59. 37.100
//  libavformat    59. 27.100 / 59. 27.100
//  libavdevice    59.  7.100 / 59.  7.100
//  libavfilter     8. 44.100 /  8. 44.100
//  libswscale      6.  7.100 /  6.  7.100
//  libswresample   4.  7.100 /  4.  7.100
//  libpostproc    56.  6.100 / 56.  6.100

/// Additional versions of FFMPEG not officially supported
// v5.0.1 (conda-forge)
// libavutil      57. 17.100 / 57. 17.100
// libavcodec     59. 18.100 / 59. 18.100
// libavformat    59. 16.100 / 59. 16.100
// libavdevice    59.  4.100 / 59.  4.100
// libavfilter     8. 24.100 /  8. 24.100
// libswscale      6.  4.100 /  6.  4.100
// libswresample   4.  3.100 /  4.  3.100
// libpostproc    56.  3.100 / 56.  3.100


#include <string>

#include <gz/common/config.hh>

#ifndef _WIN32
#pragma GCC system_header
#endif

#ifndef INT64_C
#define INT64_C(c) (c ## LL)
#define UINT64_C(c) (c ## ULL)
#endif

#ifndef AV_ERROR_MAX_STRING_SIZE
#define AV_ERROR_MAX_STRING_SIZE 64
#endif

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/hwcontext.h>

#if defined(HAVE_AVDEVICE)
#include <libavdevice/avdevice.h>
#endif
}

#include <gz/common/av/Export.hh>

namespace gz
{
  namespace common
  {
    // av_err2str only works in C99, this is a version that works in C++;
    // https://github.com/joncampbell123/composite-video-simulator/issues/5#issuecomment-611885908
    inline std::string av_err2str_cpp(int errnum)
    {
      thread_local char buf[AV_ERROR_MAX_STRING_SIZE];
      memset(buf, 0, sizeof(buf));
      // lavu 51.58.100
      return av_make_error_string(buf, AV_ERROR_MAX_STRING_SIZE, errnum);
    }
  }
}

#endif
