/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Feature.hpp

  Description: Inline wrapper functions for class AVT::VmbAPI::Feature.

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

#ifndef AVT_VMBAPI_FEATURE_HPP
#define AVT_VMBAPI_FEATURE_HPP
//
// Inline wrapper functions that allocate memory for STL objects in the application's context
// and to pass data across DLL boundaries using arrays
//
inline VmbErrorType Feature::GetValues( StringVector &rValues )
{
    VmbErrorType    res;
    VmbUint32_t     nSize;

    res = GetValues( (const char **)NULL, nSize );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nSize)
        {
            try
            {
                std::vector<const char*> data( nSize );
                res = GetValues( &data[0], nSize );
                if ( VmbErrorSuccess == res )
                {
                    StringVector tmpValues( data.size() );
                    std::copy( data.begin(), data.end(), tmpValues.begin() );
                    rValues.swap( tmpValues);
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rValues.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetEntries( EnumEntryVector &rEntries )
{
    VmbErrorType    res;
    VmbUint32_t     nSize;

    res = GetEntries( (EnumEntry*)NULL, nSize );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nSize )
        {
            try
            {
                EnumEntryVector tmpEntries( nSize );
                res = GetEntries( &tmpEntries[0], nSize );
                if( VmbErrorSuccess == res)
                {
                    rEntries.swap( tmpEntries );
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rEntries.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetValues( Int64Vector &rValues )
{
    VmbErrorType res;
    VmbUint32_t nSize;

    res = GetValues( (VmbInt64_t*)NULL, nSize );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nSize)
        {
            try
            {
                Int64Vector tmpValues( nSize );
                res = GetValues( &tmpValues[0], nSize );
                if( VmbErrorSuccess == res)
                {
                    rValues.swap( tmpValues );
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rValues.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetValue( std::string &rStrValue ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetValue( (char * const)NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpValue( nLength + 1, '\0' );
                res = GetValue( &tmpValue[0], nLength );
                if ( VmbErrorSuccess == res )
                {
                    rStrValue = &*tmpValue.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrValue.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetValue( UcharVector &rValue ) const
{
    VmbUint32_t i;
    return GetValue( rValue, i );
}
inline VmbErrorType Feature::GetValue( UcharVector &rValue, VmbUint32_t &rnSizeFilled ) const
{
    VmbErrorType    res;
    VmbUint32_t     nSize;

    res = GetValue( NULL, nSize, rnSizeFilled );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nSize)
        {
            try
            {
                UcharVector tmpValue( nSize );
                res = GetValue( &tmpValue[0], nSize, rnSizeFilled );
                if( VmbErrorSuccess == res )
                {
                    rValue.swap( tmpValue);
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rValue.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::SetValue( const UcharVector &rValue )
{
    if ( rValue.empty() )
    {
        return VmbErrorBadParameter;
    }
    return SetValue( &rValue[0], (VmbUint32_t)rValue.size() );
}

inline VmbErrorType Feature::GetName( std::string &rStrName ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetName( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpName( nLength + 1, '\0' );
                res = GetName( &tmpName[0], nLength );
                if( VmbErrorSuccess == res)
                {
                    rStrName = &*tmpName.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrName.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetDisplayName( std::string &rStrDisplayName ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetDisplayName( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpDisplayName( nLength + 1, '\0' );
                res = GetDisplayName( &tmpDisplayName[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrDisplayName = &*tmpDisplayName.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrDisplayName.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetCategory( std::string &rStrCategory ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetCategory( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpCategory( nLength + 1, '\0' );
                res = GetCategory( &tmpCategory[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrCategory = &*tmpCategory.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrCategory.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetUnit( std::string &rStrUnit ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetUnit( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpUnit( nLength + 1, '\0' );
                res = GetUnit( &tmpUnit[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrUnit = &*tmpUnit.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrUnit.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetRepresentation( std::string &rStrRepresentation ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetRepresentation( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpRepresentation( nLength + 1, '\0' );
                res = GetRepresentation( &tmpRepresentation[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrRepresentation = &*tmpRepresentation.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrRepresentation.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetToolTip( std::string &rStrToolTip ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetToolTip( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpToolTip( nLength + 1, '\0');
                res = GetToolTip( &tmpToolTip[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrToolTip = &*tmpToolTip.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrToolTip.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetDescription( std::string &rStrDescription ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetDescription( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpDescription( nLength + 1, '\0');
                res = GetDescription( &tmpDescription[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrDescription = &*tmpDescription.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrDescription.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetSFNCNamespace( std::string &rStrSFNCNamespace ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetSFNCNamespace( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpSFNCNamespace( nLength + 1, '\0' );
                res = GetSFNCNamespace( &tmpSFNCNamespace[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrSFNCNamespace = &*tmpSFNCNamespace.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrSFNCNamespace.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetAffectedFeatures( FeaturePtrVector &rAffectedFeatures )
{
    VmbErrorType    res;
    VmbUint32_t     nSize;

    res = GetAffectedFeatures( NULL, nSize );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nSize)
        {
            try
            {
                FeaturePtrVector tmpAffectedFeatures( nSize );
                res = GetAffectedFeatures( &tmpAffectedFeatures[0], nSize );
                if( VmbErrorSuccess == res )
                {
                    rAffectedFeatures.swap( tmpAffectedFeatures );
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rAffectedFeatures.clear();
        }
    }

    return res;
}

inline VmbErrorType Feature::GetSelectedFeatures( FeaturePtrVector &rSelectedFeatures )
{
    VmbErrorType    res;
    VmbUint32_t     nSize;

    res = GetSelectedFeatures( NULL, nSize );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nSize )
        {
            try
            {
                FeaturePtrVector tmpSelectedFeatures( nSize );
                res = GetSelectedFeatures( &tmpSelectedFeatures[0], nSize );
                if( VmbErrorSuccess == res )
                {
                    rSelectedFeatures.swap ( tmpSelectedFeatures );
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rSelectedFeatures.clear();
        }
    }

    return res;
}

#endif
