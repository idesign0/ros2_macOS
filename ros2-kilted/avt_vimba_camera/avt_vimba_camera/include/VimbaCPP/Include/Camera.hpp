/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Camera.hpp

  Description: Inline wrapper functions for class AVT::VmbAPI::Camera.

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

#ifndef AVT_VMBAPI_CAMERA_HPP
#define AVT_VMBAPI_CAMERA_HPP

//
// Inline wrapper functions that allocate memory for STL objects in the application's context
// and to pass data across DLL boundaries using arrays
//

// HINT: This information remains static throughout the object's lifetime
inline VmbErrorType Camera::GetID( std::string &rStrID ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetID( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type>tmpID( nLength + 1,'\0');
                res = GetID( &tmpID[0], nLength );
                if( VmbErrorSuccess == res)
                {
                    rStrID  = &*tmpID.begin();
                }
            }
            catch(...)
            {
                res = VmbErrorResources;
            }
        }
        else
        {
            rStrID.clear();
        }
    }

    return res;
}

// HINT: This information remains static throughout the object's lifetime
inline VmbErrorType Camera::GetName( std::string &rStrName ) const
{
    VmbErrorType res;
    VmbUint32_t nLength;

    res = GetName( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nLength)
        {
            try
            {
                std::vector<std::string::value_type> tmpName( nLength + 1,'\0' );
                res  = GetName( &tmpName[0], nLength );
                if( VmbErrorSuccess == res)
                {
                    rStrName    = &*tmpName.begin();
                }
            }
            catch(...)
            {
                res = VmbErrorResources;
            }
        }
        else
        {
            rStrName.clear();
        }
    }

    return res;
}

// HINT: This information remains static throughout the object's lifetime
inline VmbErrorType Camera::GetModel( std::string &rStrModel ) const
{
    VmbErrorType res;
    VmbUint32_t nLength;

    res = GetModel( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nLength)
        {
            try
            {
                std::vector<std::string::value_type> tmpModel( nLength + 1,'\0');
                res = GetModel( &tmpModel[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrModel   = &*tmpModel.begin();
                }
            }
            catch(...)
            {
                res = VmbErrorResources;
            }
        }
        else
        {
            rStrModel.clear();
        }
    }

    return res;
}

// HINT: This information remains static throughout the object's lifetime
inline VmbErrorType Camera::GetSerialNumber( std::string &rStrSerial ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetSerialNumber( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpSerial( nLength + 1,'\0');
                res = GetSerialNumber( &tmpSerial[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrSerial  = &*tmpSerial.begin();
                }
            }
            catch(...)
            {
                res = VmbErrorResources;
            }
        }
        else
        {
            rStrSerial.clear();
        }
    }

    return res;
}

// HINT: This information remains static throughout the object's lifetime
inline VmbErrorType Camera::GetInterfaceID( std::string &rStrInterfaceID ) const
{
    VmbErrorType res;
    VmbUint32_t nLength;

    res = GetInterfaceID( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpID( nLength + 1,'\0');
                res = GetInterfaceID( &tmpID[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrInterfaceID = &*tmpID.begin();
                }
            }
            catch(...)
            {
                res = VmbErrorResources;
            }
        }
        else
        {
            rStrInterfaceID.clear();
        }
    }

    return res;
}

inline VmbErrorType Camera::AcquireMultipleImages( FramePtrVector &rFrames, VmbUint32_t nTimeout )
{
    VmbErrorType res;
    VmbUint32_t i;
    res = AcquireMultipleImages( rFrames, nTimeout, i );
    if ( rFrames.size() != i )
    {
        res = VmbErrorInternalFault;
    }
    
    return res;
}
inline VmbErrorType Camera::AcquireMultipleImages( FramePtrVector &rFrames, VmbUint32_t nTimeout, VmbUint32_t &rNumFramesCompleted )
{
    if ( true == rFrames.empty() )
    {
        return VmbErrorBadParameter;
    }

    return AcquireMultipleImages( &rFrames[0], (VmbUint32_t)rFrames.size(), nTimeout, &rNumFramesCompleted );
}

// HINT: Size of address determines how many registers to read. Size of data has to be large enough to hold the requested information
inline VmbErrorType Camera::ReadRegisters( const Uint64Vector &rAddresses, Uint64Vector &rBuffer ) const
{
    VmbUint32_t i;
    return ReadRegisters( rAddresses, rBuffer, i );
}
inline VmbErrorType Camera::ReadRegisters( const Uint64Vector &rAddresses, Uint64Vector &rBuffer, VmbUint32_t &rCompletedReads ) const
{
    if (    true == rAddresses.empty()
         || true == rBuffer.empty()
         || rAddresses.size() > rBuffer.size() )
    {
        return VmbErrorBadParameter;
    }
    return ReadRegisters( &rAddresses[0], (VmbUint32_t)rAddresses.size(), &rBuffer[0], &rCompletedReads );
}        

// HINT: Size of address determines how many registers to write.
inline VmbErrorType Camera::WriteRegisters( const Uint64Vector &rAddresses, const Uint64Vector &rBuffer )
{
    VmbUint32_t i;
    return WriteRegisters( rAddresses, rBuffer, i );
}
inline VmbErrorType Camera::WriteRegisters( const Uint64Vector &rAddresses, const Uint64Vector &rBuffer, VmbUint32_t &rCompletedWrites )
{
    if (    true == rAddresses.empty()
         || true == rBuffer.empty()
         || rAddresses.size() != rBuffer.size() )
    {
        return VmbErrorBadParameter;
    }

    return WriteRegisters( &rAddresses[0], (VmbUint32_t)rAddresses.size(), &rBuffer[0], &rCompletedWrites );
}

// HINT: Size of buffer determines how many bytes to read.
inline VmbErrorType Camera::ReadMemory( const VmbUint64_t &rAddress, UcharVector &rBuffer ) const
{
    VmbUint32_t i;
    return ReadMemory( rAddress, rBuffer, i );
}
inline VmbErrorType Camera::ReadMemory( const VmbUint64_t &rAddress, UcharVector &rBuffer, VmbUint32_t &rCompletedReads ) const
{
    if ( true == rBuffer.empty() )
    {
        return VmbErrorBadParameter;
    }

    return ReadMemory( rAddress, &rBuffer[0], (VmbUint32_t)rBuffer.size(), &rCompletedReads );
}

// HINT: Size of buffer determines how many bytes to write.
inline VmbErrorType Camera::WriteMemory( const VmbUint64_t &rAddress, const UcharVector &rBuffer )
{
    VmbUint32_t i;
    return WriteMemory( rAddress, rBuffer, i );
}
inline VmbErrorType Camera::WriteMemory( const VmbUint64_t &rAddress, const UcharVector &rBuffer, VmbUint32_t &rCompletedWrites )
{
    if ( true == rBuffer.empty() )
    {
        return VmbErrorBadParameter;
    }

    return WriteMemory( rAddress, &rBuffer[0], (VmbUint32_t)rBuffer.size(), &rCompletedWrites );
}

inline VmbErrorType Camera::SaveCameraSettings( std::string strFileName, VmbFeaturePersistSettings_t *pSettings ) const
{
//  parameter check
    if( true == strFileName.empty() )
    {
        return VmbErrorBadParameter;
    }

    return SaveCameraSettings( strFileName.c_str(), pSettings );
}

inline VmbErrorType Camera::LoadCameraSettings( std::string strFileName, VmbFeaturePersistSettings_t *pSettings ) const
{
//  parameter check
    if( true == strFileName.empty() )
    {
        return VmbErrorBadParameter;
    }

    return LoadCameraSettings( strFileName.c_str(), pSettings );
}

#endif
