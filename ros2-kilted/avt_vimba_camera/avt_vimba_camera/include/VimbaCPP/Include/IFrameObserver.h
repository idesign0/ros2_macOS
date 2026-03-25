/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        IFrameObserver.h

  Description: Definition of interface AVT::VmbAPI::IFrameObserver.

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

#ifndef AVT_VMBAPI_IFRAMEOBSERVER_H
#define AVT_VMBAPI_IFRAMEOBSERVER_H

#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>
#include <VimbaCPP/Include/Frame.h>

namespace AVT {
namespace VmbAPI {

class IFrameObserver 
{
  public:
    //
    // Method:      FrameReceived()
    //
    // Purpose:     The event handler function that gets called whenever
    //              a new frame is received
    //
    // Parameters:
    //
    // [in]     const FramePtr      pFrame                  The frame that was received
    //
    IMEXPORT virtual void FrameReceived( const FramePtr pFrame ) = 0;

    //
    // Method:      IFrameObserver destructor
    //
    // Purpose:     Destroys an instance of class IFrameObserver
    //
    IMEXPORT virtual ~IFrameObserver() {}

  protected:
    CameraPtr m_pCamera;
    IMEXPORT IFrameObserver( CameraPtr pCamera ) : m_pCamera( pCamera ) {}

    IMEXPORT IFrameObserver( IFrameObserver& ) { /* No copy ctor */ }

  private:
    IFrameObserver() { /* No default ctor */ }
};

}} // namespace AVT::VmbAPI

#endif
