/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        BasicLockable.cpp

  Description: Implementation of class AVT::VmbAPI::BasicLockable.

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

#include <VimbaCPP/Include/BasicLockable.h>
#include <VimbaCPP/Include/LoggerDefines.h>

namespace AVT {
namespace VmbAPI {

BasicLockable::BasicLockable()
    :   m_pMutex( MutexPtr( new Mutex() ))
{
}

BasicLockable::~BasicLockable()
{
}

BasicLockable::BasicLockable( MutexPtr pMutex )
    :   m_pMutex( pMutex )
{
}

MutexPtr& BasicLockable::GetMutex()
{
    return m_pMutex;
}
const MutexPtr& BasicLockable::GetMutex() const
{
    return m_pMutex;
}

}} //namespace AVT::VmbAPI
