/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AncillaryData.h

  Description: Definition of class AVT::VmbAPI::AncillaryData.

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

#ifndef AVT_VMBAPI_ANCILLARYDATA_H
#define AVT_VMBAPI_ANCILLARYDATA_H

#include <VimbaC/Include/VmbCommonTypes.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/FeatureContainer.h>

namespace AVT {
namespace VmbAPI {

class AncillaryData : public FeatureContainer 
{
  public:
    AncillaryData( VmbFrame_t *pFrame );
    ~AncillaryData();

    //
    // Method:      Open()
    //
    // Purpose:     Opens the ancillary data to allow access to the elements of the ancillary data via feature access.
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //
    // Details:     This function can only succeed if the given frame has been filled by the API.
    //    
    IMEXPORT VmbErrorType Open();

    //
    // Method:      Close()
    //
    // Purpose:     Closes the ancillary data inside a frame.
    //
    // Returns:
    //
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:     The given handle is not valid
    //
    // Details:     After reading the ancillary data and before re-queuing the frame, ancillary data
    //              must be closed.
    //
    IMEXPORT VmbError_t Close();

    //
    // Method:      GetBuffer()
    //
    // Purpose:     Returns the underlying buffer
    //
    // Parameters:  [out]       VmbUchar_t*&        pBuffer     A pointer to the buffer
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetBuffer( VmbUchar_t* &pBuffer );

    //
    // Method:      GetBuffer()
    //
    // Purpose:     Returns the underlying buffer
    //
    // Parameters:  [out]       const VmbUchar_t*&  pBuffer     A pointer to the buffer
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetBuffer( const VmbUchar_t* &pBuffer ) const;

    //
    // Method:      GetSize()
    //
    // Purpose:     Returns the size of the underlying buffer
    //
    // Parameters:  [out]       VmbUint32_t&        size    The size of the buffer
    //
    // Returns:
    //
    //  - VmbErrorSuccess:      If no error
    //
    IMEXPORT VmbErrorType GetSize( VmbUint32_t &size ) const;

  private:
    struct Impl;
    Impl *m_pImpl;

    // No default ctor
    AncillaryData();
    // No copy ctor
    AncillaryData( const AncillaryData& );
    // No assignment operator
    AncillaryData& operator=( const AncillaryData& );
};

}} // namespace AVT::VmbAPI

#endif
