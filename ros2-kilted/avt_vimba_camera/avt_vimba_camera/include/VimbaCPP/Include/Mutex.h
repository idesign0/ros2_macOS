/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Mutex.h

  Description: Definition of class AVT::VmbAPI::Mutex.

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

#ifndef AVT_VMBAPI_MUTEX
#define AVT_VMBAPI_MUTEX

#include <VimbaCPP/Include/VimbaCPPCommon.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <pthread.h>
#endif

namespace AVT {
namespace VmbAPI {

class Mutex
{
  public:
    IMEXPORT explicit Mutex( bool bInitLock = false );
    IMEXPORT ~Mutex();

    IMEXPORT void Lock();
    IMEXPORT void Unlock();

  protected:
#ifdef _WIN32
    HANDLE          m_hMutex;
#else
    pthread_mutex_t m_Mutex;
#endif

  private:
    Mutex& operator=( const Mutex& );
    Mutex( const Mutex& );
};

}} //namespace AVT::VmbAPI

#endif //AVT_VMBAPI_MUTEX
