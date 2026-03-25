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
#include "gz/common/config.hh"
#include "gz/common/Console.hh"
#include "gz/common/ffmpeg_inc.hh"
#include "gz/common/Video.hh"
#include "gz/common/av/Util.hh"

using namespace gz;
using namespace common;

// Private data structure for the Video class
class common::Video::Implementation
{
  /// \brief libav Format I/O context
  public: AVFormatContext *formatCtx = nullptr;

  /// \brief libav main external API structure
  public: AVCodecContext *codecCtx = nullptr;

  /// \brief audio video frame
  public: AVFrame *avFrame = nullptr;

  /// \brief Destination audio video frame (32-byte aligned lines)
  public: AVFrame *avFrameDst = nullptr;

  /// \brief Line sizes of an unaligned output frame
  public: int dstLineSizes[4];

  /// \brief software scaling context
  public: SwsContext *swsCtx = nullptr;

  /// \brief index of first video stream or -1
  public: int videoStream = -1;

  /// \brief Pixel format of the output image. Has to be 24-bit RGB.
  public: AVPixelFormat dstPixelFormat = AV_PIX_FMT_RGB24;

  /// \brief When input data end, the decoder can still hold some decoded
  /// frames. According to
  /// https://www.ffmpeg.org/doxygen/3.4/group__lavc__encdec.html , end of
  /// stream situations require flushing, i.e. setting the codec in draining
  /// mode and reading what's left there. This variable tells whether we have
  /// already entered the flushing mode.
  public: bool drainingMode = false;
};

int AVCodecDecode(AVCodecContext *_codecCtx,
    AVFrame *_frame, int *_gotFrame, AVPacket *_packet)
{
  // from https://blogs.gentoo.org/lu_zero/2016/03/29/new-avcodec-api/
  int ret;

  *_gotFrame = 0;

  if (_packet)
  {
    ret = avcodec_send_packet(_codecCtx, _packet);
    if (ret < 0)
    {
      return ret == AVERROR_EOF ? 0 : ret;
    }
  }

  ret = avcodec_receive_frame(_codecCtx, _frame);
  if (ret < 0 && ret != AVERROR(EAGAIN))
  {
    return ret;
  }
  if (ret >= 0)
  {
    *_gotFrame = 1;
  }

  // new API always consumes the whole packet
  return _packet ? _packet->size : 0;
}

/////////////////////////////////////////////////
Video::Video()
  : dataPtr(utils::MakeUniqueImpl<Implementation>())
{
  // Make sure libav is loaded.
  common::load();
}

/////////////////////////////////////////////////
Video::~Video()
{
  this->Cleanup();
}

/////////////////////////////////////////////////
void Video::Cleanup()
{
  // Free the YUV frame
  av_free(this->dataPtr->avFrame);

  // Close the video file
  avformat_close_input(&this->dataPtr->formatCtx);

  // Close the codec
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 101)
  avcodec_free_context(&this->dataPtr->codecCtx);
#else
  avcodec_close(this->dataPtr->codecCtx);
#endif

  av_free(this->dataPtr->avFrameDst);
}

/////////////////////////////////////////////////
bool Video::Load(const std::string &_filename)
{
  const AVCodec * codec = nullptr;
  this->dataPtr->videoStream = -1;

  if (this->dataPtr->formatCtx || this->dataPtr->avFrame ||
      this->dataPtr->codecCtx)
  {
    this->Cleanup();
  }

  this->dataPtr->avFrame = av_frame_alloc();

  // Open video file
  if (avformat_open_input(&this->dataPtr->formatCtx, _filename.c_str(),
        nullptr, nullptr) < 0)
  {
    gzerr << "Unable to read video file[" << _filename << "]\n";
    return false;
  }

  // Retrieve stream information
  if (avformat_find_stream_info(this->dataPtr->formatCtx, nullptr) < 0)
  {
    gzerr << "Couldn't find stream information\n";
    return false;
  }

  // Find the first video stream
  for (unsigned int i = 0; i < this->dataPtr->formatCtx->nb_streams; ++i)
  {
    enum AVMediaType codec_type;
    codec_type = this->dataPtr->formatCtx->streams[i]->codecpar->codec_type;
    if (codec_type == AVMEDIA_TYPE_VIDEO)
    {
      this->dataPtr->videoStream = static_cast<int>(i);
      break;
    }
  }

  if (this->dataPtr->videoStream == -1)
  {
    gzerr << "Unable to find a video stream\n";
    return false;
  }

  // Find the decoder for the video stream
  auto stream = this->dataPtr->formatCtx->streams[this->dataPtr->videoStream];
  codec = avcodec_find_decoder(stream->codecpar->codec_id);
  if (codec == nullptr)
  {
    gzerr << "Codec not found\n";
    return false;
  }

  // AVCodecContext is not included in an AVStream as of ffmpeg 3.1
  // allocate a codec context based on updated example
  // github.com/FFmpeg/FFmpeg/commit/bba6a03b2816d805d44bce4f9701a71f7d3f8dad
  this->dataPtr->codecCtx = avcodec_alloc_context3(codec);
  if (!this->dataPtr->codecCtx)
  {
    gzerr << "Failed to allocate the codec context" << std::endl;
    return false;
  }

  // Copy codec parameters from input stream to output codec context
  if (avcodec_parameters_to_context(this->dataPtr->codecCtx,
                                    stream->codecpar) < 0)
  {
    gzerr << "Failed to copy codec parameters to decoder context"
           << std::endl;
    return false;
  }

  // Inform the codec that we can handle truncated bitstreams -- i.e.,
  // bitstreams where frame boundaries can fall in the middle of packets
#if LIBAVCODEC_VERSION_MAJOR < 60
  if (codec->capabilities & AV_CODEC_CAP_TRUNCATED)
    this->dataPtr->codecCtx->flags |= AV_CODEC_FLAG_TRUNCATED;
#endif

  // Open codec
  if (avcodec_open2(this->dataPtr->codecCtx, codec, nullptr) < 0)
  {
    gzerr << "Could not open codec\n";
    return false;
  }

  this->dataPtr->swsCtx = sws_getContext(
      this->dataPtr->codecCtx->width,
      this->dataPtr->codecCtx->height,
      this->dataPtr->codecCtx->pix_fmt,
      this->dataPtr->codecCtx->width,
      this->dataPtr->codecCtx->height,
      this->dataPtr->dstPixelFormat,
      0, nullptr, nullptr, nullptr);

  if (this->dataPtr->swsCtx == nullptr)
  {
    gzerr << "Error while calling sws_getContext\n";
    return false;
  }

  // swscale needs 32-byte-aligned output frame on some systems
  this->dataPtr->avFrameDst = av_frame_alloc();
  this->dataPtr->avFrameDst->format = this->dataPtr->dstPixelFormat;
  this->dataPtr->avFrameDst->width = this->dataPtr->codecCtx->width;
  this->dataPtr->avFrameDst->height = this->dataPtr->codecCtx->height;
  av_frame_get_buffer(this->dataPtr->avFrameDst, 32);

  // dstLineSizes are the line sizes of unaligned image frame (needed for
  // copying data to the (unaligned) output buffer of the NextFrame() call)
  av_image_fill_linesizes(this->dataPtr->dstLineSizes,
                          this->dataPtr->dstPixelFormat,
                          this->dataPtr->codecCtx->width);

  // DEBUG: Will save all the frames
  // Image img;
  // char buf[1024];
  // int frame = 0;

  // // the decoding loop, running until EOF
  // while (this->GetNextFrame(img))
  // {
  //   printf("WH[%d %d]\n",this->dataPtr->codecCtx->width,
  //   this->dataPtr->codecCtx->height);
  //   snprintf(buf, sizeof(buf), "/tmp/test_%3d.png", frame++);
  //   img.SavePNG(buf);
  // }
  // printf("Done\n");

  return true;
}

/////////////////////////////////////////////////
bool Video::NextFrame(unsigned char **_buffer)
{
  AVPacket* packet;
  int frameAvailable = 0;
  int ret;

  while (frameAvailable == 0)
  {
    packet = av_packet_alloc();
    if (!packet)
    {
      gzerr << "Failed to allocate AVPacket" << std::endl;
      return false;
    }

    // this loop will always exit because each call to AVCodecDecode()
    // reads from the input buffer and it has to either end at some time or
    // return a valid frame

    // in draining mode, we no longer read the input stream as it has ended
    if (!this->dataPtr->drainingMode)
    {
      // read a frame from the input stream
      ret = av_read_frame(this->dataPtr->formatCtx, packet);
      if (ret < 0)
      {
        if (ret == AVERROR_EOF)
        {
          // end of stream, enter draining mode
          avcodec_send_packet(this->dataPtr->codecCtx, nullptr);
          this->dataPtr->drainingMode = true;
        }
        else
        {
          gzerr << "Error reading packet: " << av_err2str_cpp(ret)
                 << ". Stopped reading the file." << std::endl;
          return false;
        }
      }
      else if (packet->stream_index != this->dataPtr->videoStream)
      {
        // packet belongs to a stream we're not interested in (e.g. audio)
        av_packet_unref(packet);
        continue;
      }
    }

    // Process all the data in the frame
    ret = ::AVCodecDecode(
      this->dataPtr->codecCtx, this->dataPtr->avFrame, &frameAvailable,
      this->dataPtr->drainingMode ? nullptr : packet);

    if (ret == AVERROR_EOF)
    {
      if (!this->dataPtr->drainingMode)
        av_packet_unref(packet);
      return false;
    }
    else if (ret < 0)
    {
      gzerr << "Error while processing packet data: "
             << av_err2str_cpp(ret) << std::endl;
      // continue processing data
    }
    if (!this->dataPtr->drainingMode)
      av_packet_unref(packet);
  }

  // processing the image if available
  if (frameAvailable)
  {
    sws_scale(this->dataPtr->swsCtx,
              this->dataPtr->avFrame->data,
              this->dataPtr->avFrame->linesize,
              0,
              this->dataPtr->codecCtx->height,
              this->dataPtr->avFrameDst->data,
              this->dataPtr->avFrameDst->linesize);

    // avFrameDst now contains data that are in RGB24, but have 32-byte aligned
    // lines; dstLineSizes are the line sizes of unaligned RGB24 which we want
    // in the output buffer
    av_image_copy(_buffer,
                  this->dataPtr->dstLineSizes,
                  const_cast<const uint8_t **>(this->dataPtr->avFrameDst->data),
                  this->dataPtr->avFrameDst->linesize,
                  this->dataPtr->dstPixelFormat,
                  this->dataPtr->codecCtx->width,
                  this->dataPtr->codecCtx->height);

    // Debug:
    // pgm_save(_buffer, this->dataPtr->dstLineSizes[0],
    // this->dataPtr->codecCtx->width,
    // this->dataPtr->codecCtx->height, buf);

    return true;
  }

  return false;  // shouldn't ever get here, but just to be sure
}

/////////////////////////////////////////////////
int Video::Width() const
{
  return this->dataPtr->codecCtx->width;
}

/////////////////////////////////////////////////
int Video::Height() const
{
  return this->dataPtr->codecCtx->height;
}

/////////////////////////////////////////////////
Video::Length Video::Duration() const
{
  return Video::Length(this->dataPtr->formatCtx->duration);
}
