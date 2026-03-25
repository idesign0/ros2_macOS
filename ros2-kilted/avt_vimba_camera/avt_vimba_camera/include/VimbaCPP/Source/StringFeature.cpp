/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        StringFeature.cpp

  Description: Implementation of class AVT::VmbAPI::StringFeature.
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

#include <VimbaCPP/Source/StringFeature.h>

namespace AVT {
namespace VmbAPI {

StringFeature::StringFeature( const VmbFeatureInfo_t *featureInfo, FeatureContainer* const pFeatureContainer )
    :   BaseFeature( featureInfo, pFeatureContainer )
{
}

VmbErrorType StringFeature::GetValue( char * const pStrValue, VmbUint32_t &rnLength ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    if ( NULL == pStrValue )
    {
        return (VmbErrorType)VmbFeatureStringMaxlengthQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rnLength );
    }
    else
    {
        return (VmbErrorType)VmbFeatureStringGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pStrValue, rnLength, &rnLength );
    }
}

VmbErrorType StringFeature::SetValue( const char *pStrValue ) 
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureStringSet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pStrValue );
}


}} // namespace AVT::VmbAPI
