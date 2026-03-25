/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        EnumFeature.cpp

  Description: Implementation of class AVT::VmbAPI::EnumFeature.
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

#include <VimbaCPP/Source/EnumFeature.h>
#include <memory.h>

namespace AVT {
namespace VmbAPI {

EnumFeature::EnumFeature( const VmbFeatureInfo_t *featureInfo, FeatureContainer* const pFeatureContainer )
    :BaseFeature( featureInfo, pFeatureContainer )
{
}

VmbErrorType EnumFeature::GetValue( char * const pStrValue, VmbUint32_t &rnSize ) const
{
    VmbErrorType res;
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    const char* pStrTempValue;
    res = (VmbErrorType)VmbFeatureEnumGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &pStrTempValue );

    if ( VmbErrorSuccess == res )
    {
        VmbUint32_t nLength=0;
        while ( pStrTempValue[nLength] != '\0' )
        {
            ++nLength;
        }

        if ( NULL == pStrValue )
        {
            rnSize = nLength;
        }
        else if ( nLength <= rnSize )
        {
            ::memcpy( pStrValue, pStrTempValue, (size_t)nLength );
            rnSize = nLength;
        }
        else
        {
            res = VmbErrorMoreData;
        }
    }

    return res;
}

VmbErrorType EnumFeature::GetValue( VmbInt64_t &rnValue ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    const char *pName = NULL;
    VmbError_t res = VmbFeatureEnumGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &pName );
    if ( VmbErrorSuccess == res )
    {
        res = VmbFeatureEnumAsInt( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pName, &rnValue );
    }

    return (VmbErrorType)res;
}

VmbErrorType EnumFeature::GetEntry( EnumEntry &rEntry, const char * pStrEntryName ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbFeatureEnumEntry_t entry;
    VmbError_t res = VmbFeatureEnumEntryGet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pStrEntryName, &entry, sizeof( VmbFeatureEnumEntry_t ));
    if ( VmbErrorSuccess == res )
    {
        rEntry = EnumEntry( entry.name, entry.displayName, entry.description, entry.tooltip, entry.sfncNamespace, entry.visibility, entry.intValue );
    }

    return (VmbErrorType)res;
}

VmbErrorType EnumFeature::SetValue( const char *pStrValue )
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureEnumSet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pStrValue );
}

VmbErrorType EnumFeature::SetValue( const VmbInt64_t &rnValue )
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    const char *pName = NULL;
    VmbError_t res = VmbFeatureEnumAsString( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), rnValue, &pName );
    if ( VmbErrorSuccess == res )
    {
        res = VmbFeatureEnumSet( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pName );
    }

    return (VmbErrorType)res;
}

VmbErrorType EnumFeature::GetValues( const char **pRange, VmbUint32_t &rnSize )
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbUint32_t nCount = 0;
    VmbError_t res = VmbFeatureEnumRangeQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), NULL, 0, &nCount );

    if (    VmbErrorSuccess == res
         && 0 < nCount )
    {
        std::vector<const char*> data( nCount );

        res = VmbFeatureEnumRangeQuery( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), &data[0], nCount, &nCount );

        if ( VmbErrorSuccess == res )
        {
            m_EnumStringValues.clear();

            for (   std::vector<const char*>::iterator iter = data.begin();
                    data.end() != iter;
                    ++iter )
            {
                m_EnumStringValues.push_back( std::string( *iter ));
            }

            if ( NULL == pRange )
            {
                rnSize = (VmbUint32_t)m_EnumStringValues.size();
                res = VmbErrorSuccess;
            }
            else if ( m_EnumStringValues.size() <= rnSize )
            {
                VmbUint32_t i = 0;
                for (   StringVector::iterator iter = m_EnumStringValues.begin();
                        m_EnumStringValues.end() != iter;
                        ++iter, ++i )
                {
                    pRange[i] = iter->c_str();
                }
                rnSize = (VmbUint32_t)m_EnumStringValues.size();
                res = VmbErrorSuccess;
            }
            else
            {
                res = VmbErrorMoreData;
            }
        }
    }

    return (VmbErrorType)res;
}

VmbErrorType EnumFeature::GetValues( VmbInt64_t *pValues, VmbUint32_t &rnSize )
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    VmbUint32_t nCount = 0;
    VmbError_t res = GetValues( (const char**)NULL, nCount );

    if (    VmbErrorSuccess == res
         && 0 < nCount )
    {
        std::vector<const char*> data( nCount );

        res = GetValues( &data[0], nCount );

        if ( VmbErrorSuccess == res )
        {
            m_EnumIntValues.clear();

            VmbInt64_t nValue;
            for (   std::vector<const char*>::iterator iter = data.begin();
                    data.end() != iter;
                    ++iter )
            {
                res = VmbFeatureEnumAsInt( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), (*iter), &nValue );

                if ( VmbErrorSuccess == res )
                {
                    m_EnumIntValues.push_back( nValue );
                }
                else
                {
                    m_EnumIntValues.clear();
                    break;
                }
            }

            if ( VmbErrorSuccess == res )
            {
                if ( NULL == pValues )
                {
                    rnSize = (VmbUint32_t)m_EnumIntValues.size();
                }
                else if ( m_EnumIntValues.size() <= rnSize )
                {
                    VmbUint32_t i = 0;
                    for (   Int64Vector::iterator iter = m_EnumIntValues.begin();
                        m_EnumIntValues.end() != iter;
                        ++iter, ++i )
                    {
                        pValues[i] = (*iter);
                    }
                    rnSize = (VmbUint32_t)m_EnumIntValues.size();
                }
                else
                {
                    res = VmbErrorMoreData;
                }
            }
        }
    }

    return (VmbErrorType)res;
}

VmbErrorType EnumFeature::GetEntries( EnumEntry *pEntries, VmbUint32_t &rnSize )
{
    VmbErrorType res = GetValues( (const char**)NULL, rnSize );

    if (    0 < m_EnumStringValues.size()
         && VmbErrorSuccess == res )
    {
        m_EnumEntries.clear();

        for (   StringVector::iterator iter = m_EnumStringValues.begin();
                m_EnumStringValues.end() != iter;
                ++iter )
        {
            EnumEntry entry;
            res = GetEntry( entry, (*iter).c_str() );
            if ( VmbErrorSuccess == res )
            {
                m_EnumEntries.push_back( entry );
            }
            else
            {
                m_EnumEntries.clear();
                break;
            }
        }

        if ( VmbErrorSuccess == res )
        {
            if ( NULL == pEntries )
            {
                rnSize = (VmbUint32_t)m_EnumEntries.size();
            }
            else if ( m_EnumEntries.size() <= rnSize )
            {
                VmbUint32_t i = 0;
                for (   EnumEntryVector::iterator iter = m_EnumEntries.begin();
                        m_EnumEntries.end() != iter;
                        ++iter, ++i )
                {
                    pEntries[i] = (*iter);
                }
                rnSize = (VmbUint32_t)m_EnumIntValues.size();
            }
            else
            {
                res = VmbErrorMoreData;
            }
        }
    }

    return res;
}

VmbErrorType EnumFeature::IsValueAvailable( const char *pStrValue, bool &bAvailable ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    return (VmbErrorType)VmbFeatureEnumIsAvailable( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), pStrValue, &bAvailable );
}

VmbErrorType EnumFeature::IsValueAvailable( const VmbInt64_t nValue, bool &rbAvailable ) const
{
    if ( NULL == m_pFeatureContainer )
    {
        return VmbErrorDeviceNotOpen;
    }

    const char* pName = NULL;
    VmbError_t res = VmbFeatureEnumAsString( m_pFeatureContainer->GetHandle(), m_featureInfo.name.c_str(), nValue, &pName );
    if ( VmbErrorSuccess == res )
    {
        res = IsValueAvailable( pName, rbAvailable );
    }

    return (VmbErrorType)res;
}

}} // namespace AVT::VmbAPI

