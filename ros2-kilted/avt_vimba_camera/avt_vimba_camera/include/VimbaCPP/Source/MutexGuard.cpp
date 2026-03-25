/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        MutexGuard.cpp

  Description: Implementation of a mutex helper class for locking and unlocking.
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

#include <VimbaCPP/Source/MutexGuard.h>

#include <VimbaCPP/Include/VimbaSystem.h>

namespace AVT {
namespace VmbAPI {

MutexGuard::MutexGuard()
    : m_pMutex( NULL )
{
}

MutexGuard::MutexGuard( MutexPtr &pMutex )
{
    if ( SP_ISNULL( pMutex ))
    {
        LOG_FREE_TEXT( "No mutex passed." );
    }
    else
    {
        m_pMutex = SP_ACCESS(pMutex );
        Protect( );
    }
}

MutexGuard::MutexGuard( BasicLockablePtr pLockable )
{
    if ( SP_ISNULL( pLockable ))
    {
        LOG_FREE_TEXT( "No mutex passed." );
    }
    else
    {
        m_pMutex = SP_ACCESS(SP_ACCESS(pLockable)->GetMutex());
        Protect( );
    }
}

MutexGuard::MutexGuard( const BasicLockable &rLockable )
{
    m_pMutex = SP_ACCESS(rLockable.GetMutex() );
    Protect( );
}

MutexGuard::~MutexGuard()
{
    Release();
}

void MutexGuard::Protect(  )
{
    if( m_pMutex == NULL )
    {
        LOG_FREE_TEXT( "No mutex passed." );
        return;
    }
    m_pMutex->Lock();
}


bool MutexGuard::Release()
{
    if( m_pMutex == NULL)
    {
        return false;
    }

    m_pMutex ->Unlock();
    m_pMutex = NULL;

    return true;
}

}} //namespace AVT::VmbAPI
