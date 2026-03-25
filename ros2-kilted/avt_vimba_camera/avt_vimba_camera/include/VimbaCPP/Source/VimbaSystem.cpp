/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        VimbaSystem.cpp

  Description: Implementation of class AVT::VmbAPI::VimbaSystem.

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

#include <cstring>
#include <algorithm>

#include <VimbaCPP/Include/VimbaSystem.h>

#include <VimbaCPP/Include/SharedPointerDefines.h>
#include <VimbaCPP/Source/ConditionHelper.h>
#include <VimbaCPP/Source/Clock.h>
#include <VimbaCPP/Source/DefaultCameraFactory.h>
#include <VimbaCPP/Source/Helper.h>
#include <VimbaCPP/Source/Version.h>

namespace AVT {
namespace VmbAPI {

typedef std::map<std::string, CameraPtr> CameraPtrMap;
typedef std::map<std::string, InterfacePtr> InterfacePtrMap;

struct VimbaSystem::Impl
{
    // Found cameras and interfaces
    LockableMap<std::string, CameraPtr>             m_cameras;
    ConditionHelper                                 m_camerasConditionHelper;
    LockableMap<std::string, InterfacePtr>          m_interfaces;
    ConditionHelper                                 m_interfacesConditionHelper;
    // Registered observers
    LockableVector<ICameraListObserverPtr>          m_cameraObservers;
    ConditionHelper                                 m_cameraObserversConditionHelper;
    LockableVector<IInterfaceListObserverPtr>       m_interfaceObservers;
    ConditionHelper                                 m_interfaceObserversConditionHelper;
    
    // GigE specifics
    bool                                            m_bGeVDiscoveryAutoOn;
    bool                                            m_bGeVTLPresent;
    // CameraFactory
    ICameraFactoryPtr                               m_pCameraFactory;

    // Logger
    Logger                                          m_pLogger;

    VmbErrorType UpdateCameraList();
    VmbErrorType UpdateInterfaceList();
    void AppendCamToMap( VmbCameraInfo_t camInfo );
    bool IsIPAddress( const char *pStrID );

    VmbErrorType GetInterfaceList( std::vector<VmbInterfaceInfo_t> &interfaceInfos );

    static void VMB_CALL CameraDiscoveryCallback( const VmbHandle_t handle, const char *name, void *context );
    static void VMB_CALL InterfaceDiscoveryCallback( const VmbHandle_t handle, const char *name, void *context );
};

VimbaSystem &VimbaSystem::GetInstance()
{
    return _instance;
}

VmbErrorType VimbaSystem::QueryVersion( VmbVersionInfo_t &rVersion )
{
    rVersion.major = VIMBACPP_VERSION_MAJOR;
    rVersion.minor = VIMBACPP_VERSION_MINOR;
    rVersion.patch = VIMBACPP_VERSION_PATCH;

    return VmbErrorSuccess;
}

VmbErrorType VimbaSystem::Startup()
{
    VmbError_t res = VmbErrorSuccess;
    VmbBool_t gevTL = false;

    res = VmbStartup();
    if ( VmbErrorSuccess == res )
    {
        res = VmbFeatureBoolGet( gVimbaHandle, "GeVTLIsPresent", &gevTL );
        if ( VmbErrorSuccess == res )
        {
            m_pImpl->m_bGeVTLPresent = gevTL;
        }

        SetHandle( gVimbaHandle );
    }

    return (VmbErrorType)res;
}

VmbErrorType VimbaSystem::Shutdown()
{
    m_pImpl->m_bGeVDiscoveryAutoOn = m_pImpl->m_bGeVTLPresent = false;
    
    // Begin exclusive write lock camera observer list
    if ( true == m_pImpl->m_cameraObserversConditionHelper.EnterWriteLock( m_pImpl->m_cameraObservers, true ))
    {
        m_pImpl->m_cameraObservers.Vector.clear();

        // End write lock camera observer list
        m_pImpl->m_cameraObserversConditionHelper.ExitWriteLock( m_pImpl->m_cameraObservers );
    }
        
    // Begin exclusive write lock interface observer list
    if ( true == m_pImpl->m_interfaceObserversConditionHelper.EnterWriteLock( m_pImpl->m_interfaceObservers, true ))
    {
        m_pImpl->m_interfaceObservers.Vector.clear();

        // End write lock interface observer list
        m_pImpl->m_interfaceObserversConditionHelper.ExitWriteLock( m_pImpl->m_interfaceObservers );
    }    

    // Begin exclusive write lock camera list
    if ( true == m_pImpl->m_camerasConditionHelper.EnterWriteLock( m_pImpl->m_cameras, true ))
    {
        for (   CameraPtrMap::iterator iter = m_pImpl->m_cameras.Map.begin();
                m_pImpl->m_cameras.Map.end() != iter;
                ++iter)
        {
            SP_ACCESS( iter->second )->Close();
        }
        m_pImpl->m_cameras.Map.clear();

        // End write lock camera list
        m_pImpl->m_camerasConditionHelper.ExitWriteLock( m_pImpl->m_cameras );
    }    

    // Begin exclusive write lock interface list
    if ( true == m_pImpl->m_interfacesConditionHelper.EnterWriteLock( m_pImpl->m_interfaces, true ))
    {
        for (   InterfacePtrMap::iterator iter = m_pImpl->m_interfaces.Map.begin();
                m_pImpl->m_interfaces.Map.end() != iter;
                ++iter)
        {
            SP_ACCESS( iter->second )->Close();
        }
        m_pImpl->m_interfaces.Map.clear();

        // End write lock interface list
        m_pImpl->m_interfacesConditionHelper.ExitWriteLock( m_pImpl->m_interfaces );
    }    

    VmbShutdown();
    
    return VmbErrorSuccess;
}

VmbErrorType VimbaSystem::GetInterfaces( InterfacePtr *pInterfaces, VmbUint32_t &rnSize )
{
    VmbErrorType res = VmbErrorInternalFault;

    // Begin write lock interface list
    if ( true == m_pImpl->m_interfacesConditionHelper.EnterWriteLock( m_pImpl->m_interfaces ))
    {
        res = m_pImpl->UpdateInterfaceList();

        if ( VmbErrorSuccess == res)
        {
            if ( NULL == pInterfaces )
            {
                rnSize = (VmbUint32_t)m_pImpl->m_interfaces.Map.size();
                res = VmbErrorSuccess;
            }
            else if ( m_pImpl->m_interfaces.Map.size() <= rnSize )
            {
                VmbUint32_t i = 0;
                for (   InterfacePtrMap::iterator iter = m_pImpl->m_interfaces.Map.begin();
                        m_pImpl->m_interfaces.Map.end() != iter;
                        ++iter, ++i )
                {
                    pInterfaces[i] = iter->second;
                }
                rnSize = (VmbUint32_t)m_pImpl->m_interfaces.Map.size();
                res = VmbErrorSuccess;
            }
            else
            {
                res = VmbErrorMoreData;
            }
        }

        // End write lock interface list
        m_pImpl->m_interfacesConditionHelper.ExitWriteLock( m_pImpl->m_interfaces );
    }
    
    return res;
}

VmbErrorType VimbaSystem::GetInterfaceByID( const char *pStrID, InterfacePtr &rInterface )
{
    if ( NULL == pStrID )
    {
        return VmbErrorBadParameter;
    }

    VmbErrorType res = VmbErrorNotFound;
    
    // Begin write lock interface list
    if ( true == m_pImpl->m_interfacesConditionHelper.EnterWriteLock( m_pImpl->m_interfaces ))
    {
        InterfacePtrMap::iterator iter = m_pImpl->m_interfaces.Map.find( pStrID );
        if ( m_pImpl->m_interfaces.Map.end() != iter )
        {
            rInterface = iter->second;
            res = VmbErrorSuccess;
        }
        else
        {
            std::vector<VmbInterfaceInfo_t> interfaceInfos;
            res = m_pImpl->GetInterfaceList( interfaceInfos );

            if ( VmbErrorSuccess == res )
            {
                for (   std::vector<VmbInterfaceInfo_t>::iterator iterInfo = interfaceInfos.begin();
                        interfaceInfos.end() != iterInfo;
                        ++iterInfo )
                {
                    if ( 0 ==  strcmp( iterInfo->interfaceIdString, pStrID ))
                    {
                        SP_SET( m_pImpl->m_interfaces.Map[pStrID], new Interface( &(*iterInfo) ));
                        break;
                    }
                }

                iter = m_pImpl->m_interfaces.Map.find( pStrID );
                if ( m_pImpl->m_interfaces.Map.end() != iter )
                {
                    rInterface = iter->second;
                }
                else
                {
                    res = VmbErrorNotFound;
                }
            }
        }

        // End write lock interface list
        m_pImpl->m_interfacesConditionHelper.ExitWriteLock( m_pImpl->m_interfaces );
    }

    return res;
}

VmbErrorType VimbaSystem::OpenInterfaceByID( const char *pStrID, InterfacePtr &rInterface )
{
    if ( NULL == pStrID )
    {
        return VmbErrorBadParameter;
    }

    VmbErrorType res = GetInterfaceByID( pStrID, rInterface );
    if ( VmbErrorSuccess == res )
    {
        return SP_ACCESS( rInterface )->Open();
    }

    return res;
}

VmbErrorType VimbaSystem::GetCameras( CameraPtr *pCameras, VmbUint32_t &rnSize )
{
    VmbErrorType res = VmbErrorInternalFault;

    // Begin write lock camera list
    if ( true == m_pImpl->m_camerasConditionHelper.EnterWriteLock( m_pImpl->m_cameras ))
    {
        res = m_pImpl->UpdateCameraList();

        if ( VmbErrorSuccess == res )
        {
            if ( NULL == pCameras )
            {
                rnSize = (VmbUint32_t)m_pImpl->m_cameras.Map.size();
                res = VmbErrorSuccess;
            }
            else if ( m_pImpl->m_cameras.Map.size() <= rnSize )
            {
                VmbUint32_t i = 0;
                for (   CameraPtrMap::iterator iter = m_pImpl->m_cameras.Map.begin();
                        m_pImpl->m_cameras.Map.end() != iter;
                        ++iter, ++i )
                {
                    pCameras[i] = iter->second;
                }
                rnSize = (VmbUint32_t)m_pImpl->m_cameras.Map.size();
                res = VmbErrorSuccess;
            }
            else
            {
                res = VmbErrorMoreData;
            }
        }

        // End write lock camera list
        m_pImpl->m_camerasConditionHelper.ExitWriteLock( m_pImpl->m_cameras );
    }    

    return res;
}

VmbErrorType VimbaSystem::GetCameraByID( const char *pStrID, CameraPtr &rCamera )
{
    if ( NULL == pStrID )
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorNotFound;

    // Begin write lock camera list
    if ( true == m_pImpl->m_camerasConditionHelper.EnterWriteLock( m_pImpl->m_cameras ))
    {
        // Try to identify the desired camera by its ID (in the list of known cameras)
        CameraPtrMap::iterator iter = m_pImpl->m_cameras.Map.find( pStrID );
        if ( m_pImpl->m_cameras.Map.end() != iter )
        {
            rCamera = iter->second;
            res = VmbErrorSuccess;
        }
        else
        {
            // Try to identify the desired camera by IP or MAC address (in the list of known cameras)
            if (    true == m_pImpl->m_bGeVTLPresent
                 && false == m_pImpl->IsIPAddress(pStrID) )
            {
                // check if GeV discovery is enabled
                const char *pDiscoveryStatus = NULL;
                res = VmbFeatureEnumGet( gVimbaHandle, "GeVDiscoveryStatus", &pDiscoveryStatus );
                if ( VmbErrorSuccess == res )
                {
                    VmbInt64_t discoveryValue = 0;
                    res = VmbFeatureEnumAsInt( gVimbaHandle, "GeVDiscoveryStatus", pDiscoveryStatus, &discoveryValue );
                    if ( 1 != discoveryValue )
                    {
                        // HINT: We have to send one discovery packet in case we want to open a GigE cam (unless we open it by IP address)
                        res = VmbFeatureCommandRun( gVimbaHandle, "GeVDiscoveryAllOnce" );
                        if ( VmbErrorSuccess != res )
                        {
                            LOG_FREE_TEXT( "Could not ping camera over ethernet" )
                        }
                    }
                }
            }

            VmbCameraInfo_t camInfo;
            res = VmbCameraInfoQuery( pStrID, &camInfo, sizeof camInfo );
            if ( VmbErrorSuccess == res )
            {
                iter = m_pImpl->m_cameras.Map.find( camInfo.cameraIdString );
                if ( m_pImpl->m_cameras.Map.end() != iter )
                {
                    rCamera = iter->second;
                }
                else
                {
                    // We don't know the camera because it is new or we have to
                    // try to identify it by IP or MAC address directly
                    std::string cameraIdString;
                    if ( std::strcmp( camInfo.cameraIdString, pStrID ))
                    {
                        // TODO: Remove this with interface change                        
                        cameraIdString.assign( camInfo.cameraIdString ).append( AVT_IP_OR_MAC_ADDRESS ).append( pStrID );
                        camInfo.cameraIdString = cameraIdString.c_str();
                    }
                    m_pImpl->AppendCamToMap( camInfo );

                    iter = m_pImpl->m_cameras.Map.find( camInfo.cameraIdString );
                    if ( m_pImpl->m_cameras.Map.end() != iter )
                    {
                        rCamera = iter->second;
                    }
                    else
                    {
                        res = VmbErrorNotFound;
                    }
                }
            }
        }

        // End write lock camera list
        m_pImpl->m_camerasConditionHelper.ExitWriteLock( m_pImpl->m_cameras );
    }

    return (VmbErrorType)res;
}

VmbErrorType VimbaSystem::OpenCameraByID( const char *pStrID, VmbAccessModeType eAccessMode, CameraPtr &rCamera )
{
    if ( NULL == pStrID )
    {
        return VmbErrorBadParameter;
    }

    VmbErrorType res = GetCameraByID( pStrID, rCamera );
    if ( VmbErrorSuccess == res )
    {
        return SP_ACCESS( rCamera )->Open( eAccessMode );
    }

    return res;
}

CameraPtr VimbaSystem::GetCameraPtrByHandle( const VmbHandle_t handle ) const
{
    CameraPtr res;

    // Begin read lock camera list
    if ( true == m_pImpl->m_camerasConditionHelper.EnterReadLock( m_pImpl->m_cameras ) )
    {
        for (   CameraPtrMap::const_iterator iter = m_pImpl->m_cameras.Map.begin();
                m_pImpl->m_cameras.Map.end() != iter;
                ++iter)
        {
            if ( SP_ACCESS( iter->second )->GetHandle() == handle )
            {
                res = iter->second;
                break;
            }
        }

        // End read lock camera list
        m_pImpl->m_camerasConditionHelper.ExitReadLock( m_pImpl->m_cameras );
    }
    else
    {
        LOG_FREE_TEXT( "Could not lock camera list")
    }

    return res;
}

void VMB_CALL VimbaSystem::Impl::CameraDiscoveryCallback( const VmbHandle_t /*handle*/, const char* /*name*/, void* /*context*/ )
{
    VmbError_t err;
    std::vector<char> strID;
    VmbUint32_t nCount = 0;

    // Get the ID of the camera that has triggered the callback
    err = VmbFeatureStringMaxlengthQuery( gVimbaHandle, "DiscoveryCameraIdent", &nCount );
    if (    0 < nCount
        &&  VmbErrorSuccess == err )
    {                    
        strID.resize( nCount );
        err = VmbFeatureStringGet( gVimbaHandle, "DiscoveryCameraIdent", &strID[0], nCount, &nCount );
        if ( VmbErrorSuccess == err )
        {
            UpdateTriggerType reason = (UpdateTriggerType)0;
            const char* pReason = NULL;
            VmbInt64_t nReason = 0;

            // Get the reason that has triggered the callback
            err = VmbFeatureEnumGet( gVimbaHandle, "DiscoveryCameraEvent", &pReason );
            if ( VmbErrorSuccess == err )
            {
                err = VmbFeatureEnumAsInt( gVimbaHandle, "DiscoveryCameraEvent", pReason, &nReason );
                if ( VmbErrorSuccess == err )
                {
                    switch ( nReason )
                    {
                    case 0: reason = UpdateTriggerPluggedOut;
                        break;
                    case 1: reason = UpdateTriggerPluggedIn;
                        break;
                    default: reason = UpdateTriggerOpenStateChanged;
                    }
                    
                    // Begin read lock camera list
                    if ( true == _instance.m_pImpl->m_camerasConditionHelper.EnterReadLock( _instance.m_pImpl->m_cameras ))
                    {
                        CameraPtrMap::iterator iter = _instance.m_pImpl->m_cameras.Map.find( &strID[0] );
                        CameraPtr pCam;

                        bool bFound;

                        // Was the camera known before?
                        if ( _instance.m_pImpl->m_cameras.Map.end() != iter )
                        {
                            bFound = true;
                            pCam = iter->second;
                        }
                        else
                        {
                            bFound = false;
                        }

                        // End read lock camera list
                        _instance.m_pImpl->m_camerasConditionHelper.ExitReadLock( _instance.m_pImpl->m_cameras );

                        // If the camera was not known before we query for it
                        if ( false == bFound )
                        {
                            err = _instance.GetCameraByID( &strID[0], pCam );
                            if ( VmbErrorSuccess != err )
                            {
                                err = VmbErrorInternalFault;
                                LOG_FREE_TEXT( "Could not find a known camera in camera list")
                            }
                        }

                        // Now that we know about the reason for the callback and the camera we can call all registered observers
                        if ( VmbErrorSuccess == err )
                        {
                            // Begin read lock camera observer list
                            if ( true == _instance.m_pImpl->m_cameraObserversConditionHelper.EnterReadLock( _instance.m_pImpl->m_cameraObservers ))
                            {
                                for (   ICameraListObserverPtrVector::iterator iter = _instance.m_pImpl->m_cameraObservers.Vector.begin();
                                        _instance.m_pImpl->m_cameraObservers.Vector.end() != iter;
                                        ++iter )
                                {
                                    SP_ACCESS(( *iter ))->CameraListChanged( pCam, reason );
                                }

                                // End read lock camera observer list
                                _instance.m_pImpl->m_cameraObserversConditionHelper.ExitReadLock( _instance.m_pImpl->m_cameraObservers );
                            }
                            else
                            {
                                LOG_FREE_TEXT( "Could not lock camera observer list")
                            }
                        }
                    }
                    else
                    {
                        LOG_FREE_TEXT( "Could not lock camera list")
                    }
                }
                else
                {
                    LOG_FREE_TEXT( "Could not get integer representation of enum string" )
                }
            }
            else
            {
                LOG_FREE_TEXT( "Could not get callback trigger" )
            }
        }
        else
        {
            LOG_FREE_TEXT( "Could not get camera ID" )
        }
    }
    else
    {
        LOG_FREE_TEXT( "Could not get length of camera ID or length is 0" )
    }
}

void VMB_CALL VimbaSystem::Impl::InterfaceDiscoveryCallback( const VmbHandle_t /*handle*/, const char * /*name*/, void * /*context*/ )
{
    VmbError_t err;
    std::vector<char> strID;
    VmbUint32_t nCount = 0;

    // Get the ID of the interface that has triggered the callback
    err = VmbFeatureStringMaxlengthQuery( gVimbaHandle, "DiscoveryInterfaceIdent", &nCount );
    if (    0 < nCount
        &&  VmbErrorSuccess == err )
    {                    
        strID.resize( nCount );
        err = VmbFeatureStringGet( gVimbaHandle, "DiscoveryInterfaceIdent", &strID[0], nCount, &nCount );
    }

    if ( VmbErrorSuccess == err )
    {
        // Begin read lock interface list
        if ( true == _instance.m_pImpl->m_interfacesConditionHelper.EnterReadLock( _instance.m_pImpl->m_interfaces ))
        {
            InterfacePtrMap::iterator iter = _instance.m_pImpl->m_interfaces.Map.find( &strID[0] );
            InterfacePtr pInterface;
            UpdateTriggerType reason = (UpdateTriggerType)0;
            bool bFound;

            if ( _instance.m_pImpl->m_interfaces.Map.end() != iter )
            {
                bFound = true;
                pInterface = iter->second;
            }
            else
            {
                bFound = false;
            }

            // End read lock interface list
            _instance.m_pImpl->m_interfacesConditionHelper.ExitReadLock( _instance.m_pImpl->m_interfaces );

            // Begin write lock interface list
            if ( true == _instance.m_pImpl->m_interfacesConditionHelper.EnterWriteLock( _instance.m_pImpl->m_interfaces ))
            {
                err = _instance.m_pImpl->UpdateInterfaceList();

                // End write lock interface list
                _instance.m_pImpl->m_interfacesConditionHelper.ExitWriteLock( _instance.m_pImpl->m_interfaces );
                
                if ( VmbErrorSuccess == err )
                {
                    // Begin read lock interface list
                    if ( true == _instance.m_pImpl->m_interfacesConditionHelper.EnterReadLock( _instance.m_pImpl->m_interfaces ))
                    {
                        iter = _instance.m_pImpl->m_interfaces.Map.find( &strID[0] );

                        // The interface was known before
                        if ( true == bFound )
                        {
                            // The interface now has been removed
                            if ( _instance.m_pImpl->m_interfaces.Map.end() == iter )
                            {
                                reason = UpdateTriggerPluggedOut;
                            }
                            else
                            {
                                reason = UpdateTriggerOpenStateChanged;
                            }
                        }
                        // The interface is new
                        else
                        {
                            if ( _instance.m_pImpl->m_interfaces.Map.end() != iter )
                            {
                                pInterface = iter->second;
                                reason = UpdateTriggerPluggedIn;
                            }
                            else
                            {
                                err = VmbErrorInternalFault;
                                // Do some logging
                                LOG_FREE_TEXT( "Could not find interface in interface list." )
                            }
                        }

                        // End read lock interface list
                        _instance.m_pImpl->m_interfacesConditionHelper.ExitReadLock( _instance.m_pImpl->m_interfaces );

                        if ( VmbErrorSuccess == err )
                        {
                            // Begin read lock interface observer list
                            if ( true == _instance.m_pImpl->m_interfaceObserversConditionHelper.EnterReadLock( _instance.m_pImpl->m_interfaceObservers ))
                            {
                                for (   IInterfaceListObserverPtrVector::iterator iter = _instance.m_pImpl->m_interfaceObservers.Vector.begin();
                                    _instance.m_pImpl->m_interfaceObservers.Vector.end() != iter;
                                    ++iter)
                                {
                                    SP_ACCESS(( *iter ))->InterfaceListChanged( pInterface, reason );
                                }

                                // End read lock interface observer list
                                _instance.m_pImpl->m_interfaceObserversConditionHelper.ExitReadLock( _instance.m_pImpl->m_interfaceObservers );
                            }
                            else
                            {
                                LOG_FREE_TEXT( "Could not lock interface observer list")
                            }
                        }                        
                    }
                    else
                    {
                        LOG_FREE_TEXT( "Could not lock interface list")
                    }
                }                
            }            
        }
        else
        {
            LOG_FREE_TEXT( "Could not lock interface list")
        }
    }
}

VmbErrorType VimbaSystem::RegisterCameraListObserver( const ICameraListObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorSuccess;

    // Begin write lock camera observer list
    if ( true == _instance.m_pImpl->m_cameraObserversConditionHelper.EnterWriteLock( m_pImpl->m_cameraObservers ))
    {
        // The very same observer cannot be registered twice
        for ( size_t i=0; i<m_pImpl->m_cameraObservers.Vector.size(); ++i )
        {
            if ( SP_ISEQUAL( rObserver, m_pImpl->m_cameraObservers.Vector[i] ))
            {
                res = VmbErrorInvalidCall;
                break;
            }
        }

        if ( VmbErrorSuccess == res )
        {
            m_pImpl->m_cameraObservers.Vector.push_back( rObserver );

            if ( 1 == m_pImpl->m_cameraObservers.Vector.size() )
            {
                res = VmbFeatureInvalidationRegister( gVimbaHandle, "DiscoveryCameraEvent", m_pImpl->CameraDiscoveryCallback, this );
                if (    VmbErrorSuccess == res
                     && true == m_pImpl->m_bGeVTLPresent )
                {
                    // check if GeV discovery was already enabled
                    const char *pDiscoveryStatus = NULL;
                    res = VmbFeatureEnumGet( gVimbaHandle, "GeVDiscoveryStatus", &pDiscoveryStatus );
                    if ( VmbErrorSuccess == res )
                    {
                        VmbInt64_t discoveryValue = 0;
                        res = VmbFeatureEnumAsInt( gVimbaHandle, "GeVDiscoveryStatus", pDiscoveryStatus, &discoveryValue );
                        if ( VmbErrorSuccess == res )
                        {
                            if ( 1 != discoveryValue )
                            {
                                // HINT: Without enabling GEVDiscovery registering a device observer is pointless
                                res = VmbFeatureCommandRun( gVimbaHandle, "GeVDiscoveryAllAuto" );
                                if ( VmbErrorSuccess == res )
                                {
                                    m_pImpl->m_bGeVDiscoveryAutoOn = true;
                                }
                            }
                        }
                    }
                }

                if ( VmbErrorSuccess != res )
                {
                    // Rollback
                    m_pImpl->m_cameraObservers.Vector.pop_back();
                    // Do some logging
                    LOG_FREE_TEXT( "Could not register camera list observer" )
                }
            }
        }

        // End write lock camera observer list
        _instance.m_pImpl->m_cameraObserversConditionHelper.ExitWriteLock( m_pImpl->m_cameraObservers );
    }    
    
    return (VmbErrorType)res;
}

VmbErrorType VimbaSystem::UnregisterCameraListObserver( const ICameraListObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorNotFound;

    // Begin exclusive write lock camera observer list
    if ( true == m_pImpl->m_cameraObserversConditionHelper.EnterWriteLock( m_pImpl->m_cameraObservers, true ))
    {
        for (   ICameraListObserverPtrVector::iterator iter = m_pImpl->m_cameraObservers.Vector.begin();
                m_pImpl->m_cameraObservers.Vector.end() != iter;)    
        {
            if ( SP_ISEQUAL( rObserver, *iter ))
            {
                // If we are about to unregister the last observer we cancel all camera discovery notifications
                if ( 1 == m_pImpl->m_cameraObservers.Vector.size() )
                {
                    res = VmbFeatureInvalidationUnregister( gVimbaHandle, "DiscoveryCameraEvent", m_pImpl->CameraDiscoveryCallback );
                    if (    VmbErrorSuccess == res
                         && true == m_pImpl->m_bGeVTLPresent )
                    {
                        // check if GeV discovery was already enabled
                        const char *pDiscoveryStatus = NULL;
                        res = VmbFeatureEnumGet( gVimbaHandle, "GeVDiscoveryStatus", &pDiscoveryStatus );
                        if ( VmbErrorSuccess == res )
                        {
                            VmbInt64_t discoveryValue = 0;
                            res = VmbFeatureEnumAsInt( gVimbaHandle, "GeVDiscoveryStatus", pDiscoveryStatus, &discoveryValue );
                            if ( VmbErrorSuccess == res )
                            {
                                if ( 0 != discoveryValue )
                                {
                                    // HINT: After unregistering the last device observer we do not need to send discovery pings anymore
                                    res = VmbFeatureCommandRun( gVimbaHandle, "GeVDiscoveryAllOff" );
                                    if ( VmbErrorSuccess == res )
                                    {
                                        m_pImpl->m_bGeVDiscoveryAutoOn = false;
                                    }
                                    else
                                    {
                                        // Rollback
                                        VmbFeatureInvalidationRegister( gVimbaHandle, "DiscoveryCameraEvent", m_pImpl->CameraDiscoveryCallback, this );
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (    VmbErrorSuccess == res
                     || 1 < m_pImpl->m_cameraObservers.Vector.size() )
                {
                    iter = m_pImpl->m_cameraObservers.Vector.erase( iter );
                    res = VmbErrorSuccess;
                }
                break;
            }
            else
            {
                ++iter;
            }
        }

        // End write lock camera observer list
        m_pImpl->m_cameraObserversConditionHelper.ExitWriteLock( m_pImpl->m_cameraObservers );
    }
    else
    {
        LOG_FREE_TEXT( "Could not lock camera observer list.")
        res = VmbErrorInternalFault;
    }

    return (VmbErrorType)res;
}

VmbErrorType VimbaSystem::RegisterInterfaceListObserver( const IInterfaceListObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorSuccess;

    // Begin write lock interface observer list
    if ( true == _instance.m_pImpl->m_interfaceObserversConditionHelper.EnterWriteLock( m_pImpl->m_interfaceObservers ))
    {
        // The very same observer cannot be registered twice
        for ( size_t i=0; i<m_pImpl->m_interfaceObservers.Vector.size(); ++i )
        {
            if ( SP_ISEQUAL( rObserver, m_pImpl->m_interfaceObservers.Vector[i] ))
            {
                res = VmbErrorInvalidCall;
                break;
            }
        }

        if ( VmbErrorSuccess == res )
        {
            m_pImpl->m_interfaceObservers.Vector.push_back( rObserver );

            if ( 1 == m_pImpl->m_interfaceObservers.Vector.size() )
            {
                res = VmbFeatureInvalidationRegister( gVimbaHandle, "DiscoveryInterfaceEvent", m_pImpl->InterfaceDiscoveryCallback, this );

                if ( VmbErrorSuccess != res )
                {
                    // Rollback
                    m_pImpl->m_interfaceObservers.Vector.pop_back();

                    // Do some logging
                    LOG_FREE_TEXT( "Could not register interface list observer" )
                }
            }
        }

        // End write lock interface observer list
        _instance.m_pImpl->m_interfaceObserversConditionHelper.ExitWriteLock( m_pImpl->m_interfaceObservers );
    }    

    return (VmbErrorType)res;
}

VmbErrorType VimbaSystem::UnregisterInterfaceListObserver( const IInterfaceListObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    VmbError_t res = VmbErrorNotFound;

    // Begin exclusive write lock interface observer list
    if ( true == _instance.m_pImpl->m_interfaceObserversConditionHelper.EnterWriteLock( m_pImpl->m_interfaceObservers, true ))
    {
        for (   IInterfaceListObserverPtrVector::iterator iter = m_pImpl->m_interfaceObservers.Vector.begin();
                m_pImpl->m_interfaceObservers.Vector.end() != iter;)
        {
            if ( SP_ISEQUAL( rObserver, *iter ))
            {
                // If we are about to unregister the last observer we cancel all interface discovery notifications
                if ( 1 == m_pImpl->m_interfaceObservers.Vector.size() )
                {
                    res = VmbFeatureInvalidationUnregister( gVimbaHandle, "DiscoveryInterfaceEvent", m_pImpl->InterfaceDiscoveryCallback );
                }
                if (    VmbErrorSuccess == res
                     || 1 < m_pImpl->m_interfaceObservers.Vector.size() )
                {
                    iter = m_pImpl->m_interfaceObservers.Vector.erase( iter );
                    res = VmbErrorSuccess;
                }
                break;
            }
            else
            {
                ++iter;
            }
        }

        // End write lock interface observer list
        _instance.m_pImpl->m_interfaceObserversConditionHelper.ExitWriteLock( m_pImpl->m_interfaceObservers );
    }
    else
    {
        LOG_FREE_TEXT( "Could not lock interface observer list.")
        res = VmbErrorInternalFault;
    }

    return (VmbErrorType)res;
}

VmbErrorType VimbaSystem::RegisterCameraFactory( const ICameraFactoryPtr &cameraFactory )
{
    if ( SP_ISNULL( cameraFactory ))
    {
        return VmbErrorBadParameter;
    }

    m_pImpl->m_pCameraFactory = cameraFactory;
    
    return VmbErrorSuccess;
}

VmbErrorType VimbaSystem::UnregisterCameraFactory()
{
    m_pImpl->m_pCameraFactory = ICameraFactoryPtr( new DefaultCameraFactory() );

    if ( SP_ISNULL( m_pImpl->m_pCameraFactory ))
    {
        return VmbErrorInternalFault;
    }

    return VmbErrorSuccess;
}

// Singleton
VimbaSystem::VimbaSystem()
    :   m_pImpl( new Impl() )
{
    m_pImpl->m_bGeVDiscoveryAutoOn = false;
    m_pImpl->m_bGeVTLPresent = false;
    m_pImpl->m_pLogger = new LOGGER_DEF;
    m_pImpl->m_pCameraFactory = ICameraFactoryPtr( new DefaultCameraFactory() );
}

// Singleton
VimbaSystem::VimbaSystem( const VimbaSystem& )
{
    // No generated copy ctor
}

VimbaSystem& VimbaSystem::operator=( const VimbaSystem& )
{
    // No assignment operator
    return *this;
}

VimbaSystem::~VimbaSystem()
{
    delete m_pImpl->m_pLogger;
    m_pImpl->m_pLogger = NULL;
    delete m_pImpl;
    m_pImpl = NULL;
}

// Instance
VimbaSystem VimbaSystem::_instance;

// Gets a list of all connected interfaces and updates the internal interfaces map accordingly.
// Reference counting for removed interfaces is decreased,
// new interfaces are added.
VmbErrorType VimbaSystem::Impl::UpdateInterfaceList()
{
    std::vector<VmbInterfaceInfo_t> interfaceInfos;
    VmbErrorType res = GetInterfaceList( interfaceInfos );
    VmbUint32_t nCount = (VmbUint32_t)interfaceInfos.size();

    if ( VmbErrorSuccess == res )
    {
        InterfacePtrMap::iterator iter = m_interfaces.Map.begin();
        std::vector<VmbInterfaceInfo_t>::iterator iterInfo = interfaceInfos.begin();
        bool bFound = false;

        // Delete removed Interfaces from m_interfaces
        while ( m_interfaces.Map.end() != iter )
        {
            for ( VmbUint32_t i=0; i<nCount; ++i, ++iterInfo )
            {
                if ( iterInfo->interfaceIdString == iter->first )
                {
                    bFound = true;
                    break;
                }
            }

            if ( false == bFound )
            {
                m_interfaces.Map.erase( iter++ );
            }
            else
            {
                ++iter;
            }

            bFound = false;
            iterInfo = interfaceInfos.begin();
        }

        // Add new Interfaces to m_Interfaces
        while ( 0 < nCount-- )
        {
            iter = m_interfaces.Map.find( iterInfo->interfaceIdString );

            if ( m_interfaces.Map.end() == iter )
            {
                SP_SET( m_interfaces.Map[iterInfo->interfaceIdString], new Interface( &(*iterInfo) ));
            }

            ++iterInfo;
        }
    }

    return res;
}

// Gets a list of all connected cameras and updates the internal cameras map accordingly.
// Reference counting for removed cameras is decreased,
// new cameras are added.
VmbErrorType VimbaSystem::Impl::UpdateCameraList()
{
    VmbError_t res = VmbErrorSuccess;
    VmbUint32_t nCount = 0;
    std::vector<VmbCameraInfo_t> cameraInfos( 10 );

    // HINT: We explicitly have to enable GeVDiscovery to be able to use UpdateCameraList.
    if ( true == m_bGeVTLPresent )
    {
        // check GeV discovery status
        const char *pDiscoveryStatus = NULL;
        res = VmbFeatureEnumGet( gVimbaHandle, "GeVDiscoveryStatus", &pDiscoveryStatus );
        if ( VmbErrorSuccess == res )
        {
            VmbInt64_t discoveryValue = 0;
            res = VmbFeatureEnumAsInt( gVimbaHandle, "GeVDiscoveryStatus", pDiscoveryStatus, &discoveryValue );
            if ( VmbErrorSuccess == res )
            {
                if ( 1 != discoveryValue )
                {
                    res = VmbFeatureCommandRun( gVimbaHandle, "GeVDiscoveryAllOnce" );
                    if ( VmbErrorSuccess == res )
                    {
                        m_bGeVDiscoveryAutoOn = true;
                    }
                }
            }
        }
    }
    try
    {
        if ( VmbErrorSuccess == res )
        {
            // First get 10 cameras at most
            res = VmbCamerasList( &cameraInfos[0], (VmbUint32_t)cameraInfos.size(), &nCount, sizeof(VmbCameraInfo_t) );
            // If there are more get them eventually
            // If even more new cameras were discovered in between the function calls we increase the allocated memory consecutively
            while ( VmbErrorMoreData == res )
            {
                cameraInfos.resize( nCount );
                res = VmbCamerasList( &cameraInfos[0], (VmbUint32_t)cameraInfos.size(), &nCount, sizeof(VmbCameraInfo_t) );
            }
        }

        if ( VmbErrorSuccess == res )
        {
            if( 0 != nCount )
            {
                if( nCount < cameraInfos.size() )
                {
                    cameraInfos.resize( nCount );
                }
                CameraPtrMap::iterator  mapPos  = m_cameras.Map.begin();
                typedef std::vector<VmbCameraInfo_t>::const_iterator const_info_iterator;

                // Delete removed cameras from m_cameras
                while ( m_cameras.Map.end() != mapPos )
                {
                    bool bFound = false;
                    for( const_info_iterator infoPos = cameraInfos.begin(); cameraInfos.end() != infoPos; ++infoPos )
                    {
                        if ( infoPos->cameraIdString == mapPos->first )
                        {
                            bFound = true;
                            break;
                        }
                    }

                    if ( false == bFound )
                    {
                        m_cameras.Map.erase( mapPos++ );
                    }
                    else
                    {
                        ++mapPos;
                    }
                }

                // Add new cameras to m_cameras
                for (const_info_iterator infoPos= cameraInfos.begin(); infoPos != cameraInfos.end(); ++infoPos )
                {
                    CameraPtrMap::const_iterator findPos = m_cameras.Map.find( infoPos->cameraIdString );
            
                    if ( m_cameras.Map.end() == findPos )
                    {
                        AppendCamToMap( *infoPos );
                    }
                }
            }
            else
            {
                m_cameras.Map.clear();
            }
        }
    }
    catch( const std::bad_alloc& /*badAlloc*/ )
    {
        return VmbErrorResources;
    }
    

    return (VmbErrorType)res;
}

Logger VimbaSystem::GetLogger() const
{
    if(m_pImpl == NULL)
        return NULL;
    return m_pImpl->m_pLogger;
}

bool VimbaSystem::Impl::IsIPAddress( const char *pStrID )
{
    if( NULL == pStrID )
    {
        return false;
    }

    size_t nCount = 0;
    size_t nSize = 0;
    size_t nIndex = 0;
    while( pStrID[nIndex] != '\0' )
    {
        if( isdigit( pStrID[nIndex] ) != 0 )
        {
            if( nSize >= 3 )
            {
                return false;
            }
            nSize++;
        }
        else if( '.' == pStrID[nIndex] )
        {
            if(     (nSize <= 0)
                ||  (nSize > 3)
                ||  (nCount >= 3) )
            {
                return false;
            }
            nCount++;
            nSize = 0;
        }
        else
        {
            return false;
        }

        nIndex++;
    }
    if(     (nSize <= 0)
        ||  (nSize > 3)
        ||  (nCount != 3) )
    {
        return false;
    }

    return true;
}

void VimbaSystem::Impl::AppendCamToMap( VmbCameraInfo_t camInfo )
{
    InterfacePtr        pInterface;
    std::string         strInterfaceName,
                        strInterfaceSerial;
    VmbAccessModeType   interfaceAccess;
    VmbInterfaceType    interfaceType;

    // HINT: Before inserting (and potentially overwriting) a camera, we check whether it is present already
    if ( m_cameras.Map.end() == m_cameras.Map.find( camInfo.cameraIdString ))
    {
        if ( VmbErrorSuccess == _instance.GetInterfaceByID( camInfo.interfaceIdString, pInterface ))
        {
            if (    VmbErrorSuccess == SP_ACCESS( pInterface )->GetName( strInterfaceName )
                 && VmbErrorSuccess == SP_ACCESS( pInterface )->GetSerialNumber( strInterfaceSerial )
                 && VmbErrorSuccess == SP_ACCESS( pInterface )->GetPermittedAccess( interfaceAccess )
                 && VmbErrorSuccess == SP_ACCESS( pInterface )->GetType( interfaceType ))
            {
                try
                {
                    // TODO: Remove pCam with Interface change
                    CameraPtr pCam = SP_ACCESS( m_pCameraFactory )->CreateCamera(   camInfo.cameraIdString,
                                                                                    camInfo.cameraName,
                                                                                    camInfo.modelName,
                                                                                    camInfo.serialString,
                                                                                    camInfo.interfaceIdString,
                                                                                    interfaceType,
                                                                                    strInterfaceName.c_str(),
                                                                                    strInterfaceSerial.c_str(),
                                                                                    interfaceAccess );
                    // TODO: Remove with interface change
                    char* strTemp = (char*)strstr( camInfo.cameraIdString, AVT_IP_OR_MAC_ADDRESS );
                    if ( strTemp )
                    {
                        *strTemp = '\0';
                    }
                    m_cameras.Map[camInfo.cameraIdString] = pCam;
                }
                catch( ... )
                {
                    // Do some logging
                    LOG_FREE_TEXT( "Could not create camera" )
                }

                CameraPtrMap::iterator iter = m_cameras.Map.find( camInfo.cameraIdString );
                if (    m_cameras.Map.end() != iter
                     && SP_ISNULL( iter->second ))
                {
                    m_cameras.Map.erase( iter );
                    // Do some logging
                    LOG_FREE_TEXT( "NULL camera created" )
                }
            }
            else // Could not get interface infos 
            {
                // Do some logging
                LOG_FREE_TEXT( "Could not get interface infos" )
            }
        }
        else // Could not get interface
        {
            // Do some logging
            LOG_FREE_TEXT( "Could not get interface" )
        }
    }
}

VmbErrorType VimbaSystem::Impl::GetInterfaceList( std::vector<VmbInterfaceInfo_t> &rInterfaceInfos )
{
    VmbError_t res;
    VmbUint32_t nCount;

    res = VmbInterfacesList( NULL, 0, &nCount, sizeof(VmbInterfaceInfo_t));
    if ( VmbErrorSuccess == res )
    {
        rInterfaceInfos.resize( nCount );
        res = VmbInterfacesList( &rInterfaceInfos[0], nCount, &nCount, sizeof(VmbInterfaceInfo_t));
    }

    return (VmbErrorType)res;
}

}} // namespace AVT::VmbAPI
