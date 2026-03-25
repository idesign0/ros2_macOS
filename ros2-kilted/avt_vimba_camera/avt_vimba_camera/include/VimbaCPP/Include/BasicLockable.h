/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BasicLockable.h

  Description: Definition of class AVT::VmbAPI::BasicLockable.

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

#ifndef AVT_VMBAPI_BASICLOCKABLE
#define AVT_VMBAPI_BASICLOCKABLE

#include <VimbaCPP/Include/VimbaCPPCommon.h>
#include <VimbaCPP/Include/SharedPointerDefines.h>
#include <VimbaCPP/Include/Mutex.h>

namespace AVT {
namespace VmbAPI {

class BasicLockable
{
  public:
    IMEXPORT BasicLockable();
    IMEXPORT BasicLockable( MutexPtr pMutex );

    IMEXPORT virtual ~BasicLockable();

    MutexPtr& GetMutex();
    const MutexPtr& GetMutex() const;

    void Lock()
    {
        SP_ACCESS(m_pMutex)->Lock();
    }
    void Unlock()
    {
        SP_ACCESS(m_pMutex)->Unlock();
    }
  private:
    MutexPtr m_pMutex;
};

}} //namespace AVT::VmbAPI

#endif 