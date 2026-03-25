/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FloatFeature.cpp

  Description: Implementation of class AVT::VmbAPI::FloatFeature.
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

#include <VimbaCPP/Source/FloatFeature.h>

namespace AVT {
namespace VmbAPI {

FloatFeature::FloatFeature( const VmbFeatureInfo_t *featureInfo, FeatureContainer* const pFeatureContainer )
    :   BaseFeature( featureInfo, pFeatureContainer )
{
}

VmbErrorType FloatFeature::GetValue( double &rfValue ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureFloatGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rfValue );
}

VmbErrorType FloatFeature::SetValue( const double &rfValue ) 
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureFloatSet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), rfValue );
}

VmbErrorType FloatFeature::GetRange( double &rfMinimum, double &rfMaximum ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureFloatRangeQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &rfMinimum, &rfMaximum );
}

VmbErrorType FloatFeature::HasIncrement( VmbBool_t &incrementSupported ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }
    VmbBool_t hasIncrement;
    VmbError_t Result =VmbFeatureFloatIncrementQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(),&hasIncrement, NULL );
    if( VmbErrorSuccess == Result)
    {
        incrementSupported = hasIncrement;
        return VmbErrorSuccess;
    }
    return static_cast<VmbErrorType>( Result);
}

VmbErrorType FloatFeature::GetIncrement( double &rnIncrement ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }
    VmbBool_t hasIncrement;
    VmbError_t Result =VmbFeatureFloatIncrementQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(),&hasIncrement, &rnIncrement );
    if( VmbErrorSuccess == Result && !hasIncrement)
    {
        return VmbErrorNotImplemented;
    }
    return static_cast<VmbErrorType>( Result);
}

}} // namespace AVT::VmbAPI
