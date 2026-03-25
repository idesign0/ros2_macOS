/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------
 
  File:        Interface.cpp

  Description: Implementation of class AVT::VmbAPI::Interface.

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
#include <map>
#pragma warning(default:4996)

#include <VimbaCPP/Include/Interface.h>

namespace AVT {
namespace VmbAPI {

struct Interface::Impl
{
    // Copy of interface infos
    struct InterfaceInfo
    {
        std::string         interfaceIdString;     // Unique identifier for each interface
        VmbInterface_t      interfaceType;         // Interface type
        std::string         interfaceName;         // Interface name, given by the transport layer
        std::string         serialString;          // Serial number
        VmbAccessMode_t     permittedAccess;       // Used access mode, see VmbAccessMode_t
    } m_interfaceInfo;
};    

Interface::Interface()
{
    // No default ctor
}

Interface::Interface( const Interface& )
{
    // No copy ctor
}

Interface& Interface::operator=( const Interface& )
{
    // No assignment operator
    return *this;
}

Interface::Interface(const VmbInterfaceInfo_t *pInterfaceInfo)
    :   m_pImpl( new Impl() )
{
    m_pImpl->m_interfaceInfo.interfaceIdString.assign( pInterfaceInfo->interfaceIdString ? pInterfaceInfo->interfaceIdString : "" );
    m_pImpl->m_interfaceInfo.interfaceName.assign( pInterfaceInfo->interfaceName ? pInterfaceInfo->interfaceName : "" );
    m_pImpl->m_interfaceInfo.interfaceType = pInterfaceInfo->interfaceType;
    m_pImpl->m_interfaceInfo.permittedAccess = pInterfaceInfo->permittedAccess;
    m_pImpl->m_interfaceInfo.serialString.assign( pInterfaceInfo->serialString ? pInterfaceInfo->serialString : "" );
}

VmbErrorType Interface::Open()
{
    VmbError_t res;
    VmbHandle_t hHandle;
    
    res = VmbInterfaceOpen( m_pImpl->m_interfaceInfo.interfaceIdString.c_str(), &hHandle );


    if ( VmbErrorSuccess == res )
    {
        SetHandle( hHandle );
    }

    return (VmbErrorType)res;
}

Interface::~Interface()
{
    Close();

    delete m_pImpl;
}

VmbErrorType Interface::Close() 
{
    VmbError_t res = VmbErrorSuccess;

    if ( NULL != GetHandle() )
    {
        Reset();

        res = VmbInterfaceClose( GetHandle() );

        RevokeHandle();
    }

    return (VmbErrorType)res;
}

VmbErrorType Interface::GetID( char * const pStrID, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrID )
    {
        rnLength = (VmbUint32_t)m_pImpl->m_interfaceInfo.interfaceIdString.length();
        res = VmbErrorSuccess;
    }
    else if ( m_pImpl->m_interfaceInfo.interfaceIdString.length() <= rnLength )
    {
        std::copy( m_pImpl->m_interfaceInfo.interfaceIdString.begin(), m_pImpl->m_interfaceInfo.interfaceIdString.end(), pStrID );
        pStrID[m_pImpl->m_interfaceInfo.interfaceIdString.length()] = '\0';
        rnLength = (VmbUint32_t)m_pImpl->m_interfaceInfo.interfaceIdString.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType Interface::GetType( VmbInterfaceType &reType ) const
{
    reType = (VmbInterfaceType)m_pImpl->m_interfaceInfo.interfaceType;

    return VmbErrorSuccess;
}

VmbErrorType Interface::GetName( char * const pStrName, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrName )
    {
        rnLength = (VmbUint32_t)m_pImpl->m_interfaceInfo.interfaceName.length();
        res = VmbErrorSuccess;
    }
    else if ( m_pImpl->m_interfaceInfo.interfaceName.length() <= rnLength )
    {
        std::copy( m_pImpl->m_interfaceInfo.interfaceName.begin(), m_pImpl->m_interfaceInfo.interfaceName.end(), pStrName );
        pStrName[m_pImpl->m_interfaceInfo.interfaceName.length()] = '\0';
        rnLength = (VmbUint32_t)m_pImpl->m_interfaceInfo.interfaceName.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType Interface::GetSerialNumber( char * const pStrSerial, VmbUint32_t &rnLength ) const
{
    VmbErrorType res;

    if ( NULL == pStrSerial )
    {
        rnLength = (VmbUint32_t)m_pImpl->m_interfaceInfo.serialString.length();
        res = VmbErrorSuccess;
    }
    else if ( m_pImpl->m_interfaceInfo.serialString.length() <= rnLength )
    {
        std::copy( m_pImpl->m_interfaceInfo.serialString.begin(), m_pImpl->m_interfaceInfo.serialString.end(), pStrSerial );
        pStrSerial[m_pImpl->m_interfaceInfo.serialString.length()] = '\0';
        rnLength = (VmbUint32_t)m_pImpl->m_interfaceInfo.serialString.length();
        res = VmbErrorSuccess;
    }
    else
    {
        res = VmbErrorMoreData;
    }

    return res;
}

VmbErrorType Interface::GetPermittedAccess( VmbAccessModeType &reAccessMode ) const
{
    reAccessMode = (VmbAccessModeType)m_pImpl->m_interfaceInfo.permittedAccess;

    return VmbErrorSuccess;
}

}} // namespace AVT::VmbAPI
