/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Frame.h

  Description: Definition of class AVT::VmbAPI::Frame.

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

#ifndef AVT_VMBAPI_FRAME_H
#define AVT_VMBAPI_FRAME_H

#include <VimbaC/Include/VimbaC.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>
#include <VimbaCPP/Include/IFrameObserver.h>
#include <VimbaCPP/Include/AncillaryData.h>
#include <vector>

namespace AVT {
namespace VmbAPI {

class Camera; // forward declaration of camera class for befriending

class Frame 
{
  friend class Camera;

  public:
    //
    // Method:      Frame constructor
    //
    // Purpose:     Creates an instance of class Frame of a certain size
    //
    // Parameters:  [in ]   VmbInt64_t      bufferSize  The size of the underlying buffer
    //
    IMEXPORT explicit Frame( VmbInt64_t bufferSize );

    //
    // Method:      Frame constructor
    //
    // Purpose:     Creates an instance of class Frame with the given user buffer of the given size
    //
    // Parameters:  [in ]   VmbUchar_t*     pBuffer     A pointer to an allocated buffer
    // Parameters:  [in ]   VmbInt64_t      bufferSize  The size of the underlying buffer
    //
    IMEXPORT Frame( VmbUchar_t *pBuffer, VmbInt64_t bufferSize );

    //
    // Method:      Frame destructor
    //
    // Purpose:     Destroys an instance of class Frame
    //
    IMEXPORT ~Frame();

    //
    // Method:      RegisterObserver()
    //
    // Purpose:     Registers an observer that will be called whenever a new frame arrives
    //
    // Parameters:  [in ]   const IFrameObserverPtr&   pObserver   An object that implements the IObserver interface
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //  - VmbErrorBadParameter: "pObserver" is NULL.
    //  - VmbErrorResources:    The observer was in use
    //
    // Details:     As new frames arrive, the observer's FrameReceived method will be called.
    //              Only one observer can be registered.
    //
    IMEXPORT VmbErrorType RegisterObserver( const IFrameObserverPtr &pObserver );

    //
    // Method:      UnregisterObserver()
    //
    // Purpose:     Unregisters the observer that was called whenever a new frame arrived
    //
    IMEXPORT VmbErrorType UnregisterObserver();

    //
    // Method:      GetAncillaryData()
    //
    // Purpose:     Returns the part of a frame that describes the chunk data as an object
    //
    // Parameters:  [out]   AncillaryDataPtr&        pAncillaryData      The wrapped chunk data
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //  - VmbErrorNotFound:     No chunk data present
    //
    IMEXPORT VmbErrorType GetAncillaryData( AncillaryDataPtr &pAncillaryData );

    //
    // Method:      GetAncillaryData()
    //
    // Purpose:     Returns the part of a frame that describes the chunk data as an object
    //
    // Parameters:  [out]   ConstAncillaryDataPtr&    pAncillaryData     The wrapped chunk data
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //  - VmbErrorNotFound:     No chunk data present
    //
    IMEXPORT VmbErrorType GetAncillaryData( ConstAncillaryDataPtr &pAncillaryData ) const;

    //
    // Method:      GetBuffer()
    //
    // Purpose:     Returns the complete buffer including image and chunk data
    //
    // Parameters:  [out]   VmbUchar_t*      pBuffer        A pointer to the buffer
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetBuffer( VmbUchar_t* &pBuffer );

    //
    // Method:      GetBuffer()
    //
    // Purpose:     Returns the complete buffer including image and chunk data
    //
    // Parameters:  [out]   const VmbUchar_t*      pBuffer  A pointer to the buffer
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetBuffer( const VmbUchar_t* &pBuffer ) const;

    //
    // Method:      GetImage()
    //
    // Purpose:     Returns only the image data
    //
    // Parameters:  [out]   VmbUchar_t*         pBuffer     A pointer to the buffer
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetImage( VmbUchar_t* &pBuffer );

    //
    // Method:      GetImage()
    //
    // Purpose:     Returns only the image data
    //
    // Parameters:  [out]   const VmbUchar_t*    pBuffer    A pointer to the buffer
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetImage( const VmbUchar_t* &pBuffer ) const;

    //
    // Method:      GetReceiveStatus()
    //
    // Purpose:     Returns the receive status of a frame
    //
    // Parameters:  [out]   VmbFrameStatusType&   status    The receive status
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetReceiveStatus( VmbFrameStatusType &status ) const;
    
    //
    // Method:      GetImageSize()
    //
    // Purpose:     Returns the memory size of the image
    //
    // Parameters:  [out]   VmbUint32_t&    imageSize       The size in bytes
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetImageSize( VmbUint32_t &imageSize ) const;
    
    //
    // Method:      GetAncillarySize()
    //
    // Purpose:     Returns memory size of the chunk data
    //
    // Parameters:  [out]   VmbUint32_t&    ancillarySize   The size in bytes
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetAncillarySize( VmbUint32_t &ancillarySize ) const;
    
    //
    // Method:      GetBufferSize()
    //
    // Purpose:     Returns the memory size of the frame buffer holding 
    //              both the image data and the ancillary data
    //
    // Parameters:  [out]   VmbUint32_t&    bufferSize      The size in bytes
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetBufferSize( VmbUint32_t &bufferSize ) const;

    //
    // Method:      GetPixelFormat()
    //
    // Purpose:     Returns the GenICam pixel format
    //
    // Parameters:  [out]   VmbPixelFormatType&      pixelFormat    The GenICam pixel format
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetPixelFormat( VmbPixelFormatType &pixelFormat ) const;

    //
    // Method:      GetWidth()
    //
    // Purpose:     Returns the width of the image
    //
    // Parameters:  [out]   VmbUint32_t&    width       The width in pixels
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetWidth( VmbUint32_t &width ) const;

    //
    // Method:      GetHeight()
    //
    // Purpose:     Returns the height of the image
    //
    // Parameters:  [out]   VmbUint32_t&    height       The height in pixels
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetHeight( VmbUint32_t &height ) const;

    //
    // Method:      GetOffsetX()
    //
    // Purpose:     Returns the x offset of the image
    //
    // Parameters:  [out]   VmbUint32_t&    offsetX     The x offset in pixels
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetOffsetX( VmbUint32_t &offsetX ) const;

    //
    // Method:      GetOffsetY()
    //
    // Purpose:     Returns the y offset of the image
    //
    // Parameters:  [out]   VmbUint32_t&    offsetY     The y offset in pixels
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetOffsetY( VmbUint32_t &offsetY ) const;

    //
    // Method:      GetFrameID()
    //
    // Purpose:     Returns the frame ID
    //
    // Parameters:  [out]   VmbUint64_t&     frameID    The frame ID
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetFrameID( VmbUint64_t &frameID ) const;

    //
    // Method:      GetTimeStamp()
    //
    // Purpose:     Returns the time stamp
    //
    // Parameters:  [out]   VmbUint64_t&     timestamp  The time stamp
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetTimestamp( VmbUint64_t &timestamp ) const;

    bool GetObserver( IFrameObserverPtr &observer ) const;

  private:
    struct Impl;
    Impl *m_pImpl;

    // No default ctor
    Frame();
    // No copy ctor
    Frame( Frame& );
    // No assignment operator
    Frame& operator=( const Frame& );
};

typedef std::vector<FramePtr> FramePtrVector;

}} // namespace AVT::VmbAPI

#endif
