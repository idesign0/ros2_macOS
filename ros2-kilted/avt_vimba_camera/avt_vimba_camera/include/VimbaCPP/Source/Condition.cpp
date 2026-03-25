/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Condition.cpp

  Description: Implementation of a condition class.
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

#include <VimbaCPP/Source/Condition.h>

namespace AVT {
namespace VmbAPI {

Condition::Condition()
    :   m_nWaiterNumber( 0 )
    ,   m_nReleaseNumber( 0 )
    ,   m_bLocked( true )
{
    SP_SET( m_Semaphore, new Semaphore() );
}

void Condition::Wait( const BasicLockable &rLockable )
{
    Wait( rLockable.GetMutex() );
}

void Condition::Wait( const MutexPtr &rMutex )
{
    m_nWaiterNumber++;

    SP_ACCESS( rMutex )->Unlock();

    SP_ACCESS( m_Semaphore )->Acquire();

    SP_ACCESS( rMutex) ->Lock();

    if ( m_nWaiterNumber > 0 )
    {
        m_nWaiterNumber--;
    }

    if ( m_nReleaseNumber > 0 )
    {
        m_nReleaseNumber--;
    }

    if(     m_nWaiterNumber > 0
        &&  m_nReleaseNumber > 0 )
    {
        SP_ACCESS( m_Semaphore )->Release();
        m_bLocked = false;
    }
    else
    {
        m_bLocked = true;
    }
    
    if( m_nReleaseNumber > m_nWaiterNumber )
    {
        m_nReleaseNumber = m_nWaiterNumber;
    }
}

void Condition::Signal( bool bSingle )
{
    if( m_nWaiterNumber > m_nReleaseNumber )
    {
        if( true == bSingle )
        {
            m_nReleaseNumber++;
        }
        else
        {
            m_nReleaseNumber = m_nWaiterNumber;
        }

        if( true == m_bLocked )
        {
            SP_ACCESS( m_Semaphore )->Release();
            m_bLocked = false;
        }
    }
}

}} // namespace AVT::VmbAPI
