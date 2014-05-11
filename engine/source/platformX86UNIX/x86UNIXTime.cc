//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------



#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/x86UNIXState.h"
#include "platform/event.h"
#include "game/gameInterface.h"

#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

extern x86UNIXPlatformState *x86UNIXState;

U32 x86UNIXGetTickCount();
//--------------------------------------
void Platform::getLocalTime(LocalTime &lt)
{
   struct tm *systime;
   time_t long_time;

   time( &long_time );                // Get time as long integer.
   systime = localtime( &long_time ); // Convert to local time.
   
   lt.sec      = systime->tm_sec;      
   lt.min      = systime->tm_min;      
   lt.hour     = systime->tm_hour;     
   lt.month    = systime->tm_mon;    
   lt.monthday = systime->tm_mday; 
   lt.weekday  = systime->tm_wday;  
   lt.year     = systime->tm_year;     
   lt.yearday  = systime->tm_yday;  
   lt.isdst    = systime->tm_isdst;    
}

U32 Platform::getTime()
{
   time_t long_time;
   time( &long_time );
   return long_time;
}   

U32 Platform::getRealMilliseconds()
{
//   struct rusage usageStats;
//   getrusage(RUSAGE_SELF, &usageStats);
//   return usageStats.ru_utime.tv_usec;
   return x86UNIXGetTickCount();
}

U32 Platform::getVirtualMilliseconds()
{
   return x86UNIXState->currentTime;
}   

void Platform::advanceTime(U32 delta)
{
   x86UNIXState->currentTime += delta;
}

//-------------------------------------------------------------------------------
void TimeManager::process()
{
   U32 curTime = Platform::getRealMilliseconds();
   TimeEvent event;
   event.elapsedTime = curTime - x86UNIXState->lastTimeTick;

// TODO: Fix background sleeping stuff
   // if we're not the foreground window, sleep for 1 ms
   // if (!x86UNIXState->windowActive())
      // Sleep(0, getBackgroundSleepTime() * 1000000);

   if(event.elapsedTime > sgTimeManagerProcessInterval)
   {
      x86UNIXState->lastTimeTick = curTime;
      Game->postEvent(event);
   }
}

// Move this into the time manager stuff
// else
// {
//    // if we're not in journal mode, sleep for 1 ms
//    // JMQ: since linux's minimum sleep latency seems to be 20ms, this can
//    // increase player pings by 10-20ms in the dedicated server. the server sleeps anyway when
//    // there are no players connected.
//    // JMQ: recent kernels (such as RH 8.0 2.4.18) reduce the latency
//    // to 2-4 ms on average.
//    if (!Game->isJournalReading() && (x86UNIXState->getDSleep() || 
//           Con::getIntVariable("Server::PlayerCount") - 
//           Con::getIntVariable("Server::BotCount") <= 0))
//    {
//       PROFILE_START(XUX_Sleep);
//       Sleep(0, getBackgroundSleepTime() * 100000);
//       PROFILE_END();
//    }
// }

//------------------------------------------------------------------------------
//-------------------------------------- x86UNIX Implementation
//
//
static bool   sg_initialized = false;
static U32 sg_secsOffset  = 0;
//--------------------------------------
U32 x86UNIXGetTickCount()
{
   // TODO: What happens when crossing a day boundary?
   //
   timeval t;

   if (sg_initialized == false) {
      sg_initialized = true;

      gettimeofday(&t, NULL);
      sg_secsOffset = t.tv_sec;
   }

   gettimeofday(&t, NULL);

   U32 secs  = t.tv_sec - sg_secsOffset;
   U32 uSecs = t.tv_usec;

   // Make granularity 1 ms
   return (secs * 1000) + (uSecs / 1000);
}


void Platform::sleep(U32 ms)
{
	// note: this will overflow if you want to sleep for more than 49 days. just so ye know.
	usleep( ms * 1000 );
}
	    

