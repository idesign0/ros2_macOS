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

#ifndef CAMERA_ARAVIS2__ERROR_H_
#define CAMERA_ARAVIS2__ERROR_H_

// GLib
#include <glib-2.0/glib.h>

// Std
#include <string>

// ROS
#include <rclcpp/rclcpp.hpp>

/// Macro to check if error occurred and log if necessary
#define CHECK_GERROR(err, logger) \
    if (err)                      \
        err.log(logger, __FILE__, __LINE__);

/// Macro to check if error occurred and log if necessary with costom message
#define CHECK_GERROR_MSG(err, logger, custom_msg) \
    if (err)                                      \
        err.log(logger, __FILE__, __LINE__, custom_msg);

/// Macro to assert success and log GError if necessary
#define CHECK_SUCCESS_GERROR(err, logger, success) \
    if (err)                                       \
        err.log(logger, __FILE__, __LINE__);       \
    success &= !err;

/// Macro to assert success and log GError if necessary with custom mesage
#define CHECK_SUCCESS_GERROR_MSG(err, logger, custom_msg, success) \
    if (err)                                                       \
        err.log(logger, __FILE__, __LINE__, custom_msg);           \
    success &= !err;

namespace camera_aravis2
{

class GuardedGError
{
    //--- METHOD DECLARATION ---//
  public:
    GuardedGError() = default;

    ~GuardedGError();

    void reset();

    GError** ref();

    GError* operator->() noexcept;

    operator bool() const;

    void log(const rclcpp::Logger& logger,
             const std::string& file,
             const int& line,
             const std::string& custom_msg = "") const;

    //--- MEMBER DECLARATION ---//

  private:
    GError* err = nullptr;
};

}  // namespace camera_aravis2

#endif  // CAMERA_ARAVIS2__ERROR_H_
