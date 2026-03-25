/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        IntFeature.h

  Description: Definition of class AVT::VmbAPI::IntFeature.
               Intended for use in the implementation of Vimba CPP API.

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

#ifndef AVT_VMBAPI_INTFEATURE_H
#define AVT_VMBAPI_INTFEATURE_H

#include <VimbaC/Include/VimbaC.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Source/BaseFeature.h>
#include <VimbaCPP/Include/FeatureContainer.h>

namespace AVT {
namespace VmbAPI {

class IntFeature : public BaseFeature 
{
  public:
    IntFeature( const VmbFeatureInfo_t *featureInfo, FeatureContainer* const pFeatureContainer );

    IMEXPORT virtual VmbErrorType GetValue( VmbInt64_t &value ) const;

    IMEXPORT virtual VmbErrorType SetValue( const VmbInt64_t &value );

    IMEXPORT virtual VmbErrorType GetRange( VmbInt64_t &minimum, VmbInt64_t &maximum ) const;

    IMEXPORT virtual VmbErrorType HasIncrement( VmbBool_t &incrementSupported) const;

    IMEXPORT virtual VmbErrorType GetIncrement( VmbInt64_t &increment ) const;
};

}} // namespace AVT::VmbAPI

#endif
