/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <string>

#include <gz/common/Console.hh>
#include <gz/common/av/Util.hh>
#include <gz/common/ffmpeg_inc.hh>

/////////////////////////////////////////////////
// avcodec log callback. We use this to redirect message to gazebo's console
// messages.
void logCallback(void *_ptr, int _level, const char *_fmt, va_list _args)
{
  static char message[8192];

  if (_level > av_log_get_level())
    return;

  std::string msg = "ffmpeg ";

  // Get the ffmpeg module.
  if (_ptr)
  {
    AVClass *avc = *reinterpret_cast<AVClass**>(_ptr);
    const char *module = avc->item_name(_ptr);
    if (module)
      msg += std::string("[") + module + "] ";
  }

  // Create the actual message
  vsnprintf(message, sizeof(message), _fmt, _args);
  msg += message;

  // Output to the appropriate stream.
  switch (_level)
  {
    case AV_LOG_DEBUG:
      // There are a lot of debug messages. So we'll skip those.
      break;
    case AV_LOG_PANIC:
    case AV_LOG_FATAL:
    case AV_LOG_ERROR:
      gzerr << msg << std::endl;
      break;
    case AV_LOG_WARNING:
      gzwarn << msg << std::endl;
      break;
    default:
      gzmsg << msg << std::endl;
      break;
  }
}

/////////////////////////////////////////////////
void gz::common::load()
{
  static bool first = true;
  if (first)
  {
    first = false;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
    avcodec_register_all();
    av_register_all();
#endif

#if defined(__linux__) && defined(HAVE_AVDEVICE)
    avdevice_register_all();
#endif

    // Set the log callback function.
    av_log_set_callback(logCallback);
  }
}
