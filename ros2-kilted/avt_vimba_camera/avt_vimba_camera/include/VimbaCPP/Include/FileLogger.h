/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        FileLogger.h

  Description: Definition of class AVT::VmbAPI::FileLogger.

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

#ifndef AVT_VMBAPI_FILELOGGER_H
#define AVT_VMBAPI_FILELOGGER_H

#include <string>
#include <stdio.h>
#include <fstream>

#include <VimbaCPP/Include/SharedPointerDefines.h>
#include <VimbaCPP/Include/Mutex.h>

namespace AVT {
namespace VmbAPI {

class FileLogger
{
  public:
    FileLogger( const char *pFileName, bool append = true );
    virtual ~FileLogger();

    void Log( const std::string &StrMessage );

private:
    std::ofstream   m_File;
    MutexPtr        m_pMutex;

    std::string GetTempPath();
    FileLogger( const FileLogger& );
    FileLogger& operator=( const FileLogger& );
};

}} //namespace AVT:VmbAPI

#endif
