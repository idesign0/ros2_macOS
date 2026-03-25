/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        CommandFeature.h

  Description: Definition of class AVT::VmbAPI::CommandFeature.
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

#ifndef AVT_VMBAPI_COMMANDFEATURE_H
#define AVT_VMBAPI_COMMANDFEATURE_H

#include <VimbaC/Include/VmbCommonTypes.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Source/BaseFeature.h>
#include <VimbaCPP/Include/FeatureContainer.h>

namespace AVT {
namespace VmbAPI {

class CommandFeature : public BaseFeature 
{
  public:
    CommandFeature( const VmbFeatureInfo_t *featureInfo, FeatureContainer *pFeatureContainer );

    IMEXPORT virtual VmbErrorType RunCommand();

    IMEXPORT virtual VmbErrorType IsCommandDone( bool & isDone ) const;
};

}} // namespace AVT::VmbAPI

#endif
