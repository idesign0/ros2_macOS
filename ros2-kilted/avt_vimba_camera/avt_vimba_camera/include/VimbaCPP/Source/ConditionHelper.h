/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ConditionHelper.h

  Description: Definition of helper class for conditions.
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

#ifndef AVT_VMBAPI_CONDITIONHELPER_H
#define AVT_VMBAPI_CONDITIONHELPER_H

#include <VimbaCPP/Source/Condition.h>

#include <map>

namespace AVT {
namespace VmbAPI {

class ConditionHelper
{
  public:
    ConditionHelper();

    // Waits until writing access has finished and returns true.
    // If exclusive writing access was granted the function exits immediately without locking and returns false
    bool EnterReadLock( BasicLockable &rLockable );
    bool EnterReadLock( MutexPtr &pMutex );
    void ExitReadLock( BasicLockable &rLockable );
    void ExitReadLock( MutexPtr &pMutex );

    // Waits until writing and reading access have finished and returns true.
    // If exclusive writing access was granted the function exits immediately without locking and returns false
    bool EnterWriteLock( BasicLockable &rLockable, bool bExclusive = false );
    bool EnterWriteLock( MutexPtr &pMutex, bool bExclusive = false );
    void ExitWriteLock( BasicLockable &rLockable );
    void ExitWriteLock( MutexPtr &pMutex );

  private:
    Condition                           m_ReadCondition;
    Condition                           m_WriteCondition;
    bool                                m_bIsWritingList;
    bool                                m_bExclusive;
    int                                 m_nNumListReads;
};

}} // namespace AVT::VmbAPI

#endif // CONDITIONHELPER_H