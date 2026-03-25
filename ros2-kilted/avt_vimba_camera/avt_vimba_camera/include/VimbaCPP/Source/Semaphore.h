/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Semaphore.h

  Description: Definition of an semaphore class.
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

#ifndef AVT_VMBAPI_SEMAPHORE
#define AVT_VMBAPI_SEMAPHORE

#include <VimbaCPP/Include/VimbaCPPCommon.h>

#ifdef WIN32
    #include <windows.h>
#else
    #include <semaphore.h>
#endif

namespace AVT {
namespace VmbAPI {

class Semaphore
{
  public:
    Semaphore( int nInit = 0, int nMax = 1 );
    ~Semaphore();

    void Acquire();
    void Release();

  private:
    // No copy ctor
    Semaphore( const Semaphore &rSemaphore );
    // No assignment
    Semaphore& operator=( const Semaphore& );

#ifdef WIN32
    HANDLE          m_hSemaphore;
#else
    sem_t           m_Semaphore;
#endif
};

}} //namespace AVT::VmbAPI

#endif //AVT_VMBAPI_MUTEX
