/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        IntFeature.cpp

  Description: Implementation of class AVT::VmbAPI::IntFeature.
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

#include <VimbaCPP/Source/IntFeature.h>

namespace AVT {
namespace VmbAPI {

IntFeature::IntFeature( const VmbFeatureInfo_t *featureInfo, FeatureContainer* const pFeatureContainer )
    :   BaseFeature( featureInfo, pFeatureContainer )
{
}

VmbErrorType IntFeature::GetValue( VmbInt64_t &rnValue ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureIntGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rnValue );
}

VmbErrorType IntFeature::SetValue( const VmbInt64_t &rnValue )
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureIntSet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), rnValue );
}

VmbErrorType IntFeature::GetRange( VmbInt64_t &rnMinimum, VmbInt64_t &rnMaximum ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureIntRangeQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rnMinimum, &rnMaximum );
}

VmbErrorType IntFeature::HasIncrement( VmbBool_t & incrementSupported) const
{
    incrementSupported = VmbBoolTrue;
    return VmbErrorSuccess;
}
VmbErrorType IntFeature::GetIncrement( VmbInt64_t &rnIncrement ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureIntIncrementQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rnIncrement );
}


}} // namespace AVT::VmbAPI
