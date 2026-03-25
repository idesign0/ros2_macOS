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
#ifndef GZ_COMMON_AUDIO_DECODER_HH_
#define GZ_COMMON_AUDIO_DECODER_HH_

#include <stdint.h>
#include <string>

#include <gz/common/av/Export.hh>
#include <gz/utils/ImplPtr.hh>

namespace gz
{
  namespace common
  {
    /// \class AudioDecoder AudioDecoder.hh gz/common/common.hh
    /// \brief An audio decoder based on FFMPEG.
    class GZ_COMMON_AV_VISIBLE AudioDecoder
    {
      /// \brief Constructor.
      public: AudioDecoder();

      /// \brief Set the file to decode.
      /// \param[in] _filename Path to an audio file.
      /// \return True if the file was successfull opened.
      public: bool SetFile(const std::string &_filename);

      /// \brief Get the audio filename that was set.
      /// \return The name of the set audio file.
      /// \sa AudioDecoder::SetFile
      public: std::string File() const;

      /// \brief Decode the loaded audio file.
      /// \sa AudioDecoder::SetFile
      /// \param[out] _outBuffer Buffer that holds the decoded audio data.
      /// \param[out] _outBufferSize Size of the _outBuffer.
      /// \return True if decoding was succesful.
      public: bool Decode(uint8_t **_outBuffer, unsigned int *_outBufferSize);

      /// \brief Get the sample rate from the latest decoded file.
      /// \return Integer sample rate, such as 44100.
      /// If no file is decoded, -1 is returned.
      public: int SampleRate();

      /// \brief Private data pointer
      GZ_UTILS_UNIQUE_IMPL_PTR(dataPtr)
    };
  }
}
#endif
