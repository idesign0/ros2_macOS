/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Semaphore.cpp

  Description: Implementation of an semaphore class.
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

#include <math.h>

#include <VimbaCPP/Source/Semaphore.h>
#include <VimbaCPP/Include/LoggerDefines.h>

namespace AVT {
namespace VmbAPI {

Semaphore::Semaphore( int nInit, int nMax )
#ifdef WIN32
    :   m_hSemaphore( NULL )
#endif
{
#ifdef WIN32
    m_hSemaphore = CreateSemaphore( NULL, nInit, nMax, NULL );
    if( NULL == m_hSemaphore )
    {
        LOG_FREE_TEXT( "Could not create semaphore." );
        throw std::bad_alloc();
    }
#else
    sem_init( &m_Semaphore, false, (unsigned int)nInit );
#endif
}

Semaphore::Semaphore( const Semaphore& )
{
    // No compiler generated copy ctor
}

Semaphore& Semaphore::operator=( const Semaphore& )
{
    // No assignment operator
    return *this;
}

Semaphore::~Semaphore()
{  
#ifdef WIN32
    CloseHandle( m_hSemaphore );
#else
    sem_destroy( &m_Semaphore );
#endif
}

void Semaphore::Acquire()
{
#ifdef WIN32
    WaitForSingleObject( m_hSemaphore, INFINITE );
#else
    sem_wait( &m_Semaphore );
#endif
}

void Semaphore::Release()
{
#ifdef WIN32
    ReleaseSemaphore( m_hSemaphore, 1, NULL );
#else
    sem_post( &m_Semaphore );
#endif
}

} //namespace VmbAPI
} //namespace AVT