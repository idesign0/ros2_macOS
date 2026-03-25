/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        IRegisterDevice.h

  Description: Definition of interface AVT::VmbAPI::IRegisterDevice.

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

#ifndef AVT_VMBAPI_IREGISTERDEVICE_H
#define AVT_VMBAPI_IREGISTERDEVICE_H

#include <VimbaC/Include/VmbCommonTypes.h>
#include <vector>

namespace AVT {
namespace VmbAPI {

class IRegisterDevice 
{
  public:

    virtual ~IRegisterDevice() {};

    //
    // Method:      ReadRegisters()
    //
    // Purpose:     Reads one or more registers consecutively. The number of registers to be read is determined by the number of provided addresses.
    //
    // Parameters:  [in ]   const Uint64Vector&   addresses     A list of register addresses
    //              [out]   Uint64Vector&         buffer        The returned data as vector
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested registers have been read
    //  - VmbErrorBadParameter: Vectors "addresses" and/or "buffer" are empty.
    //  - VmbErrorIncomplete:   If at least one, but not all registers have been read. See overload ReadRegisters( const Uint64Vector&, Uint64Vector&, VmbUint32_t& ).
    //
    virtual VmbErrorType ReadRegisters( const Uint64Vector &addresses, Uint64Vector &buffer ) const = 0;

    //
    // Method:      ReadRegisters()
    //
    // Purpose:     Same as ReadRegisters( const Uint64Vector&, Uint64Vector& ), but returns the number of successful read operations in case of an error.
    //
    // Parameters:  [in ]   const Uint64Vector&   addresses         A list of register addresses
    //              [out]   Uint64Vector&         buffer            The returned data as vector
    //              [out]   VmbUint32_t&          completedReads    The number of successfully read registers
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested registers have been read
    //  - VmbErrorBadParameter: Vectors "addresses" and/or "buffer" are empty.
    //  - VmbErrorIncomplete:   If at least one, but not all registers have been read.
    //
    virtual VmbErrorType ReadRegisters( const Uint64Vector &addresses, Uint64Vector &buffer, VmbUint32_t &completedReads ) const = 0;
    
    //
    // Method:      WriteRegisters()
    //
    // Purpose:     Writes one or more registers consecutively. The number of registers to be written is determined by the number of provided addresses.
    //
    // Parameters:  [in ]    const Uint64Vector&   addresses    A list of register addresses
    //              [in ]    const Uint64Vector&   buffer       The data to write as vector
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested registers have been written
    //  - VmbErrorBadParameter: Vectors "addresses" and/or "buffer" are empty.
    //  - VmbErrorIncomplete:   If at least one, but not all registers have been written. See overload WriteRegisters( const Uint64Vector&, const Uint64Vector&, VmbUint32_t& ).
    //
    virtual VmbErrorType WriteRegisters( const Uint64Vector &addresses, const Uint64Vector &buffer ) = 0;

    //
    // Method:      WriteRegisters()
    //
    // Purpose:     Same as WriteRegisters( const Uint64Vector&, const Uint64Vector& ), but returns the number of successful write operations in case of an error VmbErrorIncomplete.
    //
    // Parameters:  [in ]   const Uint64Vector&   addresses         A list of register addresses
    //              [in ]   const Uint64Vector&   buffer            The data to write as vector
    //              [out]   VmbUint32_t&          completedWrites   The number of successfully written registers
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested registers have been written
    //  - VmbErrorBadParameter: Vectors "addresses" and/or "buffer" are empty.
    //  - VmbErrorIncomplete:   If at least one, but not all registers have been written.
    //
    virtual VmbErrorType WriteRegisters( const Uint64Vector &addresses, const Uint64Vector &buffer, VmbUint32_t &completedWrites ) = 0;
    
    //
    // Method:      ReadMemory()
    //
    // Purpose:     Reads a block of memory. The number of bytes to read is determined by the size of the provided buffer.
    //
    // Parameters:  [in ]   const VmbUint64_t&   address    The address to read from
    //              [out]   UcharVector&         buffer     The returned data as vector
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested bytes have been read
    //  - VmbErrorBadParameter: Vector "buffer" is empty.
    //  - VmbErrorIncomplete:   If at least one, but not all bytes have been read. See overload ReadMemory( const VmbUint64_t&, UcharVector&, VmbUint32_t& ).
    //
    virtual VmbErrorType ReadMemory( const VmbUint64_t &address, UcharVector &buffer ) const = 0;

    //
    // Method:      ReadMemory()
    //
    // Purpose:     Same as ReadMemory( const Uint64Vector&, UcharVector& ), but returns the number of bytes successfully read in case of an error VmbErrorIncomplete.
    //
    // Parameters:  [in ]   const VmbUint64_t&   address        The address to read from
    //              [out]   UcharVector&         buffer         The returned data as vector
    //              [out]   VmbUint32_t&         sizeComplete   The number of successfully read bytes
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested bytes have been read
    //  - VmbErrorBadParameter: Vector "buffer" is empty.
    //  - VmbErrorIncomplete:   If at least one, but not all bytes have been read.
    //
    virtual VmbErrorType ReadMemory( const VmbUint64_t &address, UcharVector &buffer, VmbUint32_t &sizeComplete ) const = 0;
    
    //
    // Method:      WriteMemory()
    //
    // Purpose:     Writes a block of memory. The number of bytes to write is determined by the size of the provided buffer.
    //
    // Parameters:  [in]    const VmbUint64_t&   address    The address to write to
    //              [in]    const UcharVector&   buffer     The data to write as vector
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested bytes have been written
    //  - VmbErrorBadParameter: Vector "buffer" is empty.
    //  - VmbErrorIncomplete:   If at least one, but not all bytes have been written. See overload WriteMemory( const VmbUint64_t&, const UcharVector&, VmbUint32_t& ).
    //
    virtual VmbErrorType WriteMemory( const VmbUint64_t &address, const UcharVector &buffer ) = 0;

    //
    // Method:      WriteMemory()
    //
    // Purpose:     Same as WriteMemory( const Uint64Vector&, const UcharVector& ), but returns the number of bytes successfully written in case of an error VmbErrorIncomplete.
    //
    // Parameters:  [in]    const VmbUint64_t&   address        The address to write to
    //              [in]    const UcharVector&   buffer         The data to write as vector
    //              [out]   VmbUint32_t&         sizeComplete   The number of successfully written bytes
    //
    // Returns:
    //  - VmbErrorSuccess:      If all requested bytes have been written
    //  - VmbErrorBadParameter: Vector "buffer" is empty.
    //  - VmbErrorIncomplete:   If at least one, but not all bytes have been written.
    //
    virtual VmbErrorType WriteMemory( const VmbUint64_t &address, const UcharVector &buffer, VmbUint32_t &sizeComplete ) = 0;
};

}} // namespace AVT::VmbAPI

#endif
