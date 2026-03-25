/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

  -----------------------------------------------------------------------------

  File:        VimbaSystem.hpp

  Description: Inline wrapper functions for class AVT::VmbAPI::VimbaSystem.

  -----------------------------------------------------------------------------

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

#ifndef AVT_VMBAPI_VIMBASYSTEM_HPP
#define AVT_VMBAPI_VIMBASYSTEM_HPP

//
// Inline wrapper functions that allocate memory for STL objects in the application's context
// and to pass data across DLL boundaries using arrays
//
inline VmbErrorType VimbaSystem::GetInterfaces( InterfacePtrVector &rInterfaces )
{
    VmbErrorType    res;
    VmbUint32_t     nSize;

    res = GetInterfaces( NULL, nSize );
    if ( VmbErrorSuccess == res )
    {
        if( 0 != nSize)
        {
            try
            {
                InterfacePtrVector tmpInterfaces( nSize );
                res = GetInterfaces( &tmpInterfaces[0], nSize );
                if( VmbErrorSuccess == res )
                {
                    rInterfaces.swap( tmpInterfaces);
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rInterfaces.clear();
        }
    }

    return res;
}

inline VmbErrorType VimbaSystem::GetCameras( CameraPtrVector &rCameras )
{
    VmbErrorType    res;
    VmbUint32_t     nSize;

    res = GetCameras( NULL, nSize );
    if (    VmbErrorSuccess == res)
    {
        if( 0 != nSize)
        {
            try
            {
                CameraPtrVector tmpCameras( nSize );
                res = GetCameras( &tmpCameras[0], nSize );
                if( VmbErrorSuccess == res )
                {
                    if( nSize < tmpCameras.size() )
                    {
                        tmpCameras.resize( nSize);
                    }
                    rCameras.swap( tmpCameras );
                }
            }
            catch(...)
            {
                return VmbErrorResources;
            }
        }
        else
        {
            rCameras.clear();
        }
    }

    return res;
}

#endif
