/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        Camera.cpp

  Description: Implementation of class AVT::VmbAPI::Camera.

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
#pragma warning(disable:4996)
#include <sstream>
#pragma warning(default:4996)
#include <cstring>

#include <VimbaCPP/Include/Camera.h>

#include <VimbaCPP/Include/LoggerDefines.h>
#include <VimbaCPP/Source/ConditionHelper.h>
#include <VimbaCPP/Source/FrameImpl.h>
#include <VimbaCPP/Source/FrameHandler.h>
#include <VimbaCPP/Source/Helper.h>
#include <VimbaCPP/Source/MutexGuard.h>

namespace AVT {
namespace VmbAPI {

// Method: GetFeatureValueInt
//
//Purpose: helper function to read integer value from camera.
//
// Parameters:
//
// [in] cam        camera to get integer value from
// [in] name       name of the feature
// [out] val       returns integer value of feature on VmbErrorSuccess
//
VmbErrorType GetFeatureValueInt( Camera&cam,const char* name, VmbInt64_t &val)
{
    if( NULL == name)
    {
        LOG_FREE_TEXT("feature name is NULL");
        return VmbErrorBadParameter;
    }
    FeaturePtr      pFeature;
    VmbErrorType    res         = cam.GetFeatureByName( name, pFeature );
    if ( VmbErrorSuccess != res )
    {
        LOG_FREE_TEXT( std::string("Could not get feature by name for ") + name);
        return res;
    }
    res = SP_ACCESS(pFeature)->GetValue( val );
    if( VmbErrorSuccess != res)
    {
        LOG_FREE_TEXT( std::string("Could not get value of feature ") + name);
    }
    return res;
}
//
// Method: RunFeatureCommand
//
// Purpose: helper to run a command feature for camera.
//
// Parameters:
//
// [in] cam        camera to run command on
// [in] name       command name to run
//
VmbErrorType RunFeatureCommand( Camera&cam,const char* name)
{
    if( NULL == name)
    {
        LOG_FREE_TEXT("feature name is NULL");
        return VmbErrorBadParameter;
    }
    FeaturePtr      pFeature;
    VmbErrorType    res         = cam.GetFeatureByName( name, pFeature );
    if ( VmbErrorSuccess != res )
    {
        LOG_FREE_TEXT( std::string("Could not get feature by name for ") + name);
        return res;
    }
    res = SP_ACCESS(pFeature)->RunCommand();
    if( VmbErrorSuccess != res)
    {
        LOG_FREE_TEXT( std::string("Could not run feature command ") + name);
    }
    return res;
}

// small helper class that keeps track of resources needed for image acquisition
struct AcquireImageHelper
{
private:
    //clean up tasks
    enum tear_down_tasks
    {
        RevokeFrame,
        FlushQueue,
        EndCapture,
        AcquisitionStop,
    };
    typedef std::vector<tear_down_tasks>    task_storage;
    task_storage                            m_Tasks;        // storage for cleanup tasks
    Camera&                                 m_Camera;
    ///get the top most taks and pop it from stack
    tear_down_tasks GetTask()
    {
        tear_down_tasks current_task = m_Tasks.back();
        m_Tasks.pop_back();
        return current_task;
    }
    const AcquireImageHelper& operator=( const AcquireImageHelper &o);
    // Method: SetupFrame
    //
    // Purpose: prepare a frame with given payload size.
    //
    // Parameters:
    // [in,out] pFrame         a frame pointer that can point to Null
    // [in]     payload_size   payload size for frame
    //
    static VmbErrorType SetupFrame(FramePtr &pFrame, VmbInt64_t PayloadSize)
    {
        if( PayloadSize <= 0)
        {
            LOG_FREE_TEXT("payload size has to be larger than 0");
            return VmbErrorBadParameter;
        }
        VmbUint32_t     buffer_size(0);
        VmbErrorType    Result;
        if( ! SP_ISNULL( pFrame) )  // if frame already exists, check its buffer size
        {
            Result = SP_ACCESS( pFrame) ->GetBufferSize(buffer_size);
            if( VmbErrorSuccess != Result)
            {
                LOG_FREE_TEXT("Could not get frame buffer size");
                return Result;
            }
            if( buffer_size >= PayloadSize) // buffer is large enough, no need to create new frame
            {
                return VmbErrorSuccess;
            }
        }
        try
        {
            SP_SET( pFrame, new Frame( PayloadSize));
            if( SP_ISNULL( pFrame) ) // in case we find a not throwing new
            {
                LOG_FREE_TEXT("error allocating frame");
                return VmbErrorResources;
            }
        }
        catch(...)
        {
            LOG_FREE_TEXT("error allocating frame");
            return VmbErrorResources;
        }
        return VmbErrorSuccess;
    }
public:
    // construct helper from camera
    AcquireImageHelper(Camera &Cam)
        : m_Camera( Cam)
    {}
    // destroy will tear all down
    ~AcquireImageHelper()
    {
        TearDown();
    }
    //
    // Method:: AnnounceFrames
    //
    // Purpose: helper to announce a list of frames to the camera.
    //
    // Parameters:
    //
    // [in]        Camera              Camera to announce the frames too
    // [in,out]    pFrames             storage for frame pointer, if they are NULL or have no sufficient space the frames will be created
    // [in]        nFrameCount         number of frame pointers in pFrames
    // [in]        nPayloadSize        payload size for one frame
    // [out]       nFramesAnnounced    returns number of successful announced frames
    // Returns:
    //
    // the first error that occurred or VmbErrorSuccess if non occurred 
    // Details: note the function will try to construct and announce nFrameCount frames t o the camera, even if some of them can not be created or announced, only if nFramesAnnounced == 0 the function was unsuccessful
    //
    static VmbErrorType AnnounceFrames(Camera &Camera, FramePtr *pFrames, VmbUint32_t nFrameCount, VmbInt64_t nPayloadSize, VmbUint32_t &nFramesAnnounced)
    {
        VmbErrorType    Result  = VmbErrorSuccess;
        nFramesAnnounced        = 0;
        for( VmbUint32_t FrameNumber= 0; FrameNumber < nFrameCount; ++FrameNumber)
        {
            VmbErrorType LocalResult = SetupFrame( pFrames[ FrameNumber ], nPayloadSize);         //< try to init frame
            if( VmbErrorSuccess == LocalResult)
            {
                LocalResult = Camera.AnnounceFrame( pFrames[ FrameNumber] );       //< announce frame if successful initialized
                if ( VmbErrorSuccess == LocalResult )
                {
                    ++nFramesAnnounced;
                }
                else
                {
                    std::stringstream strMsg("Could only successfully announce ");
                    strMsg << nFramesAnnounced << " of " <<  nFrameCount  << " frames. Will continue with queuing those.";
                    LOG_FREE_TEXT( strMsg.str() );
                }
            }
            if( VmbErrorSuccess == Result )
            {
                Result = LocalResult;
            }
        }
        return Result;
    }
    //
    // Method: AnnounceFrames
    //
    // Purpose: announce a FramePtrVector to the camera.
    //
    // Parameters:
    // [in]        Camera          camera to announce the frames to
    // [in,out]    Frames          vector of frame pointers that will contain the announced frames on return, can be empty on input
    // [in]        nBufferCount    number of frames to announce, if nBufferCount > Frames.size() on return, some frames could not be announced
    // [in]        nPayloadSize    frame payload size
    // [in]        Observer        observer to attach to frames
    //
    static VmbErrorType AnnounceFrames(Camera &Camera, FramePtrVector &Frames, VmbUint32_t nBufferCount, VmbInt64_t nPayloadSize, const IFrameObserverPtr& Observer)
    {
        try
        {
            Frames.reserve( nBufferCount);
        }
        catch(...)
        {
            LOG_FREE_TEXT("could not allocate frames");
            return VmbErrorResources;
        }
        VmbErrorType Result = VmbErrorSuccess;
        for( VmbUint32_t i=0; i < nBufferCount; ++i)
        {
            FramePtr tmpFrame;
            VmbErrorType LocalResult = SetupFrame( tmpFrame, nPayloadSize );
            if( ! SP_ISNULL( tmpFrame) )
            {
                LocalResult = SP_ACCESS( tmpFrame)->RegisterObserver( Observer );
                if( VmbErrorSuccess == LocalResult )
                {
                    LocalResult = Camera.AnnounceFrame( tmpFrame);
                    if( VmbErrorSuccess == LocalResult )
                    {
                        Frames.push_back( tmpFrame );
                    }
                    else
                    {
                        LOG_FREE_TEXT("could not announce frame");
                    }
                }
                else
                {
                    LOG_FREE_TEXT("could not register frame observer");
                }
            }
            else
            {
                LOG_FREE_TEXT("could not allocate frame");
            }
            if( VmbErrorSuccess == Result)
            {
                Result = LocalResult;
            }
        }
        return Result;
    }
    //
    // Method: Prepare
    //
    // Purpose: prepare image grab for single image.
    //
    // Parameters:
    //
    // [in,out]    pFrame          frame to hold the image
    // [in]        PayloadSize     frame payload size
    //
    VmbErrorType Prepare(FramePtr &pFrame, VmbInt64_t PayloadSize)
    {
        VmbErrorType res;
        res = SetupFrame( pFrame, PayloadSize);                     // init frame if necessary
        if ( VmbErrorSuccess != res )
        {
            LOG_FREE_TEXT("Could not create frame");
            return res;
        }
        res = m_Camera.AnnounceFrame( pFrame );                     // announce frame to camera
        if ( VmbErrorSuccess != res )
        {
            LOG_FREE_TEXT("Could not Announce frame");
            return res;
        }
        m_Tasks.push_back( RevokeFrame);                            // if successful announced we need to revoke frames
        res = m_Camera.StartCapture();                              // start capture logic
        if ( VmbErrorSuccess != res )
        {
            LOG_FREE_TEXT( "Could not Start Capture" );
            return res;
        }
        m_Tasks.push_back( EndCapture);                             // if capture logic is started we need end capture task
        res = m_Camera.QueueFrame( pFrame );                        // queue frame in processing logic
        if ( VmbErrorSuccess != res )
        {
            LOG_FREE_TEXT( "Could not queue frame");
            return res;
        }
        m_Tasks.pop_back();
        m_Tasks.push_back( FlushQueue);                             // if frame queued we need flush queue task
        m_Tasks.push_back( EndCapture);
        FeaturePtr pFeature;
        res = RunFeatureCommand( m_Camera, "AcquisitionStart" );    // start acquisition
        if ( VmbErrorSuccess != res )
        {
            LOG_FREE_TEXT("Could not run command AcquisitionStart");
            return res;
        }
        m_Tasks.push_back( AcquisitionStop);
        return res;
    }
    // Method: Prepare
    //
    // Purpose: prepare image acquisition for multiple frames.
    //
    // Parameters:
    //
    // [in,out]     pFrames         non NULL pointer to field of frame pointers (can point to NULL) that hold the captured images
    // [in]         nFrameCount     number of frames in vector
    // [in]         nPayLoadSize    payload size
    // [out]        nFramesQueued   returns number of successful queued images
    //
    VmbErrorType Prepare(FramePtr *pFrames, VmbUint32_t nFrameCount, VmbInt64_t nPayloadSize, VmbUint32_t &nFramesQueued )
    {
        if( NULL == pFrames || 0 == nFrameCount)                            // sanity check
        {
            return VmbErrorBadParameter;
        }
        nFramesQueued = 0;
        VmbErrorType    Result          = VmbErrorSuccess;
        VmbUint32_t     FramesAnnounced = 0;
        Result = AnnounceFrames( m_Camera, pFrames, nFrameCount, nPayloadSize, FramesAnnounced);
        if( 0 == FramesAnnounced)
        {
            return Result;
        }
        m_Tasks.push_back( RevokeFrame);                                    // add cleanup task for announced frames
        Result = m_Camera.StartCapture();                                   // start capture logic
        if ( VmbErrorSuccess != Result)
        {
            LOG_FREE_TEXT( "Could not Start Capture" );
            return Result;
        }
        m_Tasks.push_back( EndCapture);                                     // add cleanup task to end capture
        for( VmbUint32_t FrameNumber = 0; FrameNumber < FramesAnnounced; ++FrameNumber)
        {
            Result = m_Camera.QueueFrame( pFrames[ FrameNumber ] );         // try queuing frame
            if ( VmbErrorSuccess != Result )
            {
                std::stringstream strMsg("Could only successfully queue ");
                strMsg << nFramesQueued << " of " << nFrameCount << " frames. Will continue with filling those.";
                LOG_FREE_TEXT( strMsg.str() );
                break;
            }
            else
            {
                ++nFramesQueued;
            }
        }
        if( 0 == nFramesQueued) // we cannot capture anything, there are no frames queued
        {
            return Result;
        }
        m_Tasks.pop_back();
        m_Tasks.push_back( FlushQueue);                         // if any frame was queued we need a cleanup task
        m_Tasks.push_back( EndCapture);
        FeaturePtr pFeature;
        Result = RunFeatureCommand( m_Camera, "AcquisitionStart" ); // start acquisition logic
        if ( VmbErrorSuccess != Result )
        {
            LOG_FREE_TEXT("Could not run command AcquisitionStart");
            return Result;
        }
        m_Tasks.push_back( AcquisitionStop);
        return Result;
    }
    //
    // Method: TearDown
    //
    // Purpose: free all acquired resources.
    //
    VmbErrorType TearDown()
    {
        VmbErrorType res = VmbErrorSuccess;
        while( ! m_Tasks.empty() )
        {
            VmbErrorType local_result = VmbErrorSuccess;
            switch( GetTask() )
            {
            case AcquisitionStop:
                    local_result = RunFeatureCommand(m_Camera, "AcquisitionStop");
                    if( VmbErrorSuccess != local_result)
                    {
                        LOG_FREE_TEXT("Could not run command AquireStop");
                    }
                    break;
            case EndCapture:
                    local_result = m_Camera.EndCapture();
                    if( VmbErrorSuccess != local_result)
                    {
                        LOG_FREE_TEXT("Could Not run EndCapture");
                    }
                    break;
            case FlushQueue:
                    local_result = m_Camera.FlushQueue();
                    if( VmbErrorSuccess != local_result)
                    {
                        LOG_FREE_TEXT("Could not run Flush Queue command");
                    }
                    break;
            case RevokeFrame:
                    local_result = m_Camera.RevokeAllFrames();
                    if( VmbErrorSuccess != local_result)
                    {
                        LOG_FREE_TEXT("Could Not Run Revoke Frames command");
                    }
                    break;
            }
            if( VmbErrorSuccess == res)
                res = local_result;
        }
        return res;
    }
};


struct Camera::Impl
{
    // Copy of camera infos
    struct CameraInfo
    {
        std::string     cameraIdString;             // Unique identifier for each camera
        std::string     cameraIdStringGigE;         // GigE cameras can also be opened by IP or MAC address
        std::string     cameraName;                 // Name of the camera
        std::string     modelName;                  // Model name
        std::string     serialString;               // Serial number
        std::string     interfaceIdString;          // Unique value for each interface or bus
    } m_cameraInfo;

    VmbInterfaceType m_eInterfaceType;              // The type of the interface the camera is connected to

    LockableVector<FrameHandlerPtr> m_frameHandlers;
    ConditionHelper                 m_conditionHelper;

    MutexPtr                        m_pQueueFrameMutex;
    bool                            m_bAllowQueueFrame;

    VmbFeaturePersist_t             m_persistType;
    VmbInt32_t                      m_maxIterations;
    VmbInt32_t                      m_loggingLevel;

    VmbErrorType AppendFrameToVector( const FramePtr &frame );
};

Camera::Camera()
{
    // No default ctor
}

Camera::Camera( const Camera& )
{
    // No copy ctor
}

Camera& Camera::operator=( const Camera& )
{
    // No assignment operator
    return *this;
}

Camera::Camera( const char *pID,
                const char *pName,
                const char *pModel,
                const char *pSerialNumber,
                const char *pInterfaceID,
                VmbInterfaceType eInterfaceType )
    :   m_pImpl( new Impl() )
{
    m_pImpl->m_cameraInfo.cameraIdString.assign( pID ? pID : "" );
    // TODO: Remove this with interface change
    const char* pIDGigE = strstr( pID, AVT_IP_OR_MAC_ADDRESS);
    if ( pIDGigE )
    {
        m_pImpl->m_cameraInfo.cameraIdStringGigE.assign( pIDGigE );
        m_pImpl->m_cameraInfo.cameraIdStringGigE.erase( 0, strlen( AVT_IP_OR_MAC_ADDRESS ));
        m_pImpl->m_cameraInfo.cameraIdString.erase( m_pImpl->m_cameraInfo.cameraIdString.find( AVT_IP_OR_MAC_ADDRESS ), std::string::npos );
    }
    m_pImpl->m_cameraInfo.cameraName.assign( pName ? pName : "" );
    m_pImpl->m_cameraInfo.interfaceIdString.assign( pInterfaceID ? pInterfaceID : "" );
    m_pImpl->m_cameraInfo.modelName.assign( pModel ? pModel : "" );
    m_pImpl->m_cameraInfo.serialString.assign( pSerialNumber ? pSerialNumber : "" );
    m_pImpl->m_eInterfaceType = eInterfaceType;
    m_pImpl->m_bAllowQueueFrame = true;
    SP_SET( m_pImpl->m_pQueueFrameMutex, new Mutex );
    m_pImpl->m_persistType = -1;
    m_pImpl->m_maxIterations = -1;
    m_pImpl->m_loggingLevel = -1;
}

Camera::~Camera()
{
    Close();

    delete m_pImpl;
}

VmbErrorType Camera::Open( VmbAccessModeType eAccessMode )
{
    VmbError_t res;
    VmbHandle_t hHandle;

    if ( false == m_pImpl->m_cameraInfo.cameraIdStringGigE.empty() )
    {
        res = VmbCameraOpen( m_pImpl->m_cameraInfo.cameraIdStringGigE.c_str(), (VmbAccessMode_t)eAccessMode, &hHandle );        
    }
    else
    {
        res = VmbCameraOpen( m_pImpl->m_cameraInfo.cameraIdString.c_str(), (VmbAccessMode_t)eAccessMode, &hHandle );
    }

    if ( VmbErrorSuccess == res )
    {
        SetHandle( hHandle );
    }
    
    return (VmbErrorType)res;
}

VmbErrorType Camera::Close()
{
    VmbError_t res = VmbErrorSuccess;

    if ( NULL != GetHandle() )
    {
        if (    0 < m_pImpl->m_frameHandlers.Vector.size()
             && (   VmbErrorSuccess != EndCapture()
                 || VmbErrorSuccess != FlushQueue()
                 || VmbErrorSuccess != RevokeAllFrames()) )
        {
            // Do some logging
            LOG_FREE_TEXT( "Could not successfully revoke all frames")
        }

        Reset();

        res = VmbCameraClose( GetHandle() );

        RevokeHandle();
    }

    return (VmbErrorType)res;
}

VmbErrorType Camera::GetID( char * const pStrID, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrID )
    {
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.cameraIdString.length();
        res = VmbErrorSuccess;
    }
    else if ( m_pImpl->m_cameraInfo.cameraIdString.length() <= rnLength )
    {
        std::copy( m_pImpl->m_cameraInfo.cameraIdString.begin(), m_pImpl->m_cameraInfo.cameraIdString.end(), pStrID );
        pStrID[m_pImpl->m_cameraInfo.cameraIdString.length()] = '\0';
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.cameraIdString.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType Camera::GetName( char * const pStrName, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrName )
    {
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.cameraName.length();
        res = VmbErrorSuccess;
    }
    else if ( m_pImpl->m_cameraInfo.cameraName.length() <= rnLength )
    {
        std::copy( m_pImpl->m_cameraInfo.cameraName.begin(), m_pImpl->m_cameraInfo.cameraName.end(), pStrName );
        pStrName[m_pImpl->m_cameraInfo.cameraName.length()] = '\0';
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.cameraName.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType Camera::GetModel( char * const pStrModel, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrModel )
    {
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.modelName.length();
        res = VmbErrorSuccess;
    }
    else if ( m_pImpl->m_cameraInfo.modelName.length() <= rnLength )
    {
        std::copy( m_pImpl->m_cameraInfo.modelName.begin(), m_pImpl->m_cameraInfo.modelName.end(), pStrModel );
        pStrModel[m_pImpl->m_cameraInfo.modelName.length()] = '\0';
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.modelName.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType Camera::GetSerialNumber( char * const pStrSerial, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrSerial )
    {
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.serialString.length();
        res = VmbErrorSuccess;
    }
    else if ( m_pImpl->m_cameraInfo.serialString.length() <= rnLength )
    {
        std::copy( m_pImpl->m_cameraInfo.serialString.begin(), m_pImpl->m_cameraInfo.serialString.end(), pStrSerial );
        pStrSerial[m_pImpl->m_cameraInfo.serialString.length()] = '\0';
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.serialString.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType Camera::GetInterfaceID( char * const pStrInterfaceID, VmbUint32_t &rnLength ) const
{    
    VmbErrorType res;

    if ( NULL == pStrInterfaceID )
    {
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.interfaceIdString.length();
        res = VmbErrorSuccess;
    }
    else if ( m_pImpl->m_cameraInfo.interfaceIdString.length() <= rnLength )
    {
        std::copy( m_pImpl->m_cameraInfo.interfaceIdString.begin(), m_pImpl->m_cameraInfo.interfaceIdString.end(), pStrInterfaceID );
        pStrInterfaceID[m_pImpl->m_cameraInfo.interfaceIdString.length()] = '\0';
        rnLength = (VmbUint32_t)m_pImpl->m_cameraInfo.interfaceIdString.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType Camera::GetInterfaceType( VmbInterfaceType &reInterfaceType ) const
{
    reInterfaceType = m_pImpl->m_eInterfaceType;

    return VmbErrorSuccess;
}

VmbErrorType Camera::GetPermittedAccess( VmbAccessModeType &rePermittedAccess ) const
{
    VmbError_t res;
    VmbCameraInfo_t info;

    if ( false == m_pImpl->m_cameraInfo.cameraIdStringGigE.empty() )
    {
        res = VmbCameraInfoQuery( m_pImpl->m_cameraInfo.cameraIdStringGigE.c_str(), &info, sizeof( VmbCameraInfo_t ));
    }
    else
    {
        res = VmbCameraInfoQuery( m_pImpl->m_cameraInfo.cameraIdString.c_str(), &info, sizeof( VmbCameraInfo_t ));
    }

    if ( VmbErrorSuccess == res )
    {
        rePermittedAccess = (VmbAccessModeType)info.permittedAccess;
    }

    return (VmbErrorType)res;
}

VmbErrorType Camera::ReadRegisters( const VmbUint64_t *pAddressArray, VmbUint32_t nAddressSize, VmbUint64_t *pDataArray, VmbUint32_t *pCompletedReads ) const
{
    return static_cast<VmbErrorType>( VmbRegistersRead( GetHandle(), nAddressSize, pAddressArray, pDataArray, pCompletedReads ) );
}

VmbErrorType Camera::WriteRegisters( const VmbUint64_t *pAddressArray, VmbUint32_t nAddressSize, const VmbUint64_t *pDataArray, VmbUint32_t *pCompletedWrites )
{
    return static_cast<VmbErrorType>( VmbRegistersWrite( GetHandle(), nAddressSize, pAddressArray, pDataArray, pCompletedWrites ) );
}

VmbErrorType Camera::ReadMemory( const VmbUint64_t address, VmbUchar_t *pBuffer, VmbUint32_t nBufferSize, VmbUint32_t *pSizeComplete ) const
{
    return static_cast<VmbErrorType>( VmbMemoryRead( GetHandle(), address, nBufferSize, (char*)pBuffer, pSizeComplete ) );
}

VmbErrorType Camera::WriteMemory( const VmbUint64_t address, const VmbUchar_t *pBuffer, VmbUint32_t nBufferSize, VmbUint32_t *pSizeComplete )
{
    return static_cast<VmbErrorType>( VmbMemoryWrite( GetHandle(), address, nBufferSize, (char *)pBuffer, pSizeComplete ) );
}

//Get one image synchronously.
VmbErrorType Camera::AcquireSingleImage( FramePtr &rFrame, VmbUint32_t nTimeout )
{
    VmbErrorType    res;
    VmbInt64_t      PayloadSize;
    FeaturePtr      pFeature;

    res = GetFeatureValueInt( *this, "PayloadSize", PayloadSize );
    if ( VmbErrorSuccess == res )
    {
        AcquireImageHelper AcquireHelper( *this );
        res = AcquireHelper.Prepare( rFrame, PayloadSize );
        if ( VmbErrorSuccess == res )
        {
            res = (VmbErrorType)VmbCaptureFrameWait( GetHandle(), &(SP_ACCESS( rFrame )->m_pImpl->m_frame), nTimeout );
            if ( VmbErrorSuccess != res )
            {
                LOG_FREE_TEXT( "Could not acquire single image." )
            }
        }
        else
        {
            LOG_FREE_TEXT( "Preparing image acquisition failed." );
        }
        VmbErrorType local_result = AcquireHelper.TearDown();
        if( VmbErrorSuccess != local_result )
        {
            LOG_FREE_TEXT( "Tear down capture logic failed." )
            if( VmbErrorSuccess == res)
            {
                res = local_result;
            }
        }
    }
    else
    {
        LOG_FREE_TEXT( "Could not get payload size" );
    }

    return res;
}

VmbErrorType Camera::AcquireMultipleImages( FramePtr *pFrames, VmbUint32_t nSize, VmbUint32_t nTimeout, VmbUint32_t *pNumFramesCompleted )
{
    VmbErrorType res = VmbErrorBadParameter;

    if (    NULL == pFrames
         || 0 == nSize )
    {
        return res;
    }

    if ( NULL != pNumFramesCompleted )
    {
        *pNumFramesCompleted = 0;
    }

    VmbInt64_t nPayloadSize;
    FeaturePtr pFeature;

    res = GetFeatureValueInt( *this, "PayloadSize", nPayloadSize );
    if ( VmbErrorSuccess == res )
    {
        AcquireImageHelper AquireHelper( *this );
        VmbUint32_t nFramesQueued = 0;
        res = AquireHelper.Prepare( pFrames, nSize, nPayloadSize, nFramesQueued);

        if ( VmbErrorSuccess == res )
        {
            for ( VmbUint32_t nFrameCount = 0; nFrameCount <nFramesQueued; ++ nFrameCount )
            {
                res = (VmbErrorType)VmbCaptureFrameWait( GetHandle(), &(SP_ACCESS( pFrames[nFrameCount] )->m_pImpl->m_frame), nTimeout );
                if ( VmbErrorSuccess != res )
                {
                    std::stringstream strMsg("Could only successfully fill ");
                    strMsg << nFrameCount-1 << " of " << nSize << " frames. Will stop acquisition now.";
                    LOG_FREE_TEXT( strMsg.str() );
                    break;
                }
                else if ( NULL !=  pNumFramesCompleted )
                {
                    ++(*pNumFramesCompleted);
                }
            }
            VmbErrorType local_res = AquireHelper.TearDown();
            if( VmbErrorSuccess == res)
            {
                res = local_res;
            }
        }
        else
        {
            LOG_FREE_TEXT( "Could not start capture" )
        }
    }
    else
    {
        LOG_FREE_TEXT( "Could not get feature PayloadSize");
    }

    return res;
}

VmbErrorType Camera::StartContinuousImageAcquisition( int nBufferCount, const IFrameObserverPtr &rObserver )
{
    VmbErrorType        res;
    FramePtrVector      Frames;
    VmbInt64_t          nPayloadSize;

    res = GetFeatureValueInt(*this,"PayloadSize", nPayloadSize );
    if ( VmbErrorSuccess == res )
    {
        res = AcquireImageHelper::AnnounceFrames( *this, Frames, nBufferCount, nPayloadSize, rObserver );
        if( Frames.empty() )
        {
            return res;
        }
        res = StartCapture();
        if ( VmbErrorSuccess == res )
        {
            VmbUint32_t FramesQueued = 0;
            for (   size_t FrameNumber = 0; FrameNumber < Frames.size(); ++ FrameNumber )
            {
                VmbErrorType LocalResult =  QueueFrame( Frames[ FrameNumber] );
                if ( VmbErrorSuccess == LocalResult)
                {
                    ++FramesQueued;
                }
                else
                {
                    LOG_FREE_TEXT( "Could not queue frame" )
                }
                if( VmbErrorSuccess == res)
                {
                    res = LocalResult;
                }
            }
            if( 0 != FramesQueued)
            {
                res = RunFeatureCommand(*this, "AcquisitionStart" );
                if ( VmbErrorSuccess != res )
                {
                    EndCapture();
                    FlushQueue();
                    RevokeAllFrames();
                    LOG_FREE_TEXT( "Could not start acquisition" )
                    return res;
                }

            }
            else
            {
                EndCapture();
                RevokeAllFrames();
                LOG_FREE_TEXT( "Could not queue frames" )
                return res;
            }

        }
        else
        {
            RevokeAllFrames();
            LOG_FREE_TEXT( "Could not start capturing" )
        }
    }
    else
    {
        LOG_FREE_TEXT( "Could not get feature PayloadSize" )
    }

    return res;
}

VmbErrorType Camera::StopContinuousImageAcquisition()
{
    VmbErrorType    res;
    FeaturePtr      pFeature;

    // Prevent queuing of new frames while stopping
    {
        MutexGuard guard( m_pImpl->m_pQueueFrameMutex );
        m_pImpl->m_bAllowQueueFrame = false;
    }

    res = RunFeatureCommand( *this, "AcquisitionStop" );
    if ( VmbErrorSuccess != res )
    {
        LOG_FREE_TEXT( "Could not run feature AcquisitionStop" )
    }

    res = EndCapture();
    if ( VmbErrorSuccess == res )
    {
        res = FlushQueue();
        if( VmbErrorSuccess != res)
        {
            LOG_FREE_TEXT( "Could not flush queue" )
        }
        res = RevokeAllFrames();
        if ( VmbErrorSuccess != res )
        {
            LOG_FREE_TEXT( "Could not revoke frames" )
        }
    }
    else
    {
        LOG_FREE_TEXT("Could not stop capture, unable to revoke frames")
    }

    {
        MutexGuard guard( m_pImpl->m_pQueueFrameMutex );
        m_pImpl->m_bAllowQueueFrame = true;
    }

    return res;
}

VmbErrorType Camera::AnnounceFrame( const FramePtr &frame ) 
{
    if ( SP_ISNULL( frame ))
    {
        return VmbErrorBadParameter;
    }

    if (    true == SP_ACCESS( frame )->m_pImpl->m_bAlreadyAnnounced
         || true == SP_ACCESS( frame )->m_pImpl->m_bAlreadyQueued )
    {
        return VmbErrorInvalidCall;
    }

    VmbError_t res = VmbFrameAnnounce( GetHandle(), &(SP_ACCESS( frame )->m_pImpl->m_frame), sizeof SP_ACCESS( frame )->m_pImpl->m_frame );
    
    if ( VmbErrorSuccess == res )
    {
        // Begin write lock frame handler list
        if ( true == m_pImpl->m_conditionHelper.EnterWriteLock( m_pImpl->m_frameHandlers ))
        {
            res = m_pImpl->AppendFrameToVector( frame ) ;
            if( VmbErrorSuccess == res )
            {
                SP_ACCESS( frame )->m_pImpl->m_bAlreadyAnnounced = true;
            }
            else
            {
                LOG_FREE_TEXT("could not append frame to internal vector");
            }
            // End write lock frame handler list
            m_pImpl->m_conditionHelper.ExitWriteLock( m_pImpl->m_frameHandlers );
        }
        else
        {
            LOG_FREE_TEXT( "Could not lock announced frame queue for appending frame." );
            res = VmbErrorResources;
        }
    }

    return static_cast<VmbErrorType>( res );
}

VmbErrorType Camera::RevokeFrame( const FramePtr &frame )
{
    if ( SP_ISNULL( frame ))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbFrameRevoke( GetHandle(), &(SP_ACCESS( frame )->m_pImpl->m_frame) );

    if ( VmbErrorSuccess == res )
    {
        // Begin (exclusive) write lock frame handler list
        if ( true == m_pImpl->m_conditionHelper.EnterWriteLock( m_pImpl->m_frameHandlers, true ))
        {
            // Dequeue, revoke and delete frame
            for(    FrameHandlerPtrVector::iterator iter = m_pImpl->m_frameHandlers.Vector.begin();
                    m_pImpl->m_frameHandlers.Vector.end() != iter;)
            {
                // Begin exclusive write lock frame handler
                MutexGuard lockal_lock ( SP_ACCESS((*iter))->Mutex() );
                if ( SP_ISEQUAL( frame, SP_ACCESS(( *iter ))->GetFrame() ))
                {
                    SP_ACCESS( frame )->m_pImpl->m_frame.context[FRAME_HDL] = NULL;
                    SP_ACCESS( frame )->m_pImpl->m_bAlreadyQueued = false;
                    SP_ACCESS( frame )->m_pImpl->m_bAlreadyAnnounced = false;
                    // End exclusive write lock frame handler
                    iter = m_pImpl->m_frameHandlers.Vector.erase( iter );
                    return VmbErrorSuccess;
                }
                else
                {
                    ++iter;
                }
            }

            // End (exclusive) write lock frame handler list
            m_pImpl->m_conditionHelper.ExitWriteLock( m_pImpl->m_frameHandlers );
        }
        else
        {
            LOG_FREE_TEXT( "Could not lock announced frame queue for removing frame." );
            res = VmbErrorResources;
        }
    }
    else
    {
        LOG_FREE_TEXT( "Could not revoke frames" )
    }

    return (VmbErrorType)res;
}

VmbErrorType Camera::RevokeAllFrames() 
{
    VmbError_t res;

    res = VmbFrameRevokeAll( GetHandle() );

    if ( VmbErrorSuccess == res )
    {
        // Begin (exclusive) write lock frame handler list
        if ( true == m_pImpl->m_conditionHelper.EnterWriteLock( m_pImpl->m_frameHandlers, true ))
        {
            // Dequeue, revoke and delete frames
            for (   FrameHandlerPtrVector::iterator iter = m_pImpl->m_frameHandlers.Vector.begin();
                    m_pImpl->m_frameHandlers.Vector.end() != iter;
                    ++iter )
            {
                // Begin exclusive write lock frame handler
                MutexGuard  local_lock( SP_ACCESS((*iter))->Mutex() );
                SP_ACCESS( SP_ACCESS(( *iter ))->GetFrame() )->m_pImpl->m_frame.context[FRAME_HDL] = NULL;
                SP_ACCESS( SP_ACCESS(( *iter ))->GetFrame() )->m_pImpl->m_bAlreadyQueued = false;
                SP_ACCESS (SP_ACCESS(( *iter ))->GetFrame() )->m_pImpl->m_bAlreadyAnnounced = false;
                // End exclusive write lock frame handler
            }

            m_pImpl->m_frameHandlers.Vector.clear();
            
            // End exclusive write lock frame handler list
            m_pImpl->m_conditionHelper.ExitWriteLock( m_pImpl->m_frameHandlers );
        }
        else
        {
            LOG_FREE_TEXT( "Could not lock frame handler list.")
        }
    }

    return (VmbErrorType)res;
}

VmbErrorType Camera::QueueFrame( const FramePtr &frame )
{
    if ( SP_ISNULL( frame ))
    {
        return VmbErrorBadParameter;
    }

    MutexGuard guard( m_pImpl->m_pQueueFrameMutex );
    if ( false == m_pImpl->m_bAllowQueueFrame )
    {
        LOG_FREE_TEXT( "Queuing of new frames is not possible while flushing and revoking the currently queued frames." );
        return VmbErrorInvalidCall;
    }

    // HINT: The same frame cannot be queued twice (VmbErrorOther)
    VmbError_t res = VmbCaptureFrameQueue( GetHandle(), &(SP_ACCESS( frame )->m_pImpl->m_frame), FrameHandler::FrameDoneCallback );

    if (    VmbErrorSuccess == res
         && false == SP_ACCESS( frame )->m_pImpl->m_bAlreadyQueued )
    {
        if ( false == SP_ACCESS( frame )->m_pImpl->m_bAlreadyAnnounced )
        {
            // Begin write lock frame handler list
            if ( true == m_pImpl->m_conditionHelper.EnterWriteLock( m_pImpl->m_frameHandlers ))
            {
                m_pImpl->AppendFrameToVector( frame );
                SP_ACCESS( frame )->m_pImpl->m_bAlreadyQueued = true;
                
                // End write lock frame handler list
                m_pImpl->m_conditionHelper.ExitWriteLock( m_pImpl->m_frameHandlers );
            }
            else
            {
                LOG_FREE_TEXT( "Could not lock frame queue for appending frame." );
                res = VmbErrorResources;
            }
        }
    }

    return static_cast<VmbErrorType>( res );
}

VmbErrorType Camera::FlushQueue()
{
    VmbError_t res = VmbCaptureQueueFlush( GetHandle() );

    if ( VmbErrorSuccess == res )
    {
        // Begin exclusive write lock frame handler list
        if ( true == m_pImpl->m_conditionHelper.EnterWriteLock( m_pImpl->m_frameHandlers, true ))
        {
            for (   FrameHandlerPtrVector::iterator iter = m_pImpl->m_frameHandlers.Vector.begin();
                    m_pImpl->m_frameHandlers.Vector.end() != iter;)
            {
                // Begin exclusive write lock of every single frame handler
                MutexPtr tmpMutex = SP_ACCESS((*iter))->Mutex();
                SP_ACCESS( tmpMutex )->Lock();
                //SP_ACCESS(( *iter)) ->Mutex()->Lock();
                // Dequeue frame
                SP_ACCESS( SP_ACCESS(( *iter ))->GetFrame() )->m_pImpl->m_bAlreadyQueued = false;
                if ( false == SP_ACCESS( SP_ACCESS(( *iter ))->GetFrame() )->m_pImpl->m_bAlreadyAnnounced )
                {
                    // Delete frame if it was not announced / was revoked before
                    SP_ACCESS( SP_ACCESS(( *iter ))->GetFrame() )->m_pImpl->m_frame.context[FRAME_HDL] = NULL;
                    // End write lock frame handler
                    SP_ACCESS( tmpMutex )->Unlock();
                    iter = m_pImpl->m_frameHandlers.Vector.erase( iter );
                }
                else
                {
                    // End write lock frame handler
                    SP_ACCESS( tmpMutex )->Unlock();
                    ++iter;
                }
            }
            // End write lock frame handler list
            m_pImpl->m_conditionHelper.ExitWriteLock( m_pImpl->m_frameHandlers );
        }
        else
        {
            LOG_FREE_TEXT( "Could not lock frame handler list." )
        }
    }
    else
    {
        LOG_FREE_TEXT( "Could not flush frame queue" )
    }
    
    return static_cast<VmbErrorType>( res );
}

VmbErrorType Camera::StartCapture() 
{
    return static_cast<VmbErrorType>( VmbCaptureStart( GetHandle() ) );
}

VmbErrorType Camera::EndCapture() 
{
    VmbError_t res = VmbCaptureEnd( GetHandle() );

    return static_cast<VmbErrorType>( res );
}

VmbErrorType Camera::Impl::AppendFrameToVector( const FramePtr &rFrame )
{
    try
    {
        FrameHandlerPtr pFH( new FrameHandler( rFrame, SP_ACCESS( rFrame )->m_pImpl->m_pObserver ));
        if( SP_ISNULL( pFH ) )
        {
            return VmbErrorResources;
        }
        SP_ACCESS( rFrame )->m_pImpl->m_frame.context[FRAME_HDL] = SP_ACCESS(pFH);    
        m_frameHandlers.Vector.push_back( pFH );
        return VmbErrorSuccess;
    }
    catch(...)
    {
        return VmbErrorResources;
    }
}

//
// Method:      SaveCameraSettings()
//
// Purpose:     Saves the current camera setup to an XML file
//
// Parameters:
//
//  [in]    pStrFileName    xml file name
//  [in]    pSettings       pointer to settings struct
//
// Returns:
//
//  - VmbErrorSuccess:          If no error
//  - VmbErrorApiNotStarted:    VmbStartup() was not called before the current command
//  - VmbErrorBadHandle:        The given handle is not valid
//  - VmbErrorInternalFault:    When something unexpected happens in VimbaC function
//  - VmbErrorOther:            Every other failure in load/save settings implementation class
//
VmbErrorType Camera::SaveCameraSettings( const char * const pStrFileName, VmbFeaturePersistSettings_t *pSettings ) const
{
    VmbErrorType err = VmbErrorSuccess;

//  parameter check
    if( NULL == pStrFileName )
    {
        return VmbErrorBadParameter;
    }

//  check internal settings struct variables
    VmbBool_t useInternalStruct = true;
    if( false == ((VmbFeaturePersistAll <= m_pImpl->m_persistType) && (VmbFeaturePersistNoLUT >= m_pImpl->m_persistType)) )
    {
        useInternalStruct = false;
    }
    if( false == ((0 < m_pImpl->m_maxIterations) && (10 > m_pImpl->m_maxIterations)) )
    {
        useInternalStruct = false;
    }
    if( false == ((0 < m_pImpl->m_loggingLevel) && (5 > m_pImpl->m_loggingLevel)) )
    {
        useInternalStruct = false;
    }

//  check if internal struct shall be used
    if( VmbBoolTrue == useInternalStruct )
    {
        VmbFeaturePersistSettings_t newSettings;
        newSettings.persistType = m_pImpl->m_persistType;
        newSettings.maxIterations = m_pImpl->m_maxIterations;
        newSettings.loggingLevel = m_pImpl->m_loggingLevel;
        err = (VmbErrorType)VmbCameraSettingsSave( GetHandle(), pStrFileName, &newSettings, sizeof(newSettings) );
    }
    else
    {
        if( NULL == pSettings )
        {
            err = (VmbErrorType)VmbCameraSettingsSave( GetHandle(), pStrFileName, NULL, 0 );
        }
        else
        {
            err = (VmbErrorType)VmbCameraSettingsSave( GetHandle(), pStrFileName, pSettings, sizeof(*pSettings) );
        }
    }



    return err;
}

//
// Method:      LoadCameraSettings()
//
// Purpose:     Loads the current camera setup from an XML file into the camera
//
// Parameters:
//
//  [in]    pStrFileName    xml file name
//  [in]    pSettings       pointer to settings struct
//
// Returns:
//
//  - VmbErrorSuccess:          If no error
//  - VmbErrorApiNotStarted:    VmbStartup() was not called before the current command
//  - VmbErrorBadHandle:        The given handle is not valid
//  - VmbErrorInternalFault:    When something unexpected happens in VimbaC function
//  - VmbErrorOther:            Every other failure in load/save settings implementation class
//
VmbErrorType Camera::LoadCameraSettings( const char * const pStrFileName, VmbFeaturePersistSettings_t *pSettings ) const
{
    VmbErrorType err = VmbErrorSuccess;

//  parameter check
    if( NULL == pStrFileName )
    {
        return VmbErrorBadParameter;
    }

//  check internal settings struct variables
    VmbBool_t useInternalStruct = true;
    if( false == ((VmbFeaturePersistAll <= m_pImpl->m_persistType) && (VmbFeaturePersistNoLUT >= m_pImpl->m_persistType)) )
    {
        useInternalStruct = false;
    }
    if( false == ((0 < m_pImpl->m_maxIterations) && (10 > m_pImpl->m_maxIterations)) )
    {
        useInternalStruct = false;
    }
    if( false == ((0 < m_pImpl->m_loggingLevel) && (5 > m_pImpl->m_loggingLevel)) )
    {
        useInternalStruct = false;
    }

//  check if internal struct shall be used
    if( VmbBoolTrue == useInternalStruct )
    {
        VmbFeaturePersistSettings_t newSettings;
        newSettings.persistType = m_pImpl->m_persistType;
        newSettings.maxIterations = m_pImpl->m_maxIterations;
        newSettings.loggingLevel = m_pImpl->m_loggingLevel;
        err = (VmbErrorType)VmbCameraSettingsLoad( GetHandle(), pStrFileName, &newSettings, sizeof(newSettings) );
    }
    else
    {
        if( NULL == pSettings )
        {
            err = (VmbErrorType)VmbCameraSettingsLoad( GetHandle(), pStrFileName, NULL, 0 );
        }
        else
        {
            err = (VmbErrorType)VmbCameraSettingsLoad( GetHandle(), pStrFileName, pSettings, sizeof(*pSettings) );
        }
    }

    return err;
}

//
// Method:      LoadSaveSettingsSetup()
//
// Purpose:     Sets Load/Save settings behaviour (alternative to settings struct)
//
// Parameters:
//
//  [in]    persistType      determines which feature shall be considered during load/save settings
//  [in]    maxIterations    determines how many 'tries' during loading feature values shall be performed
//  [in]    loggingLevel     determines level of detail for load/save settings logging
//
void Camera::LoadSaveSettingsSetup( VmbFeaturePersist_t persistType, VmbUint32_t maxIterations, VmbUint32_t loggingLevel )
{
    if( true == ((VmbFeaturePersistAll != persistType) && (VmbFeaturePersistStreamable != persistType) && (VmbFeaturePersistNoLUT != persistType)) )
    {
        m_pImpl->m_persistType = VmbFeaturePersistNoLUT;
    }
    else
    {
        m_pImpl->m_persistType = persistType;
    }

    if( false == ((0 < maxIterations) && (6 > maxIterations)) )
    {
        m_pImpl->m_maxIterations = 5;
    }
    else
    {
        m_pImpl->m_maxIterations = maxIterations;
    }

    if( false == ((0 < loggingLevel) && (5 > loggingLevel)) )
    {
        m_pImpl->m_loggingLevel = 4;
    }
    else
    {
        m_pImpl->m_loggingLevel = loggingLevel;
    }
}

}} // namespace AVT::VmbAPI
