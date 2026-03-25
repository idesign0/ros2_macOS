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

#include "camera_aravis2/error.h"

namespace camera_aravis2
{

//==================================================================================================
GuardedGError::~GuardedGError()
{
    reset();
}

//==================================================================================================
void GuardedGError::reset()
{
    if (!err)
        return;
    g_error_free(err);
    err = nullptr;
}

//==================================================================================================
GError** GuardedGError::ref()
{
    this->reset();

    return &err;
}

//==================================================================================================
GError* GuardedGError::operator->() noexcept
{
    return err;
}

//==================================================================================================
GuardedGError::operator bool() const
{
    return nullptr != err;
}

//==================================================================================================
void GuardedGError::log(const rclcpp::Logger& logger,
                        const std::string& file,
                        const int& line,
                        const std::string& custom_msg) const
{
    if (err == nullptr)
        return;

    RCLCPP_ERROR(logger, "%s:%i: %s (Code %i): %s. %s",
                 file.c_str(), line,
                 g_quark_to_string(err->domain),
                 err->code, err->message, custom_msg.c_str());
}

}  // namespace camera_aravis2
