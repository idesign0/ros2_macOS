// Copyright (c) 2024 Fraunhofer IOSB and contributors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//    * Neither the name of the Fraunhofer IOSB nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef CAMERA_ARAVIS2__IMAGE_BUFFER_POOL_H_
#define CAMERA_ARAVIS2__IMAGE_BUFFER_POOL_H_

// Std
#include <map>
#include <memory>
#include <mutex>
#include <stack>

// Aravis
extern "C"
{
#include "aravis-0.8/arv.h"
}

// ROS
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace camera_aravis2
{

class ImageBufferPool : public std::enable_shared_from_this<ImageBufferPool>
{
  public:
    typedef std::shared_ptr<ImageBufferPool> SharedPtr;
    typedef std::weak_ptr<ImageBufferPool> WeakPtr;

    // Note: If the ImageBufferPool is destroyed, buffers will be deallocated. Therefor, make sure
    // that the ImageBufferPool stays alive longer than the given stream object.
    //
    // logger: logger object
    // stream: weakly managed pointer to the stream. Used to register all allocated buffers
    // payload_size_bytes: size of a single buffer
    // n_preallocated_buffers: number of initially allocated and registered buffers
    ImageBufferPool(const rclcpp::Logger& logger, ArvStream* stream,
                    size_t payload_size_bytes, size_t n_preallocated_buffers = 2);
    virtual ~ImageBufferPool();

    // Get an image whose lifespan is administrated by this pool (but not registered to the camera).
    sensor_msgs::msg::Image::SharedPtr getRecyclableImg();

    // Get the image message which wraps around the given ArvBuffer.
    //
    // If this buffer is not administrated by this ImageBufferPool,
    // a new image message is allocated and the contents of the buffer
    // are copied to it.
    sensor_msgs::msg::Image::SharedPtr operator[](ArvBuffer* buffer);

    inline size_t getAllocatedSize() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return n_buffers_;
    }

    inline size_t getUsedSize() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return used_buffers_.size();
    }

    inline size_t getPayloadSize() const
    {
        return payload_size_bytes_;
    }

    // Allocate new buffers which are wrapped by an image message and
    // push them to the internal aravis stream.
    void allocateBuffers(size_t n = 1);

  protected:
    // Custom deleter for aravis buffer wrapping image messages, which
    // either pushes the buffer back to the aravis stream cleans it up
    // when the ImageBufferPool is gone.
    static void reclaim(const WeakPtr& self, sensor_msgs::msg::Image* p_img);

    // Push the buffer inside the given image back to the aravis stream,
    // remember the corresponding image message.
    void push(sensor_msgs::msg::Image* p_img);

    ArvStream* stream_         = NULL;
    size_t payload_size_bytes_ = 0;
    size_t n_buffers_          = 0;

    std::map<const uint8_t*, sensor_msgs::msg::Image::SharedPtr> available_img_buffers_;
    std::map<sensor_msgs::msg::Image*, ArvBuffer*> used_buffers_;
    std::stack<sensor_msgs::msg::Image::SharedPtr> dangling_imgs_;
    mutable std::mutex mutex_;
    SharedPtr self_;

    rclcpp::Logger logger_;
};

}  // namespace camera_aravis2

#endif  // CAMERA_ARAVIS2__IMAGE_BUFFER_POOL_H_
