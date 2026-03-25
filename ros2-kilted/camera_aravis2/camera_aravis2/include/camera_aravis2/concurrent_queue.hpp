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

#ifndef CAMERA_ARAVIS2__CONCURRENT_QUEUE_HPP_
#define CAMERA_ARAVIS2__CONCURRENT_QUEUE_HPP_

// Std
#include <condition_variable>
#include <mutex>
#include <queue>

namespace camera_aravis2
{

/**
 * @brief Templated class implementing a thread safe queue that can be accessed (via push and pop)
 * from multiple threads. This implementation is inspired by the `concurrent_queue` within the TBB
 * library.
 */
template <typename T>
class ConcurrentQueue
{
    //--- METHOD DECLARATION ---//

  public:
    /**
     * @return True, if queue is empty. False, otherwise.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        std::lock_guard<std::mutex> lck(mtx_);
        return queue_.empty();
    }

    /**
     * @return Size of queue. I.e. number of elements stored inside queue.
     */
    [[nodiscard]] size_t size() const noexcept
    {
        std::lock_guard<std::mutex> lck(mtx_);
        return queue_.size();
    }

    /**
     * @brief Pushed a copy of `input` into the queue container. This will induce a notification
     * via the condition variable that a new member is inside the queue.
     */
    void push(const T& input) noexcept
    {
        {
            std::lock_guard<std::mutex> lck(mtx_);
            queue_.push(input);
        }
        cond_.notify_one();
    }

    /**
     * @brief Pops a copy of the first item in the container and assigns it to `output`. The popped
     * item is destroyed. If queue  is empty, this method will wait until an item becomes available.
     */
    void pop(T& output) noexcept
    {
        std::unique_lock<std::mutex> lck(mtx_);
        cond_.wait(lck, [this]
                   { return !queue_.empty(); });
        output = queue_.front();
        queue_.pop();
    }

    //--- MEMBER DECLARATION ---//

  private:
    /// Queue object
    std::queue<T> queue_;

    /// Condition variable to notify when new object is within queue
    std::condition_variable cond_;

    /// Mutex, guarding queue
    mutable std::mutex mtx_;
};

}  // namespace camera_aravis2

#endif  // CAMERA_ARAVIS2__CONCURRENT_QUEUE_HPP_
