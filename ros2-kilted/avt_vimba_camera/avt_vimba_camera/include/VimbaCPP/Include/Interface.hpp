/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Interface.hpp

  Description: Inline wrapper functions for class AVT::VmbAPI::Interface.

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

#ifndef AVT_VMBAPI_INTERFACE_HPP
#define AVT_VMBAPI_INTERFACE_HPP

//
// Inline wrapper functions that allocate memory for STL objects in the application's context
// and to pass data across DLL boundaries using arrays
//

// HINT: This information remains static throughout the object's lifetime
inline VmbErrorType Interface::GetID( std::string &rStrID ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetID( NULL, nLength );
    if ( VmbErrorSuccess == res)
    {
        if( 0 != nLength)
        {
            try
            {
                std::vector<std::string::value_type> tmpID( nLength + 1, '\0' );
                res     = GetID( &tmpID[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrID  = &*tmpID.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrID.clear();
        }
    }

    return res;
}

// HINT: This information remains static throughout the object's lifetime
inline VmbErrorType Interface::GetName( std::string &rStrName ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetName( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nLength )
        {
            try
            {
                std::vector<std::string::value_type> tmpName( nLength + 1, '\0' );
                res         = GetName( &tmpName[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrName    = &*tmpName.begin();
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

// HINT: This information remains static throughout the object's lifetime
inline VmbErrorType Interface::GetSerialNumber( std::string &rStrSerial ) const
{
    VmbErrorType    res;
    VmbUint32_t     nLength;

    res = GetSerialNumber( NULL, nLength );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nLength)
        {
            try
            {
                std::vector<std::string::value_type> tmpSerial( nLength + 1, '\0');
                res         = GetSerialNumber( &tmpSerial[0], nLength );
                if( VmbErrorSuccess == res )
                {
                    rStrSerial  = &*tmpSerial.begin();
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rStrSerial.clear();
        }
    }

    return res;
}

#endif
