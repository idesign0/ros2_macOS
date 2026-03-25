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

#include <gz/common/av/Util.hh>
#include <gz/common/ffmpeg_inc.hh>
#include <gz/common/AudioDecoder.hh>
#include <gz/common/Console.hh>

#define AUDIO_INBUF_SIZE (20480 * 2)
#define AUDIO_REFILL_THRESH 4096

using namespace gz;
using namespace common;

class common::AudioDecoder::Implementation
{
  /// \brief Destructor
  public: ~Implementation();

  /// \brief libav Format I/O context.
  public: AVFormatContext *formatCtx {nullptr};

  /// \brief libav main external API structure.
  public: AVCodecContext *codecCtx {nullptr};

  /// \brief libavcodec audio codec.
  public: const AVCodec *codec {nullptr};

  /// \brief Index of the audio stream.
  public: int audioStream {0};

  /// \brief Audio file to decode.
  public: std::string filename;
};

/////////////////////////////////////////////////
common::AudioDecoder::Implementation::~Implementation()
{
  // Close the codec
  if (this->codecCtx)
  {
#if LIBAVFORMAT_VERSION_MAJOR < 59
    avcodec_close(this->codecCtx);
#else
    avcodec_free_context(&this->codecCtx);
#endif
  }

  // Close the audio file
  if (this->formatCtx)
    avformat_close_input(&this->formatCtx);
}

/////////////////////////////////////////////////
AudioDecoder::AudioDecoder()
  : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
  common::load();
}

/////////////////////////////////////////////////
bool AudioDecoder::Decode(uint8_t **_outBuffer, unsigned int *_outBufferSize)
{
  AVPacket *packet;
  unsigned int maxBufferSize = 0;
  AVFrame *decodedFrame = nullptr;

  if (this->dataPtr->codec == nullptr)
  {
    gzerr << "Set an audio file before decoding.\n";
    return false;
  }

  if (_outBufferSize == nullptr)
  {
    gzerr << "outBufferSize is null!!\n";
    return false;
  }

  *_outBufferSize = 0;

  if (*_outBuffer)
  {
    free(*_outBuffer);
    *_outBuffer = nullptr;
  }

  bool result = true;

  if (!(decodedFrame = av_frame_alloc()))
  {
    gzerr << "Audio decoder out of memory\n";
    result = false;
  }

  packet = av_packet_alloc();
  if (!packet)
  {
    gzerr << "Failed to allocate AVPacket" << std::endl;
    return false;
  }

  while (av_read_frame(this->dataPtr->formatCtx, packet) == 0)
  {
    if (packet->stream_index == this->dataPtr->audioStream)
    {
      // Inspired from
      // https://github.com/FFmpeg/FFmpeg/blob/n5.0/doc/examples/decode_audio.c#L71

      // send the packet with the compressed data to the decoder
      int ret = avcodec_send_packet(this->dataPtr->codecCtx, packet);
      if (ret < 0)
      {
        gzerr << "Error submitting the packet to the decoder" << std::endl;
        av_packet_free(&packet);
        av_frame_free(&decodedFrame);
        return false;
      }

      // read all the output frames
      // (in general there may be any number of them)
      while (ret >= 0)
      {
        ret = avcodec_receive_frame(this->dataPtr->codecCtx, decodedFrame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
          break;
        }
        else if (ret < 0)
        {
            gzerr << "Error during decoding" << std::endl;
            av_packet_free(&packet);
            av_frame_free(&decodedFrame);
            return false;
        }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(59, 24, 100)
        int numChannels = this->dataPtr->codecCtx->ch_layout.nb_channels;
#else
        int numChannels = this->dataPtr->codecCtx->channels;
#endif

        int bytesPerSample =
          av_get_bytes_per_sample(this->dataPtr->codecCtx->sample_fmt);

        // Total size of the data. Some padding can be added to
        // decodedFrame->data[0], which is why we can't use
        // decodedFrame->linesize[0].
        int size = decodedFrame->nb_samples * bytesPerSample * numChannels;

        // Resize the audio buffer as necessary
        if (*_outBufferSize + size > maxBufferSize)
        {
          maxBufferSize += size * 5;
          *_outBuffer = reinterpret_cast<uint8_t*>(realloc(*_outBuffer,
                maxBufferSize * sizeof(*_outBuffer[0])));
        }

        if (av_sample_fmt_is_planar(this->dataPtr->codecCtx->sample_fmt))
        {
          // Interleave planar data
          for (int s = 0; s < decodedFrame->nb_samples; ++s)
          {
            for (int c = 0; c < numChannels; ++c)
            {
              memcpy(*_outBuffer + *_outBufferSize +
                  (s * numChannels + c) * bytesPerSample,
                  decodedFrame->data[c] + s * bytesPerSample,
                  bytesPerSample);
            }
          }
        }
        else
        {
          memcpy(*_outBuffer + *_outBufferSize, decodedFrame->data[0],
              size);
        }
        *_outBufferSize += size;
    }
    }
    av_packet_unref(packet);
  }

  av_packet_free(&packet);
  av_frame_free(&decodedFrame);

  // Seek to the beginning so that it can be decoded again, if necessary.
  av_seek_frame(this->dataPtr->formatCtx, this->dataPtr->audioStream, 0, 0);

  return result;
}

/////////////////////////////////////////////////
int AudioDecoder::SampleRate()
{
  if (this->dataPtr->codecCtx)
    return this->dataPtr->codecCtx->sample_rate;

  return -1;
}

/////////////////////////////////////////////////
bool AudioDecoder::SetFile(const std::string &_filename)
{
  unsigned int i;

  this->dataPtr->formatCtx = avformat_alloc_context();

  // Open file
  if (avformat_open_input(&this->dataPtr->formatCtx,
        _filename.c_str(), nullptr, nullptr) < 0)
  {
    gzerr << "Unable to open audio file[" << _filename << "]\n";
    this->dataPtr->formatCtx = nullptr;
    return false;
  }

  // Hide av logging
  av_log_set_level(0);

  // Retrieve some information
  if (avformat_find_stream_info(this->dataPtr->formatCtx, nullptr) < 0)
  {
    gzerr << "Unable to find stream info.\n";
    avformat_close_input(&this->dataPtr->formatCtx);
    this->dataPtr->formatCtx = nullptr;

    return false;
  }

  // Dump information about file onto standard error.
  // dump_format(this->dataPtr->formatCtx, 0, "dump.txt", false);

  // Find audio stream;
  this->dataPtr->audioStream = -1;
  for (i = 0; i < this->dataPtr->formatCtx->nb_streams; ++i)
  {
    if (this->dataPtr->formatCtx->streams[i]->codecpar->codec_type == // NOLINT(*)
        AVMEDIA_TYPE_AUDIO)
    {
      this->dataPtr->audioStream = i;
      break;
    }
  }

  if (this->dataPtr->audioStream == -1)
  {
    gzerr << "Couldn't find audio stream.\n";
    avformat_close_input(&this->dataPtr->formatCtx);
    this->dataPtr->formatCtx = nullptr;

    return false;
  }

  // Find a decoder
  this->dataPtr->codec = avcodec_find_decoder(this->dataPtr->formatCtx->streams[
    this->dataPtr->audioStream]->codecpar->codec_id);
  if (!this->dataPtr->codec)
  {
    gzerr << "Failed to find the codec" << std::endl;
    return false;
  }
  this->dataPtr->codecCtx = avcodec_alloc_context3(this->dataPtr->codec);
  if (!this->dataPtr->codecCtx)
  {
    gzerr << "Failed to allocate the codec context" << std::endl;
    return false;
  }
  // Copy all relevant parameters from codepar to codecCtx
  avcodec_parameters_to_context(this->dataPtr->codecCtx,
    this->dataPtr->formatCtx->streams[this->dataPtr->audioStream]->codecpar);

  if (this->dataPtr->codec == nullptr)
  {
    gzerr << "Couldn't find codec for audio stream.\n";
    avformat_close_input(&this->dataPtr->formatCtx);
    this->dataPtr->formatCtx = nullptr;

    return false;
  }

#if LIBAVCODEC_VERSION_MAJOR < 60
  if (this->dataPtr->codec->capabilities & AV_CODEC_CAP_TRUNCATED)
    this->dataPtr->codecCtx->flags |= AV_CODEC_FLAG_TRUNCATED;
#endif

  // Open codec
  if (avcodec_open2(this->dataPtr->codecCtx,
        this->dataPtr->codec, nullptr) < 0)
  {
    gzerr << "Couldn't open audio codec.\n";
    avformat_close_input(&this->dataPtr->formatCtx);
    this->dataPtr->formatCtx = nullptr;

    return false;
  }

  this->dataPtr->filename = _filename;

  return true;
}

/////////////////////////////////////////////////
std::string AudioDecoder::File() const
{
  return this->dataPtr->filename;
}
