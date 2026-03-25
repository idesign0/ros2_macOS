/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ConditionHelper.cpp

  Description: Implementation of helper class for conditions.
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

#include <VimbaCPP/Source/ConditionHelper.h>

#include <VimbaCPP/Source/MutexGuard.h>

namespace AVT {
namespace VmbAPI {

ConditionHelper::ConditionHelper()
    :   m_nNumListReads( 0 )
    ,   m_bIsWritingList( false )
    ,   m_bExclusive( false )
{    
}

bool ConditionHelper::EnterReadLock( BasicLockable &rLockable )
{
    return EnterReadLock( rLockable.GetMutex() );
}
bool ConditionHelper::EnterReadLock( MutexPtr &pMutex )
{
    MutexGuard guard( pMutex );
    if ( true == m_bExclusive )
    {
        guard.Release();
        return false;
    }
    while ( true == m_bIsWritingList )
    {
        m_WriteCondition.Wait( pMutex );
    }
    ++m_nNumListReads;
    guard.Release();

    return true;
}

void ConditionHelper::ExitReadLock( BasicLockable &rLockable )
{
    ExitReadLock( rLockable.GetMutex() );
}
void ConditionHelper::ExitReadLock( MutexPtr &pMutex )
{
    MutexGuard guard( pMutex );    
    if ( 0 == --m_nNumListReads )
    {
        m_ReadCondition.Signal();
    }
    guard.Release();
}

bool ConditionHelper::EnterWriteLock( BasicLockable &rLockable, bool bExclusive )
{
    return EnterWriteLock( rLockable.GetMutex(), bExclusive );
}
bool ConditionHelper::EnterWriteLock( MutexPtr &pMutex, bool bExclusive )
{
    MutexGuard guard( pMutex );
    if ( true == m_bExclusive )
    {
        guard.Release();
        return false;
    }
    while ( true == m_bIsWritingList )
    {
        m_WriteCondition.Wait( pMutex );
    }
    m_bIsWritingList = true;
    m_bExclusive = bExclusive;
    while ( 0 < m_nNumListReads )
    {
        m_ReadCondition.Wait( pMutex );
    }
    guard.Release();

    return true;
}

void ConditionHelper::ExitWriteLock( BasicLockable &rLockable )
{
    ExitWriteLock( rLockable.GetMutex() );
}
void ConditionHelper::ExitWriteLock( MutexPtr &pMutex )
{
    MutexGuard guard( pMutex );
    m_bIsWritingList = false;
    m_bExclusive = false;
    m_WriteCondition.Signal();
    guard.Release();
}

}} // namespace AV::VimbaAPI
