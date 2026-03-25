/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FrameHandler.h

  Description: Definition of class AVT::VmbAPI::FrameHandler.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifndef AVT_VMBAPI_FRAMEHANDLER_H
#define AVT_VMBAPI_FRAMEHANDLER_H

#include <vector>

#include "VimbaC/Include/VmbCommonTypes.h"
#include "VimbaCPP/Include/BasicLockable.h"
#include "VimbaCPP/Include/SharedPointerDefines.h"
#include "VimbaCPP/Include/Frame.h"
#include "VimbaCPP/Include/IFrameObserver.h"
#include "VimbaCPP/Include/Mutex.h"

namespace AVT {
namespace VmbAPI {

enum { FRAME_HDL=0, };

class FrameHandler
{
  public:
    static void VMB_CALL FrameDoneCallback( const VmbHandle_t handle, VmbFrame_t *pFrame );

    FrameHandler( FramePtr pFrame, IFrameObserverPtr pFrameObserver );

    FramePtr GetFrame() const;
    MutexPtr&               Mutex() { return m_pMutex; }
  private:
    IFrameObserverPtr       m_pObserver;
    FramePtr                m_pFrame;
    MutexPtr                m_pMutex;
};

typedef std::vector<FrameHandlerPtr> FrameHandlerPtrVector;

}} // namespace AVT::VmbAPI

#endif
