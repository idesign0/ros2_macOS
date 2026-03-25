/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Condition.h

  Description: Definition of a condition class.
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

#ifndef AVT_VMBAPI_CONDITION_H
#define AVT_VMBAPI_CONDITION_H

#include <VimbaCPP/Include/Mutex.h>
#include <VimbaCPP/Source/Semaphore.h>
#include <VimbaCPP/Include/BasicLockable.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>

namespace AVT {
namespace VmbAPI {

class Condition
{
  private:
    unsigned long           m_nReleaseNumber;
    unsigned long           m_nWaiterNumber;
    bool                    m_bLocked;
    SP_DECL( Semaphore )    m_Semaphore;        // A binary semaphore (non recursive mutex)

  public:
    Condition();

    void Wait( const BasicLockable &rLockable );
    void Wait( const MutexPtr &rMutex );

    void Signal( bool bSingle = false );
};

}} // namespace AVT::VmbAPI

#endif //CONDITION_H