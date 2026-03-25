/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ICameraListObserver.h

  Description: Definition of interface AVT::VmbAPI::ICameraListObserver.

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

#ifndef AVT_VMBAPI_ICAMERALISTOBSERVER_H
#define AVT_VMBAPI_ICAMERALISTOBSERVER_H

#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>
#include <VimbaCPP/Include/Camera.h>
#include <vector>


namespace AVT {
namespace VmbAPI {

class ICameraListObserver 
{
  public:
    //
    // Method:      CameraListChanged()
    //
    // Purpose:     The event handler function that gets called whenever
    //              an ICameraListObserver is triggered. This occurs most
    //              likely when a camera was plugged in or out.
    //
    // Parameters:
    //
    // [out]    CameraPtr           pCam                    The camera that triggered the event
    // [out]    UpdateTriggerType   reason                  The reason why the callback routine was triggered
    //                                                      (e.g., a new camera was plugged in)
    //
    IMEXPORT virtual void CameraListChanged( CameraPtr pCam, UpdateTriggerType reason ) = 0;

    //
    // Method:      ICameraListObserver destructor
    //
    // Purpose:     Destroys an instance of class ICameraListObserver
    //
    IMEXPORT virtual ~ICameraListObserver() {}

  protected:
    IMEXPORT ICameraListObserver() { /*No default ctor*/ }
    IMEXPORT ICameraListObserver( const ICameraListObserver& ) { /* No copy ctor */ }
};
typedef std::vector<ICameraListObserverPtr> ICameraListObserverPtrVector;

}} // namespace AVT::VmbAPI

#endif
