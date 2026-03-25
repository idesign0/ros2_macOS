/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Camera.h

  Description: Definition of class AVT::VmbAPI::Camera.

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

#ifndef AVT_VMBAPI_CAMERA_H
#define AVT_VMBAPI_CAMERA_H

#include <vector>
#include <string>

#include <VimbaC/Include/VimbaC.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/IRegisterDevice.h>
#include <VimbaCPP/Include/FeatureContainer.h>
#include <VimbaCPP/Include/Frame.h>
#include <VimbaCPP/Include/IFrameObserver.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>

namespace AVT {
namespace VmbAPI {

typedef std::vector<CameraPtr> CameraPtrVector;

class Camera : public FeatureContainer, public IRegisterDevice 
{
  public:
    //
    // Method:      Camera constructor
    //
    // Purpose:     Creates an instance of class Camera
    //
    // Parameters:
    //
    // [in ]    const char*      pID            The ID of the camera    
    // [in ]    const char*      pName          The name of the camera
    // [in ]    const char*      pModel         The model name of the camera
    // [in ]    const char*      pSerialNumber  The serial number of the camera
    // [in ]    const char*      pInterfaceID   The ID of the interface the camera is connected to
    // [in ]    VmbInterfaceType interfaceType  The type of the interface the camera is connected to
    //
    // Details:   The ID of the camera may be, among others, one of the following: "169.254.12.13",
    //            "000f31000001", a plain serial number: "1234567890", or the device ID 
    //            of the underlying transport layer.
    //
    IMEXPORT Camera(    const char *pID,
                        const char *pName,
                        const char *pModel,
                        const char *pSerialNumber,
                        const char *pInterfaceID,
                        VmbInterfaceType interfaceType );

    //
    // Method:      Camera destructor
    //
    // Purpose:     Destroys an instance of class Camera
    //
    // Details:     Destroying a camera implicitly closes it beforehand.
    //
    IMEXPORT virtual ~Camera();

    //
    // Method:      Open()
    //
    // Purpose:     Opens the specified camera.
    //
    // Parameters:
    //
    //  [in ]  VmbAccessMode_t  accessMode      Access mode determines the level of control you have on the camera
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorNotFound:      The designated camera cannot be found
    //  - VmbErrorInvalidAccess: Operation is invalid with the current access mode
    //
    // Details:   A camera may be opened in a specific access mode. This mode determines
    //            the level of control you have on a camera.
    //  
    IMEXPORT virtual VmbErrorType Open( VmbAccessModeType accessMode );

    //
    // Method:      Close()
    //
    // Purpose:     Closes the specified camera.
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //
    // Details:     Depending on the access mode this camera was opened in, events are killed,
    //              callbacks are unregistered, the frame queue is cleared, and camera control is released.
    //
    IMEXPORT virtual VmbErrorType Close();

    //
    // Method:      GetID()
    //
    // Purpose:     Gets the ID of a camera.
    //
    // Parameters:  [out]   std::string&     cameraID         The ID of the camera
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    //              This information remains static throughout the object's lifetime
    //
    VmbErrorType GetID( std::string &cameraID ) const;

    //
    // Method:      GetName()
    //
    // Purpose:     Gets the name of a camera.
    //
    // Parameters:  [out]   std::string&     name         The name of the camera
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    //              This information remains static throughout the object's lifetime
    //
    VmbErrorType GetName( std::string &name ) const;
    
    //
    // Method:      GetModel()
    //
    // Purpose:     Gets the model name of a camera.
    //
    // Parameters:  [out]   std::string&     model         The model name of the camera
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    //              This information remains static throughout the object's lifetime
    //
    VmbErrorType GetModel( std::string &model ) const;

    //
    // Method:      GetSerialNumber()
    //
    // Purpose:     Gets the serial number of a camera.
    //
    // Parameters:  [out]   std::string&     serialNumber         The serial number of the camera
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    //              This information remains static throughout the object's lifetime
    //
    VmbErrorType GetSerialNumber( std::string &serialNumber ) const;

    //
    // Method:      GetInterfaceID()
    //
    // Purpose:     Gets the interface ID of a camera.
    //
    // Parameters:  [out]   std::string&     interfaceID         The interface ID of the camera
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    //              This information remains static throughout the object's lifetime
    //
    VmbErrorType GetInterfaceID( std::string &interfaceID ) const;

    //
    // Method:      GetInterfaceType()
    //
    // Purpose:     Gets the type of the interface the camera is connected to. And therefore the type of the camera itself.
    //
    // Parameters:  [out]   VmbInterfaceType&    interfaceType The interface type of the camera
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    IMEXPORT VmbErrorType GetInterfaceType( VmbInterfaceType &interfaceType ) const;

    //
    // Method:      GetPermittedAccess()
    //
    // Purpose:     Gets the access modes of a camera.
    //
    // Parameters:  [out]   VmbAccessModeType&   permittedAccess The possible access modes of the camera
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    IMEXPORT VmbErrorType GetPermittedAccess( VmbAccessModeType &permittedAccess ) const;

    //
    // Method:      ReadRegisters()
    //
    // Purpose:     Reads one or more registers consecutively. The number of registers to read is determined by the number of provided addresses.
    //
    // Parameters:  [in ]   const Uint64Vector&   addresses  A list of register addresses
    //              [out]   Uint64Vector&         buffer     The returned data as vector
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested registers have been read
    //  - VmbErrorBadParameter: Vectors "addresses" and/or "buffer" are empty.
    //  - VmbErrorIncomplete:   If at least one, but not all registers have been read. See overload ReadRegisters( const Uint64Vector&, Uint64Vector&, VmbUint32_t& ).
    //
    virtual VmbErrorType ReadRegisters( const Uint64Vector &addresses, Uint64Vector &buffer ) const;
    
    //
    // Method:      ReadRegisters()
    //
    // Purpose:     Same as ReadRegisters( const Uint64Vector&, Uint64Vector& ), but returns the number of successful read operations in case of an error.
    //
    // Parameters:  [in ]   const Uint64Vector&   addresses       A list of register addresses
    //              [out]   Uint64Vector&         buffer          The returned data as vector
    //              [out]   VmbUint32_t&          completedReads  The number of successfully read registers
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested registers have been read
    //  - VmbErrorBadParameter: Vectors "addresses" and/or "buffer" are empty.
    //  - VmbErrorIncomplete:   If at least one, but not all registers have been read.
    //
    virtual VmbErrorType ReadRegisters( const Uint64Vector &addresses, Uint64Vector &buffer, VmbUint32_t &completedReads ) const;
    
    //
    // Method:      WriteRegisters()
    //
    // Purpose:     Writes one or more registers consecutively. The number of registers to write is determined by the number of provided addresses.
    //
    // Parameters:  [in]    const Uint64Vector&     addresses  A list of register addresses
    //              [in]    const Uint64Vector&     buffer     The data to write as vector
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested registers have been written
    //  - VmbErrorBadParameter: Vectors "addresses" and/or "buffer" are empty.
    //  - VmbErrorIncomplete:   If at least one, but not all registers have been written. See overload WriteRegisters( const Uint64Vector&, const Uint64Vector&, VmbUint32_t& ).
    //
    virtual VmbErrorType WriteRegisters( const Uint64Vector &addresses, const Uint64Vector &buffer );

    //
    // Method:      WriteRegisters()
    //
    // Purpose:     Same as WriteRegisters( const Uint64Vector&, const Uint64Vector& ), but returns the number of successful write operations in case of an error.
    //
    // Parameters:  [in ]   const Uint64Vector&   addresses        A list of register addresses
    //              [in ]   const Uint64Vector&   buffer           The data to write as vector
    //              [out]   VmbUint32_t&          completedWrites  The number of successfully read registers
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested registers have been written
    //  - VmbErrorBadParameter: Vectors "addresses" and/or "buffer" are empty.
    //  - VmbErrorIncomplete:   If at least one, but not all registers have been written.
    //
    virtual VmbErrorType WriteRegisters( const Uint64Vector &addresses, const Uint64Vector &buffer, VmbUint32_t &completedWrites );

    //
    // Method:      ReadMemory()
    //
    // Purpose:     Reads a block of memory. The number of bytes to read is determined by the size of the provided buffer.
    //
    // Parameters:  [in ]   const VmbUint64_t&   address    The address to read from
    //              [out]   UcharVector&         buffer     The returned data as vector
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested bytes have been read
    //  - VmbErrorBadParameter: Vector "buffer" is empty.
    //  - VmbErrorIncomplete:   If at least one, but not all bytes have been read. See overload ReadMemory( const VmbUint64_t&, UcharVector&, VmbUint32_t& ).
    //
    virtual VmbErrorType ReadMemory( const VmbUint64_t &address, UcharVector &buffer ) const;

    //
    // Method:      ReadMemory()
    //
    // Purpose:     Same as ReadMemory( const Uint64Vector&, UcharVector& ), but returns the number of bytes successfully read in case of an error VmbErrorIncomplete.
    //
    // Parameters:  [in]    const VmbUint64_t&   address        The address to read from
    //              [out]   UcharVector&         buffer         The returned data as vector
    //              [out]   VmbUint32_t&         completeReads  The number of successfully read bytes
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested bytes have been read
    //  - VmbErrorBadParameter: Vector "buffer" is empty.
    //  - VmbErrorIncomplete:   If at least one, but not all bytes have been read.
    //
    virtual VmbErrorType ReadMemory( const VmbUint64_t &address, UcharVector &buffer, VmbUint32_t &completeReads ) const;

    //
    // Method:      WriteMemory()
    //
    // Purpose:     Writes a block of memory. The number of bytes to write is determined by the size of the provided buffer.
    //
    // Parameters:  [in]    const VmbUint64_t&   address    The address to write to
    //              [in]    const UcharVector&   buffer     The data to write as vector
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested bytes have been written
    //  - VmbErrorBadParameter: Vector "buffer" is empty.
    //  - VmbErrorIncomplete:   If at least one, but not all bytes have been written. See overload WriteMemory( const VmbUint64_t&, const UcharVector&, VmbUint32_t& ).
    //
    virtual VmbErrorType WriteMemory( const VmbUint64_t &address, const UcharVector &buffer );

    //
    // Method:      WriteMemory()
    //
    // Purpose:     Same as WriteMemory( const Uint64Vector&, const UcharVector& ), but returns the number of bytes successfully written in case of an error VmbErrorIncomplete.
    //
    // Parameters:  [in]    const VmbUint64_t&   address        The address to write to
    //              [in]    const UcharVector&   buffer         The data to write as vector
    //              [out]   VmbUint32_t&         sizeComplete   The number of successfully written bytes
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested bytes have been written
    //  - VmbErrorBadParameter: Vector "buffer" is empty.
    //  - VmbErrorIncomplete:   If at least one, but not all bytes have been written.
    //
    virtual VmbErrorType WriteMemory( const VmbUint64_t &address, const UcharVector &buffer, VmbUint32_t &sizeComplete );

    //
    // Method:      AcquireSingleImage()
    //
    // Purpose:     Gets one image synchronously.
    //
    // Parameters:  [out]   FramePtr&       pFrame          The frame that gets filled
    //              [in ]   VmbUint32_t     timeout         The time to wait until the frame got filled
    //
    // Returns:
    //  - VmbErrorSuccess:      If no error
    //  - VmbErrorBadParameter: "pFrame" is NULL.
    //  - VmbErrorTimeout:      Call timed out
    //
    IMEXPORT VmbErrorType AcquireSingleImage( FramePtr &pFrame, VmbUint32_t timeout );

    //
    // Method:      AcquireMultipleImages()
    //
    // Purpose:     Gets a certain number of images synchronously.
    //
    // Parameters:  [out]   FramePtrVector& frames          The frames that get filled
    //              [in ]   VmbUint32_t     timeout         The time to wait until one frame got filled
    //
    // Details:     The size of the frame vector determines the number of frames to use.
    //
    // Returns:
    //  - VmbErrorSuccess:          If no error
    //  - VmbErrorInternalFault:    Filling all the frames was not successful.
    //  - VmbErrorBadParameter:     Vector "frames" is empty.
    //
    VmbErrorType AcquireMultipleImages( FramePtrVector &frames, VmbUint32_t timeout );

    //
    // Method:      AcquireMultipleImages()
    //
    // Purpose:     Same as AcquireMultipleImages(FramePtrVector&, VmbUint32_t), but returns the number of frames that were filled completely.
    //
    // Parameters:  [out]   FramePtrVector& frames              The frames that get filled
    //              [in ]   VmbUint32_t     timeout             The time to wait until one frame got filled
    //              [out]   VmbUint32_t&    numFramesCompleted  The number of frames that were filled completely
    //
    // Details:     The size of the frame vector determines the number of frames to use.
    //              On return, "numFramesCompleted" holds the number of frames actually filled.
    //
    // Returns:
    //  - VmbErrorSuccess:      If no error
    //  - VmbErrorBadParameter: Vector "frames" is empty.
    //
    VmbErrorType AcquireMultipleImages( FramePtrVector &frames, VmbUint32_t timeout, VmbUint32_t &numFramesCompleted );

    //
    // Method:      StartContinuousImageAcquisition()
    //
    // Purpose:     Starts streaming and allocates the needed frames
    //
    // Parameters:  [in ]   int                         bufferCount    The number of frames to use
    //              [out]   const IFrameObserverPtr&    pObserver      The observer to use on arrival of new frames
    //
    // Returns:
    //
    //  - VmbErrorSuccess:        If no error
    //  - VmbErrorDeviceNotOpen:  The camera has not been opened before
    //  - VmbErrorApiNotStarted:  VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:      The given handle is not valid
    //  - VmbErrorInvalidAccess:  Operation is invalid with the current access mode
    //
    IMEXPORT VmbErrorType StartContinuousImageAcquisition( int bufferCount, const IFrameObserverPtr &pObserver );

    //
    // Method:      StopContinuousImageAcquisition()
    //
    // Purpose:     Stops streaming and deallocates the needed frames
    //
    IMEXPORT VmbErrorType StopContinuousImageAcquisition();

    //
    // Method:      AnnounceFrame()
    //
    // Purpose:     Announces a frame to the API that may be queued for frame capturing later.
    //
    // Parameters:
    //
    //  [in ]  const FramePtr&    pFrame         Shared pointer to a frame to announce
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:     The given handle is not valid
    //  - VmbErrorBadParameter:  "pFrame" is NULL.
    //  - VmbErrorStructSize:    The given struct size is not valid for this version of the API
    //
    // Details:     Allows some preparation for frames like DMA preparation depending on the transport layer.
    //              The order in which the frames are announced is not taken in consideration by the API.
    //
    IMEXPORT VmbErrorType AnnounceFrame( const FramePtr &pFrame );
    
    //
    // Method:      RevokeFrame()
    //
    // Purpose:     Revoke a frame from the API.
    //
    // Parameters:
    //
    //  [in ]  const FramePtr&    pFrame         Shared pointer to a frame that is to be removed from the list of announced frames
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:     The given frame pointer is not valid
    //  - VmbErrorBadParameter:  "pFrame" is NULL.
    //  - VmbErrorStructSize:    The given struct size is not valid for this version of the API
    //
    // Details:    The referenced frame is removed from the pool of frames for capturing images.
    //
    IMEXPORT VmbErrorType RevokeFrame( const FramePtr &pFrame );

    //
    // Method:      RevokeAllFrames()
    //
    // Purpose:     Revoke all frames assigned to this certain camera.
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:     The given handle is not valid
    //
    IMEXPORT VmbErrorType RevokeAllFrames();
    
    //
    // Method:      QueueFrame()
    //
    // Purpose:     Queues a frame that may be filled during frame capturing.
    //
    // Parameters:
    //
    //  [in ]  const FramePtr&    pFrame    A shared pointer to a frame
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:     The given frame is not valid
    //  - VmbErrorBadParameter:  "pFrame" is NULL.
    //  - VmbErrorStructSize:    The given struct size is not valid for this version of the API
    //  - VmbErrorInvalidCall:   StopContinuousImageAcquisition is currently running in another thread
    //
    // Details:     The given frame is put into a queue that will be filled sequentially.
    //              The order in which the frames are filled is determined by the order in which they are queued.
    //              If the frame was announced with AnnounceFrame() before, the application
    //              has to ensure that the frame is also revoked by calling RevokeFrame() or RevokeAll()
    //              when cleaning up.
    //
    IMEXPORT VmbErrorType QueueFrame( const FramePtr &pFrame );

    //
    // Method:      FlushQueue()
    //
    // Purpose:     Flushes the capture queue.
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:     The given handle is not valid
    //
    // Details:     All the currently queued frames will be returned to the user, leaving no frames in the input queue.
    //              After this call, no frame notification will occur until frames are queued again.
    //
    IMEXPORT VmbErrorType FlushQueue();
    
    //
    // Method:      StartCapture()
    //
    // Purpose:     Prepare the API for incoming frames from this camera.
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:     The given handle is not valid
    //  - VmbErrorDeviceNotOpen: Camera was not opened for usage
    //  - VmbErrorInvalidAccess: Operation is invalid with the current access mode
    //
    IMEXPORT VmbErrorType StartCapture();

    //
    // Method:      EndCapture()
    //
    // Purpose:     Stop the API from being able to receive frames from this camera.
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:     The given handle is not valid
    //
    // Details:     Consequences of VmbCaptureEnd():
    //                  - The frame queue is flushed
    //                  - The frame callback will not be called any more
    //
    IMEXPORT VmbErrorType EndCapture();

    //
    // Method:      SaveCameraSettings()
    //
    // Purpose:     Saves the current camera setup to an XML file
    //
    // Parameters:
    //
    //  [in ]   std::string                     pStrFileName    xml file name
    //  [in ]   VmbFeaturePersistSettings_t*    pSettings       pointer to settings struct
    //
    // Returns:
    //
    //  - VmbErrorSuccess:          If no error
    //  - VmbErrorApiNotStarted:    VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:        The given handle is not valid
    //  - VmbErrorInternalFault:    When something unexpected happens in VimbaC function
    //  - VmbErrorOther:            Every other failure in load/save settings implementation class
    //
    VmbErrorType SaveCameraSettings( std::string fileName, VmbFeaturePersistSettings_t *pSettings = 0 ) const;

    //
    // Method:      LoadCameraSettings()
    //
    // Purpose:     Loads the current camera setup from an XML file into the camera
    //
    // Parameters:
    //
    //  [in] std::string                    pStrFileName    xml file name
    //  [in] VmbFeaturePersistSettings_t*   pSettings       pointer to settings struct
    //
    // Returns:
    //
    //  - VmbErrorSuccess:          If no error
    //  - VmbErrorApiNotStarted:    VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:        The given handle is not valid
    //  - VmbErrorInternalFault:    When something unexpected happens in VimbaC function
    //  - VmbErrorOther:            Every other failure in load/save settings implementation class
    //
    VmbErrorType LoadCameraSettings( std::string fileName, VmbFeaturePersistSettings_t *pSettings = 0 ) const;

    //
    // Method:      LoadSaveSettingsSetup()
    //
    // Purpose:     Sets Load/Save settings behaviour (alternative to settings struct)
    //
    // Parameters:
    //
    //  [in] VmbFeaturePersist_t  persistType    determines which feature shall be considered during load/save settings
    //  [in] VmbUint32_t          maxIterations  determines how many 'tries' during loading feature values shall be performed
    //  [in] VmbUint32_t          loggingLevel   determines level of detail for load/save settings logging
    //
    IMEXPORT void LoadSaveSettingsSetup( VmbFeaturePersist_t persistType, VmbUint32_t maxIterations, VmbUint32_t loggingLevel );

  private:
    // Default ctor
    Camera();

    // Copy ctor
    Camera ( const Camera& );

    // Assignment operator
    Camera& operator=( const Camera& );

    struct Impl;
    Impl *m_pImpl;

    // Array functions to pass data across DLL boundaries
    IMEXPORT VmbErrorType GetID( char * const pID, VmbUint32_t &length ) const;
    IMEXPORT VmbErrorType GetName( char * const pName, VmbUint32_t &length ) const;
    IMEXPORT VmbErrorType GetModel( char * const pModelName, VmbUint32_t &length ) const;
    IMEXPORT VmbErrorType GetSerialNumber( char * const pSerial, VmbUint32_t &length ) const;
    IMEXPORT VmbErrorType GetInterfaceID( char * const pInterfaceID, VmbUint32_t &length ) const;
    IMEXPORT VmbErrorType AcquireMultipleImages( FramePtr *pFrames, VmbUint32_t size, VmbUint32_t nTimeout, VmbUint32_t *pNumFramesCompleted );
    IMEXPORT virtual VmbErrorType ReadRegisters( const VmbUint64_t *pAddressArray, VmbUint32_t addressSize, VmbUint64_t *pDataArray, VmbUint32_t *pCompletedReads ) const;
    IMEXPORT virtual VmbErrorType WriteRegisters( const VmbUint64_t *pAddressArray, VmbUint32_t addressSize, const VmbUint64_t *pDataArray, VmbUint32_t *pCompletedWrites );
    IMEXPORT virtual VmbErrorType ReadMemory( VmbUint64_t address, VmbUchar_t *pBuffer, VmbUint32_t bufferSize, VmbUint32_t *pSizeComplete ) const;
    IMEXPORT virtual VmbErrorType WriteMemory( VmbUint64_t address, const VmbUchar_t *pBuffer, VmbUint32_t bufferSize, VmbUint32_t *pSizeComplete );
    IMEXPORT VmbErrorType SaveCameraSettings( const char * const pStrFileName, VmbFeaturePersistSettings_t *pSettings ) const;
    IMEXPORT VmbErrorType LoadCameraSettings( const char * const pStrFileName, VmbFeaturePersistSettings_t *pSettings ) const;

    VmbFeaturePersist_t m_persistType;
    VmbUint32_t m_maxIterations;
    VmbUint32_t m_loggingLevel;
};

#include <VimbaCPP/Include/Camera.hpp>

}} // namespace AVT::VmbAPI

#endif
