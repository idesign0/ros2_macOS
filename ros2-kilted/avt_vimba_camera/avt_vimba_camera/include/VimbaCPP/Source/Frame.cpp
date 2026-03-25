/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Frame.cpp

  Description: Implementation of class AVT::VmbAPI::Frame.

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

#include "VimbaCPP/Include/Frame.h"

#include "VimbaCPP/Include/LoggerDefines.h"
#include "VimbaCPP/Include/VimbaSystem.h"
#include "VimbaCPP/Source/ConditionHelper.h"
#include "VimbaCPP/Include/SharedPointerDefines.h"
#include "VimbaCPP/Source/FrameImpl.h"
#include "VimbaCPP/Source/MutexGuard.h"
namespace AVT {
namespace VmbAPI {

Frame::Frame()    
{
    // No default ctor
}

Frame::Frame( Frame& )
{
    // No copy ctor
}

Frame& Frame::operator=( const Frame& )
{
    // No assignment operator
    return *this;
}

Frame::Frame( VmbInt64_t nBufferSize )
    :   m_pImpl( new Impl() )
{
    m_pImpl->m_bAlreadyAnnounced = false;
    m_pImpl->m_bAlreadyQueued = false;
    m_pImpl->m_bIsUserBuffer = false;
    SP_SET( m_pImpl->m_pObserverMutex, new Mutex() );
    m_pImpl->Init();
    m_pImpl->m_pBuffer = new VmbUchar_t[ (VmbUint32_t)nBufferSize ];
    m_pImpl->m_frame.bufferSize = (VmbUint32_t)nBufferSize;
    m_pImpl->m_frame.buffer = m_pImpl->m_pBuffer;
}

Frame::Frame( VmbUchar_t *pBuffer, VmbInt64_t nBufferSize )
    :   m_pImpl( new Impl() )
{
    m_pImpl->m_bAlreadyAnnounced = false;
    m_pImpl->m_bAlreadyQueued = false;
    m_pImpl->m_bIsUserBuffer = true;
    m_pImpl->m_pBuffer = NULL;
    SP_SET( m_pImpl->m_pObserverMutex, new Mutex());
    m_pImpl->Init();
    if ( NULL != pBuffer )
    {
        m_pImpl->m_pBuffer = pBuffer;
        m_pImpl->m_frame.bufferSize = (VmbUint32_t)nBufferSize;
        m_pImpl->m_frame.buffer = m_pImpl->m_pBuffer;
    }
    else
    {
        // Do some logging
        LOG_FREE_TEXT( "No valid buffer passed when constructing frame." )
    }
}

void Frame::Impl::Init()
{
    m_frame.ancillarySize = 0;
    m_frame.buffer = NULL;
    m_frame.bufferSize = 0;
    for ( int i=0; i<4; ++i)
    {
        m_frame.context[i] = NULL;
    }
    m_frame.frameID = 0;
    m_frame.height = 0;
    m_frame.imageSize = 0;
    m_frame.offsetX = 0;
    m_frame.offsetY = 0;
    m_frame.pixelFormat = 0;
    m_frame.receiveFlags = VmbFrameFlagsNone;
    m_frame.receiveStatus = VmbFrameStatusInvalid;
    m_frame.timestamp = 0;
    m_frame.width = 0;
}

Frame::~Frame()
{
    UnregisterObserver();
    if (    false == m_pImpl->m_bIsUserBuffer
         && NULL != m_pImpl->m_pBuffer )
    {
        delete [] m_pImpl->m_pBuffer;
    }

    delete m_pImpl;
}

VmbErrorType Frame::RegisterObserver( const IFrameObserverPtr &rObserver )
{
    if ( SP_ISNULL( rObserver ))
    {
        return VmbErrorBadParameter;
    }

    // Begin exclusive write lock observer
    MutexGuard local_lock( m_pImpl->m_pObserverMutex );
    m_pImpl->m_pObserver = rObserver;
    return VmbErrorSuccess;
}

VmbErrorType Frame::UnregisterObserver()
{
    VmbErrorType res = VmbErrorSuccess;

    // Begin exclusive write lock observer
    MutexGuard local_lock( m_pImpl->m_pObserverMutex );
    if ( SP_ISNULL( m_pImpl->m_pObserver ))
    {
        res = VmbErrorNotFound;
    }
    else
    {
        SP_RESET( m_pImpl->m_pObserver );
    }
    return res;
}

bool Frame::GetObserver( IFrameObserverPtr &rObserver ) const
{
    MutexGuard local_lock( m_pImpl->m_pObserverMutex );
    if ( SP_ISNULL( m_pImpl->m_pObserver ))
    {
        return false;
    }
    rObserver = m_pImpl->m_pObserver;
    return true;
}

VmbErrorType Frame::GetAncillaryData( AncillaryDataPtr &rAncillaryData )
{
    if ( m_pImpl->m_frame.ancillarySize == 0 )
    {
        return VmbErrorNotFound;
    }

    SP_SET( rAncillaryData, new AncillaryData( &m_pImpl->m_frame ));

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetAncillaryData( ConstAncillaryDataPtr &rAncillaryData ) const
{
    if ( m_pImpl->m_frame.ancillarySize == 0 )
    {
        return VmbErrorNotFound;
    }

    SP_SET( rAncillaryData, new AncillaryData( &m_pImpl->m_frame ));

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetBuffer( VmbUchar_t* &rpBuffer )
{
    rpBuffer = m_pImpl->m_pBuffer;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetBuffer( const VmbUchar_t* &rpBuffer ) const
{
    rpBuffer = m_pImpl->m_pBuffer;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetImage( VmbUchar_t* &rpBuffer )
{
    // HINT: On Allied Vision cameras image data always is at the beginning of the buffer
    rpBuffer = m_pImpl->m_pBuffer;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetImage( const VmbUchar_t* &rpBuffer ) const
{
    // HINT: On Allied Vision cameras image data always is at the beginning of the buffer
    rpBuffer = m_pImpl->m_pBuffer;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetReceiveStatus( VmbFrameStatusType &rStatus ) const
{
    rStatus = (VmbFrameStatusType)m_pImpl->m_frame.receiveStatus;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetImageSize( VmbUint32_t &rnImageSize ) const
{
    rnImageSize = m_pImpl->m_frame.imageSize;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetAncillarySize( VmbUint32_t &rnAncillarySize ) const
{
    rnAncillarySize = m_pImpl->m_frame.ancillarySize;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetBufferSize( VmbUint32_t &rnBufferSize ) const
{
    rnBufferSize =m_pImpl-> m_frame.bufferSize;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetPixelFormat( VmbPixelFormatType &rPixelFormat ) const
{
    rPixelFormat = (VmbPixelFormatType)m_pImpl->m_frame.pixelFormat;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetWidth( VmbUint32_t &rnWidth ) const
{
    rnWidth = m_pImpl->m_frame.width;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetHeight( VmbUint32_t &rnHeight ) const
{
    rnHeight = m_pImpl->m_frame.height;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetOffsetX( VmbUint32_t &rnOffsetX ) const
{
    rnOffsetX = m_pImpl->m_frame.offsetX;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetOffsetY( VmbUint32_t &rnOffsetY ) const
{
    rnOffsetY = m_pImpl->m_frame.offsetY;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetFrameID( VmbUint64_t &rnFrameID ) const
{
    rnFrameID = m_pImpl->m_frame.frameID;

    return VmbErrorSuccess;
}

VmbErrorType Frame::GetTimestamp( VmbUint64_t &rnTimestamp ) const
{
    rnTimestamp = m_pImpl->m_frame.timestamp;

    return VmbErrorSuccess;
}

}} // namespace AVT::VmbAPI
