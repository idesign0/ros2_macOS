/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        LoggerDefines.h

  Description: Definition of macros for logging.

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

#ifndef AVT_VMBAPI_LOGGERDEFINES_H
#define AVT_VMBAPI_LOGGERDEFINES_H

#include <VimbaCPP/Include/UserLoggerDefines.h>

#ifndef USER_LOGGER

    #include <VimbaCPP/Include/FileLogger.h>

    namespace AVT {
    namespace VmbAPI {

    #define LOGGER_DECL                         FileLogger
    #define LOGGER_DEF                          FileLogger( "VimbaCPP.log", true )
    #define LOGGER_LOG( logger, loggingInfo )   if ( NULL != (logger) ) (logger)->Log( loggingInfo );


    // These are all uses of LOGGER_DECL logger declarations
    typedef LOGGER_DECL* Logger;

    }}    

#else
    #include <VimbaCPP/Include/UserLoggerDefines.h>
#endif

#include <VimbaCPP/Include/VimbaSystem.h>

#define LOG_FREE_TEXT( txt )            std::string strExc( txt );\
                                        strExc.append( " in function: " );\
                                        strExc.append( __FUNCTION__ );\
                                        LOGGER_LOG( VimbaSystem::GetInstance().GetLogger(), strExc );

#endif
