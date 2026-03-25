/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        EnumEntry.hpp

  Description: Inline wrapper functions for class AVT::VmbAPI::EnumEntry.

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

#ifndef AVT_VMBAPI_ENUMENTRY_HPP
#define AVT_VMBAPI_ENUMENTRY_HPP

//
// Inline wrapper functions that allocate memory for STL objects in the application's context
// and to pass data across DLL boundaries using arrays
//
inline VmbErrorType EnumEntry::GetName( std::string &rStrName ) const
{
    VmbErrorType res;
    VmbUint32_t nLength;

    res = GetName( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpName( nLength + 1, '\0' );
                res = GetName( &tmpName[0], nLength );
                if ( VmbErrorSuccess == res )
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

inline VmbErrorType EnumEntry::GetDisplayName( std::string &rStrDisplayName ) const
{
    VmbErrorType res;
    VmbUint32_t nLength;

    res = GetDisplayName( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpName( nLength + 1, '\0' );
                res = GetDisplayName( &tmpName[0], nLength );
                if ( VmbErrorSuccess == res )
                {
                    rStrDisplayName = &*tmpName.begin();
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

inline VmbErrorType EnumEntry::GetDescription( std::string &rStrDescription ) const
{
    VmbErrorType res;
    VmbUint32_t nLength;

    res = GetDescription( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpDescription( nLength + 1, '\0' );
                res = GetDescription( &tmpDescription[0], nLength );
                if ( VmbErrorSuccess == res )
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

inline VmbErrorType EnumEntry::GetTooltip( std::string &rStrTooltip ) const
{
    VmbErrorType res;
    VmbUint32_t nLength;

    res = GetTooltip( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpTooltip( nLength + 1, '\0' );
                res = GetTooltip( &tmpTooltip[0], nLength );
                if ( VmbErrorSuccess == res )
                {
                    rStrTooltip = &*tmpTooltip.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrTooltip.clear();
        }
    }

    return res;
}

inline VmbErrorType EnumEntry::GetSFNCNamespace( std::string &rStrNamespace ) const
{
    VmbErrorType res;
    VmbUint32_t nLength;

    res = GetSFNCNamespace( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if ( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpNamespace( nLength + 1, '\0' );
                res = GetSFNCNamespace( &tmpNamespace[0], nLength );
                if ( VmbErrorSuccess == res )
                {
                    rStrNamespace =&*tmpNamespace.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrNamespace.clear();
        }
    }

    return res;
}

#endif
