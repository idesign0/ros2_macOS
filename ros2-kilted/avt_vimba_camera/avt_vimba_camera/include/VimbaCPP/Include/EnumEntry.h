/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        EnumEntry.h

  Description:  Definition of class AVT::VmbAPI::EnumEntry.
                An EnumEntry consists of
                Name
                DisplayName
                Value
                of one particular enumeration

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

#ifndef AVT_VMBAPI_ENUMENTRY_H
#define AVT_VMBAPI_ENUMENTRY_H

#include <string>

#include <VimbaC/Include/VimbaC.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>

namespace AVT {
namespace VmbAPI {

class EnumEntry
{
  public:
    //
    // Method:      EnumEntry constructor
    //
    // Purpose:     Creates an instance of class EnumEntry
    //
    // Parameters:
    //
    // [in ]    const char*             pName           The name of the enum
    // [in ]    const char*             pDisplayName    The declarative name of the enum
    // [in ]    const char*             pDescription    The description of the enum
    // [in ]    const char*             pTooltip        A tooltip that can be used by a GUI
    // [in ]    const char*             pSNFCNamespace  The SFNC namespace of the enum
    // [in ]    VmbFeatureVisibility_t  visibility      The visibility of the enum
    // [in ]    VmbInt64_t              value           The integer value of the enum
    //
    EnumEntry(  const char              *pName,
                const char              *pDisplayName,
                const char              *pDescription,
                const char              *pTooltip,
                const char              *pSNFCNamespace,
                VmbFeatureVisibility_t  visibility,
                VmbInt64_t              value);

    //
    // Method:      EnumEntry constructor
    //
    // Purpose:     Creates an instance of class EnumEntry
    //
    IMEXPORT EnumEntry();
    //
    // Method:      EnumEntry copy constructor
    //
    // Purpose:     Creates a copy of class EnumEntry
    //
    IMEXPORT EnumEntry( const EnumEntry &other);
    
    //
    // Method:      EnumEntry assignment operator
    //
    // Purpose:     assigns EnumEntry to existing instance
    //
    IMEXPORT EnumEntry& operator=( const EnumEntry&o);

    //
    // Method:      EnumEntry destructor
    //
    // Purpose:     Destroys an instance of class EnumEntry
    //
    IMEXPORT virtual ~EnumEntry();

    //
    // Method:      GetName()
    //
    // Purpose:     Gets the name of an enumeration
    //
    // Parameters:
    //
    // [out]        std::string& name   The name of the enumeration
    //
    VmbErrorType GetName( std::string &name ) const;

    //
    // Method:      GetDisplayName()
    //
    // Purpose:     Gets a more declarative name of an enumeration
    //
    // Parameters:
    //
    // [out]        std::string& displayName    The display name of the enumeration
    //
    VmbErrorType GetDisplayName( std::string &displayName ) const;

    //
    // Method:      GetDescription()
    //
    // Purpose:     Gets the description of an enumeration
    //
    // Parameters:
    //
    // [out]        std::string& description    The description of the enumeration
    //
    VmbErrorType GetDescription( std::string &description ) const;

    //
    // Method:      GetTooltip()
    //
    // Purpose:     Gets a tooltip that can be used as pop up help in a GUI
    //
    // Parameters:
    //
    // [out]        std::string& tooltip    The tooltip as string
    //
    VmbErrorType GetTooltip( std::string &tooltip ) const;

    //
    // Method:      GetValue()
    //
    // Purpose:     Gets the integer value of an enumeration
    //
    // Parameters:
    //
    // [out]        VmbInt64_t& value   The integer value of the enumeration
    //
    IMEXPORT    VmbErrorType GetValue( VmbInt64_t &value ) const;

    //
    // Method:      GetVisibility()
    //
    // Purpose:     Gets the visibility of an enumeration
    //
    // Parameters:
    //
    // [out]        VmbFeatureVisibilityType&   value   The visibility of the enumeration
    //
    IMEXPORT    VmbErrorType GetVisibility( VmbFeatureVisibilityType &value ) const;

    //
    // Method:      GetSNFCNamespace()
    //
    // Purpose:     Gets the standard feature naming convention namespace of the enumeration
    //
    // Parameters:
    //
    // [out]        std::string& sFNCNamespace    The feature's SFNC namespace
    //
    VmbErrorType GetSFNCNamespace( std::string &sFNCNamespace ) const;

  private:
    struct PrivateImpl;
    PrivateImpl           *m_pImpl;
    // Array functions to pass data across DLL boundaries
    IMEXPORT VmbErrorType GetName( char * const pName, VmbUint32_t &size ) const;
    IMEXPORT VmbErrorType GetDisplayName( char * const pDisplayName, VmbUint32_t &size ) const;
    IMEXPORT VmbErrorType GetTooltip( char * const pStrTooltip, VmbUint32_t &size ) const;
    IMEXPORT VmbErrorType GetDescription( char * const pStrDescription, VmbUint32_t &size ) const;
    IMEXPORT VmbErrorType GetSFNCNamespace( char * const pStrNamespace, VmbUint32_t &size ) const;

};

#include <VimbaCPP/Include/EnumEntry.hpp>

}} // namespace AVT::VmbAPI

#endif
