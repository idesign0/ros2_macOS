/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ICameraFactory.h

  Description: Definition of interface AVT::VmbAPI::ICameraFactory.

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

#ifndef AVT_VMBAPI_ICAMERAFACTORY_H
#define AVT_VMBAPI_ICAMERAFACTORY_H

#include <VimbaC/Include/VimbaC.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>
#include <VimbaCPP/Include/Camera.h>

namespace AVT {
namespace VmbAPI {

class ICameraFactory 
{
  public:
    //
    // Method:      CreateCamera()
    //
    // Purpose:     Factory method to create a camera that extends the Camera class
    //
    // Parameters:
    //
    // [in ]    const char*         pCameraID                   The ID of the camera
    // [in ]    const char*         pCameraName                 The name of the camera
    // [in ]    const char*         pCameraModel                The model name of the camera
    // [in ]    const char*         pCameraSerialNumber         The serial number of the camera
    // [in ]    const char*         pInterfaceID                The ID of the interface the camera is connected to
    // [in ]    VmbInterfaceType    interfaceType               The type of the interface the camera is connected to
    // [in ]    const char*         pInterfaceName              The name of the interface
    // [in ]    const char*         pInterfaceSerialNumber      The serial number of the interface
    // [in ]    VmbAccessModeType   interfacePermittedAccess    The access privileges for the interface
    //
    // Details:   The ID of the camera may be, among others, one of the following: "169.254.12.13",
    //            "000f31000001", a plain serial number: "1234567890", or the device ID 
    //            of the underlying transport layer.
    //
    IMEXPORT virtual CameraPtr CreateCamera(    const char *pCameraID,
                                                const char *pCameraName,
                                                const char *pCameraModel,
                                                const char *pCameraSerialNumber,
                                                const char *pInterfaceID,
                                                VmbInterfaceType interfaceType,
                                                const char *pInterfaceName,
                                                const char *pInterfaceSerialNumber,
                                                VmbAccessModeType interfacePermittedAccess) = 0;

    //
    // Method:      ICameraFactory destructor
    //
    // Purpose:     Destroys an instance of class Camera
    //
    IMEXPORT virtual ~ICameraFactory() {}

};

}} // namespace AVT::VmbAPI

#endif
