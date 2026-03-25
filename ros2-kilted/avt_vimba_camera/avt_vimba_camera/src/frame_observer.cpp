/// Copyright (c) 2014,
/// Systems, Robotics and Vision Group
/// University of the Balearic Islands
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///     * Redistributions of source code must retain the above copyright
///       notice, this list of conditions and the following disclaimer.
///     * Redistributions in binary form must reproduce the above copyright
///       notice, this list of conditions and the following disclaimer in the
///       documentation and/or other materials provided with the distribution.
///     * All advertising materials mentioning features or use of this software
///       must display the following acknowledgement:
///       This product includes software developed by
///       Systems, Robotics and Vision Group, Univ. of the Balearic Islands
///     * Neither the name of Systems, Robotics and Vision Group, University of
///       the Balearic Islands nor the names of its contributors may be used
///       to endorse or promote products derived from this software without
///       specific prior written permission.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
/// ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
/// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
/// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
/// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
/// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
/// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "avt_vimba_camera/frame_observer.hpp"

#include <iostream>

FrameObserver::FrameObserver(CameraPtr cam_ptr, Callback callback)
  : IFrameObserver(cam_ptr), cam_ptr_(cam_ptr), callback_(callback)
{
}

void FrameObserver::FrameReceived(const FramePtr vimba_frame_ptr)
{
  VmbFrameStatusType eReceiveStatus;
  VmbErrorType err = vimba_frame_ptr->GetReceiveStatus(eReceiveStatus);

  if (err == VmbErrorSuccess)
  {
    switch (eReceiveStatus)
    {
      case VmbFrameStatusComplete: {
        // Call the callback
        callback_(vimba_frame_ptr);
        break;
      }
      case VmbFrameStatusIncomplete: {
        std::cout << "ERR: FrameObserver VmbFrameStatusIncomplete" << std::endl;
        break;
      }
      case VmbFrameStatusTooSmall: {
        std::cout << "ERR: FrameObserver VmbFrameStatusTooSmall" << std::endl;
        break;
      }
      case VmbFrameStatusInvalid: {
        std::cout << "ERR: FrameObserver VmbFrameStatusInvalid" << std::endl;
        break;
      }
      default: {
        std::cout << "ERR: FrameObserver no known status" << std::endl;
        break;
      }
    }
  }

  cam_ptr_->QueueFrame(vimba_frame_ptr);
}
