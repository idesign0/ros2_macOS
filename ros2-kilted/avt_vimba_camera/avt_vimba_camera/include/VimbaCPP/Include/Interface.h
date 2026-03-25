/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Interface.h

  Description: Definition of class AVT::VmbAPI::Interface.

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

#ifndef AVT_VMBAPI_INTERFACE_H
#define AVT_VMBAPI_INTERFACE_H

#include <VimbaC/Include/VimbaC.h>
#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/FeatureContainer.h>
#include <vector>

namespace AVT {
namespace VmbAPI {

class Interface : public FeatureContainer
{
  public:

    Interface( const VmbInterfaceInfo_t *pInterfaceInfo );

    virtual ~Interface();

    //
    // Method:      Open()
    //
    // Purpose:     Open an interface handle for feature access.
    //
    // Parameters:  none
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorNotFound:      The designated interface cannot be found   
    //
    // Details:   An interface can be opened if interface-specific control is required, such as I/O pins
    //            on a frame grabber card. Control is then possible via feature access methods.
    //
    IMEXPORT virtual VmbErrorType Open();

    //
    // Method:      Close()
    //
    // Purpose:     Close an interface.
    //
    // Parameters:  none    
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //  - VmbErrorApiNotStarted: VmbStartup() was not called before the current command
    //  - VmbErrorBadHandle:     The handle is not valid
    //
    IMEXPORT virtual VmbErrorType Close();

    //
    // Method:      GetID()
    //
    // Purpose:     Gets the ID of an interface.
    //
    // Parameters:  [out]   std::string&        interfaceID          The ID of the interface
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    // Details:    This information remains static throughout the object's lifetime
    //
    VmbErrorType GetID( std::string &interfaceID ) const;

    //
    // Method:      GetType()
    //
    // Purpose:     Gets the type, e.g. FireWire, GigE or USB of an interface.
    //
    // Parameters:  [out]   VmbInterfaceType&   type        The type of the interface
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    //  Details:    This information remains static throughout the object's lifetime
    //
    IMEXPORT VmbErrorType GetType( VmbInterfaceType &type ) const;

    //
    // Method:      GetName()
    //
    // Purpose:     Gets the name of an interface.
    //
    // Parameters:  [out]   std::string&        name        The name of the interface
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    //              This information remains static throughout the object's lifetime
    //
    VmbErrorType GetName( std::string &name ) const;

    //
    // Method:      GetSerialNumber()
    //
    // Purpose:     Gets the serial number of an interface.
    //
    // Parameters:  [out]   std::string&    serialNumber    The serial number of the interface
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    //              This information remains static throughout the object's lifetime
    //
    VmbErrorType GetSerialNumber( std::string &serialNumber ) const;

    //
    // Method:      GetPermittedAccess()
    //
    // Purpose:     Gets the access mode of an interface.
    //
    // Parameters:  [out]   VmbAccessModeType&  accessMode  The possible access mode of the interface
    //
    // Returns:
    //  - VmbErrorSuccess:       If no error
    //
    IMEXPORT VmbErrorType GetPermittedAccess( VmbAccessModeType &accessMode ) const;

  private:
    // Default ctor
    Interface();
    // Copy ctor
    Interface( const Interface& );
    // Assignment operator
    Interface& operator=( const Interface& );

    struct Impl;
    Impl *m_pImpl;

    // Array functions to pass data across DLL boundaries
    IMEXPORT VmbErrorType GetID( char * const pID, VmbUint32_t &length ) const;
    IMEXPORT VmbErrorType GetName( char * const pName, VmbUint32_t &length ) const;
    IMEXPORT VmbErrorType GetSerialNumber( char * const pSerial, VmbUint32_t &length ) const;
};

#include <VimbaCPP/Include/Interface.hpp>

}} // namespace AVT::VmbAPI

#endif
