/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Mutex.cpp

  Description: Implementation of class AVT::VmbAPI::Mutex.

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

#include <VimbaCPP/Include/Mutex.h>
#include <VimbaCPP/Include/LoggerDefines.h>

namespace AVT {
namespace VmbAPI {

Mutex::Mutex( bool bInitLock )
#ifdef WIN32
    :   m_hMutex( NULL )
#endif
{
#ifdef WIN32
    m_hMutex = CreateMutex( NULL, FALSE, NULL );
    if( NULL == m_hMutex )
    {
        LOG_FREE_TEXT( "Could not create mutex." );
        throw std::bad_alloc();
    }
#else
    pthread_mutex_init(&m_Mutex, NULL);
#endif

    if( true == bInitLock )
    {
        Lock();
    }
}

Mutex::~Mutex()
{  
#ifdef WIN32
    CloseHandle( m_hMutex );
#else
    pthread_mutex_destroy(&m_Mutex);
#endif
}

Mutex::Mutex( const Mutex& )
{
    // No copy ctor
}

Mutex& Mutex::operator=( const Mutex& )
{
    // No assignment operator
    return *this;
}

void Mutex::Lock()
{
#ifdef WIN32
    WaitForSingleObject( m_hMutex, INFINITE );
#else
    pthread_mutex_lock( &m_Mutex );
#endif
}

void Mutex::Unlock()
{  
#ifdef WIN32
    ReleaseMutex( m_hMutex );
#else
    pthread_mutex_unlock( &m_Mutex );
#endif
}

}} //namespace AVT::VmbAPI
