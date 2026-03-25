/*=============================================================================
  Copyright (C) 2012 - 2017 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        VimbaCPPCommon.h

  Description: Common type definitions used in Vimba CPP API.

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

#ifndef AVT_VMBAPI_CPPCOMMON_H
#define AVT_VMBAPI_CPPCOMMON_H

#if defined (_WIN32)
    #if defined AVT_VMBAPI_CPP_EXPORTS          // DLL exports
        #define IMEXPORT __declspec(dllexport)
    #elif defined AVT_VMBAPI_CPP_LIB            // static LIB
        #define IMEXPORT
    #else                                       // import
        #define IMEXPORT __declspec(dllimport)
    #endif
#elif defined (__GNUC__) && (__GNUC__ >= 4) && defined (__ELF__)
    #define IMEXPORT
#elif defined (__APPLE__)
    #define IMEXPORT
#else
    #error Unknown platform, file needs adaption
#endif

#include <vector>
#include <string>
#include "VimbaC/Include/VmbCommonTypes.h"

namespace AVT {
namespace VmbAPI {

enum UpdateTriggerType
{
    UpdateTriggerPluggedIn           = 0,           // A new camera was discovered by Vimba
    UpdateTriggerPluggedOut          = 1,           // A camera has disappeared from the bus
    UpdateTriggerOpenStateChanged    = 3            // The possible opening mode of a camera has changed (e.g., because it was opened by another application)
};

typedef std::vector<VmbUint64_t>    Uint64Vector;
typedef std::vector<VmbInt64_t>     Int64Vector;
typedef std::vector<VmbUchar_t>     UcharVector;
typedef std::vector<std::string>    StringVector;
class EnumEntry;
typedef std::vector<EnumEntry>      EnumEntryVector;

}} // AVT::VmbAPI

#endif