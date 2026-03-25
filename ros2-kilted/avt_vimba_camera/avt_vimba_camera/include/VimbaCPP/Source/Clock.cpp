/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Clock.cpp

  Description: Implementation of a platform independent Sleep.
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

#ifdef WIN32
    #include <sys/timeb.h>
    #include <windows.h>
#else
    #include <sys/time.h>
    #include <unistd.h>
#endif

#include <VimbaCPP/Source/Clock.h>

namespace AVT {
namespace VmbAPI {

Clock::Clock()
    :   m_dStartTime(0.0)
{
}

Clock::~Clock()
{
}

void Clock::Reset()
{
    m_dStartTime = 0.0;
}

void Clock::SetStartTime()
{
    m_dStartTime = GetAbsTime();
}

void Clock::SetStartTime( double dStartTime )
{
    m_dStartTime = dStartTime;
}

double Clock::GetTime() const
{
    double dTime = 0.0;

#ifdef WIN32
    _timeb currSysTime;

    _ftime_s(&currSysTime);

    dTime = (currSysTime.time + ((double)currSysTime.millitm) / 1000.0);
#else
    timeval now;

    if(gettimeofday(&now, (struct timezone *)0)) return 0.0;

    dTime = ((double)now.tv_sec) + ((double)(now.tv_usec) / 1000000.0);
#endif

    return dTime - m_dStartTime;
}

double Clock::GetAbsTime()
{
    double dAbsTime = 0.0;

#ifdef WIN32
    _timeb currSysTime;

    _ftime_s(&currSysTime);

    dAbsTime = (currSysTime.time + ((double)currSysTime.millitm) / 1000.0);
#else
    timeval now;

    if(gettimeofday(&now, (struct timezone *)0)) return 0.0;

    dAbsTime = ((double)now.tv_sec) + ((double)(now.tv_usec) / 1000000.0);
#endif

    return dAbsTime;
}

void Clock::Sleep(double dTime)
{
#ifdef WIN32
    ::Sleep((unsigned long)(dTime * 1000.0));
#else
    ::usleep((unsigned long)(dTime * 1000000.0));
#endif
}

void Clock::SleepMS(unsigned long nTimeMS)
{
#ifdef WIN32
    ::Sleep(nTimeMS);
#else
    ::usleep(nTimeMS * 1000);
#endif
}

void Clock::SleepAbs(double dAbsTime)
{
    Clock clock;
    double dTimeDiff = dAbsTime - clock.GetTime();
        
#ifdef WIN32
    if(dTimeDiff > 4000000.0) dTimeDiff = 4000000.0;
#else
    if(dTimeDiff >= 4000.0) dTimeDiff = 4000.0;
#endif
    while(dTimeDiff > 0.0)
    {
        Sleep(dTimeDiff);
        dTimeDiff = dAbsTime - clock.GetTime();
#ifdef WIN32
        if(dTimeDiff > 4000000.0) dTimeDiff = 4000000.0;
#else
        if(dTimeDiff >= 4000.0) dTimeDiff = 4000.0;
#endif
    }
}

}} //namespace AVT::VmbAPI
