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

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> // fork, execvp, chdir
#include <time.h> // nanosleep

#include "console/console.h"
#include "io/fileStream.h"
#include "game/resource.h"
#include "game/version.h"
#include "math/mRandom.h"
#include "platformX86UNIX/platformX86UNIX.h"
#include "platformX86UNIX/x86UNIXStdConsole.h"
#include "platform/event.h"
#include "game/gameInterface.h"
#include "platform/platform.h"
#include "platform/platformAL.h"
#include "platform/platformInput.h"
#include "platform/platformVideo.h"
#include "debug/profiler.h"
#include "platformX86UNIX/platformGL.h"
#include "platformX86UNIX/x86UNIXOGLVideo.h"
#include "platformX86UNIX/x86UNIXState.h"

#ifndef DEDICATED
#include "platformX86UNIX/x86UNIXInputManager.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_version.h>
#endif

x86UNIXPlatformState *x86UNIXState;

extern bool InitOpenGL();

//------------------------------------------------------------------------------
static S32 ParseCommandLine(S32 argc, const char **argv, 
   Vector<char*>& newCommandLine)
{
   x86UNIXState->setExePathName(argv[0]);
   bool foundDedicated = false;

   for ( int i=0; i < argc; i++ )
   {
      // look for platform specific args
      if (dStrcmp(argv[i], "-version") == 0)
      {
         dPrintf("%s (built on %s)\n", getVersionString(), getCompileTimeString());
         dPrintf("gcc: %s\n", __VERSION__);
         return 1;
      }
      if (dStrcmp(argv[i], "-dedicated") == 0)
      {
         foundDedicated = true;
         // no continue because dedicated is also handled by script
      }
      if (dStrcmp(argv[i], "-nohomedir") == 0)
      {
         x86UNIXState->setUseRedirect(false);
         continue;
      }
      if (dStrcmp(argv[i], "-chdir") == 0)
      {
         if ( ++i >= argc )
         {
            dPrintf("Follow -chdir option with the desired working directory.\n");
            return 1;
         }
         if (chdir(argv[i]) == -1)
         {
            dPrintf("Unable to chdir to %s: %s\n", argv[i], strerror(errno));
            return 1;
         }
         continue;
      }
      
      // copy the arg into newCommandLine
      int argLen = dStrlen(argv[i]) + 1;
      char* argBuf = new char[argLen]; // this memory is deleted in main()
      dStrncpy(argBuf, argv[i], argLen);
      newCommandLine.push_back(argBuf);
   }
   x86UNIXState->setDedicated(foundDedicated);
#if defined(DEDICATED) && !defined(TORQUE_ENGINE)
   if (!foundDedicated)
   {
      dPrintf("This is a dedicated server build.  You must supply the -dedicated command line parameter.\n");
      return 1;
   }
#endif
   return 0;
}

//------------------------------------------------------------------------------
static void InitWindow(const Point2I &initialSize, const char *name)
{
   x86UNIXState->setWindowSize(initialSize);
   x86UNIXState->setWindowName(name);
}

#ifndef DEDICATED
//------------------------------------------------------------------------------
static bool InitSDL()
{
   if (SDL_Init(SDL_INIT_VIDEO) != 0)
      return false;

   atexit(SDL_Quit);


   // SDL_SysWMinfo sysinfo;
   // SDL_VERSION(&sysinfo.version);
   // if (SDL_GetWMInfo(&sysinfo) == 0)
   //    return false;

// TODO: Redo how the is acquired, use SDL_GetDesktopDisplayMode
   // x86UNIXState->setDesktopSize( 
   //    (S32) DisplayWidth( 
   //       display,
   //       x86UNIXState->getScreenNumber()),
   //    (S32) DisplayHeight( 
   //       display,
   //       x86UNIXState->getScreenNumber())
   //    );
   // x86UNIXState->setDesktopBpp( 
   //    (S32) DefaultDepth( 
   //       display,
   //       x86UNIXState->getScreenNumber()));

   // indicate that we want sys WM messages
   SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

   return true;
}

//------------------------------------------------------------------------------
static void ProcessSYSWMEvent(const SDL_Event& event)
{
   // Does nothing?
}

//------------------------------------------------------------------------------
static void SetAppState()
{
   // U8 state = SDL_GetAppState();

   // // if we're not active but we have appactive and inputfocus, set window
   // // active and reactivate input
   // if ((!x86UNIXState->windowActive() || !Input::isActive()) &&
   //    state & SDL_APPACTIVE &&
   //    state & SDL_APPINPUTFOCUS)
   // {
   //    x86UNIXState->setWindowActive(true);
   //    Input::reactivate();
   // }
   // // if we are active, but we don't have appactive or input focus,
   // // deactivate input (if window not locked) and clear windowActive
   // else if (x86UNIXState->windowActive() && 
   //    !(state & SDL_APPACTIVE && state & SDL_APPINPUTFOCUS))
   // {
   //    if (x86UNIXState->windowLocked())
   //       Input::deactivate();
   //    x86UNIXState->setWindowActive(false);
   // }
}

//------------------------------------------------------------------------------
static S32 NumEventsPending()
{
   static const int MaxEvents = 255;
   static SDL_Event events[MaxEvents];

   SDL_PumpEvents();
   return SDL_PeepEvents(events, MaxEvents, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT);
}

//------------------------------------------------------------------------------
static bool ProcessMessages()
{
   static const int MaxEvents = 255;
   static SDL_Event events[MaxEvents];
 
   SDL_PumpEvents();
   S32 numEvents = SDL_PeepEvents(events, MaxEvents, SDL_GETEVENT, SDL_QUIT, SDL_SYSWMEVENT);
   if (numEvents == 0)
      return true;
   for (int i = 0; i < numEvents; ++i)
   {
      SDL_Event& event = events[i];
      switch (event.type)
      {
         case SDL_QUIT:
            Con::printf("SDL_QUIT");
            return false;
            break;
         case SDL_WINDOWEVENT:

            switch (event.window.event) {
               case SDL_WINDOWEVENT_RESIZED:
               case SDL_WINDOWEVENT_EXPOSED:
                  Con::printf("window resized/exposed");
                  Game->refreshWindow();
                  break;
               case SDL_WINDOWEVENT_ENTER:
                  Con::printf("mouse enter");
                  break;
               case SDL_WINDOWEVENT_LEAVE:
                  Con::printf("mouse leave");
                  break;
               case SDL_WINDOWEVENT_FOCUS_GAINED:
                  Con::printf("focus gained");
                  x86UNIXState->setWindowActive(true);
                  Input::reactivate();
                  break;
               case SDL_WINDOWEVENT_FOCUS_LOST:
                  Con::printf("focus lost");
                  Input::deactivate();
                  x86UNIXState->setWindowActive(false);
                  break;
               case SDL_WINDOWEVENT_CLOSE:
                  Con::printf("window close");
                  return false;
                  break;
            }
            break;
         case SDL_SYSWMEVENT:
            ProcessSYSWMEvent(event);
            break;
      }
   }
   return true;
}

//------------------------------------------------------------------------------
// send a destroy window event to the window.  assumes
// window is created.
void SendQuitEvent()
{
   SDL_Event quitevent;
   quitevent.type = SDL_QUIT;
   SDL_PushEvent(&quitevent);
}
#endif // DEDICATED

//------------------------------------------------------------------------------
// TODO: remove this completely
static inline void Sleep(int secs, int nanoSecs)
{
   timespec sleeptime;
   sleeptime.tv_sec = secs;
   sleeptime.tv_nsec = nanoSecs;
   nanosleep(&sleeptime, NULL);
}

#ifndef DEDICATED
struct AlertWinState
{
      bool fullScreen;
      bool cursorHidden;
      bool inputGrabbed;
};

//------------------------------------------------------------------------------
void DisplayErrorAlert(const char* errMsg, bool showSDLError)
{
   char fullErrMsg[2048];
   dStrncpy(fullErrMsg, errMsg, sizeof(fullErrMsg));
   
   if (showSDLError)
   {
      const char* sdlerror = SDL_GetError();
      if (sdlerror != NULL && dStrlen(sdlerror) > 0)
      {
         dStrcat(fullErrMsg, "  (Error: ");
         dStrcat(fullErrMsg, sdlerror);
         dStrcat(fullErrMsg, ")");
      }
   }
   
   Platform::AlertOK("Error", fullErrMsg);
}


//------------------------------------------------------------------------------
static inline void AlertDisableVideo(AlertWinState& state)
{

   state.fullScreen = Video::isFullScreen();
   state.cursorHidden = (SDL_ShowCursor(SDL_QUERY) == SDL_DISABLE);
//   state.inputGrabbed = (SDL_WM_GrabInput(SDL_GRAB_QUERY) == SDL_GRAB_ON);

   // if (state.fullScreen)
   //    SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
   // if (state.cursorHidden)
   //    SDL_ShowCursor(SDL_ENABLE);
   // if (state.inputGrabbed)
   //    SDL_WM_GrabInput(SDL_GRAB_OFF);
}

//------------------------------------------------------------------------------
static inline void AlertEnableVideo(AlertWinState& state)
{
   // if (state.fullScreen)
   //    SDL_WM_ToggleFullScreen(SDL_GetVideoSurface());
   // if (state.cursorHidden)
   //    SDL_ShowCursor(SDL_DISABLE);
   // if (state.inputGrabbed)
   //    SDL_WM_GrabInput(SDL_GRAB_ON);
}
#endif // DEDICATED

//------------------------------------------------------------------------------
void Platform::AlertOK(const char *windowTitle, const char *message)
{
#ifndef DEDICATED
   SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, windowTitle, message, NULL);
#endif
   {
      if (Con::isActive() && StdConsole::isEnabled())
         Con::printf("Alert: %s %s", windowTitle, message);
      else
         dPrintf("Alert: %s %s\n", windowTitle, message);
   }
}

//------------------------------------------------------------------------------
bool Platform::AlertOKCancel(const char *windowTitle, const char *message)
{
#ifndef DEDICATED
#endif
   {
      if (Con::isActive() && StdConsole::isEnabled())
         Con::printf("Alert: %s %s", windowTitle, message);
      else
         dPrintf("Alert: %s %s\n", windowTitle, message);
      return false;
   }
}

//------------------------------------------------------------------------------
bool Platform::AlertRetry(const char *windowTitle, const char *message)
{
#ifndef DEDICATED
#endif
   {
      if (Con::isActive() && StdConsole::isEnabled())
         Con::printf("Alert: %s %s", windowTitle, message);
      else
         dPrintf("Alert: %s %s\n", windowTitle, message);
      return false;
   }
}

//Luma: YesNo alert message
bool Platform::AlertYesNo(const char *windowTitle, const char *message)
{
#ifndef DEDICATED
#endif
   {
      if (Con::isActive() && StdConsole::isEnabled())
         Con::printf("Alert: %s %s", windowTitle, message);
      else
         dPrintf("Alert: %s %s\n", windowTitle, message);
      return false;
   }
}

// Very hacky, doesn't support buttons or icons
S32 Platform::messageBox(const UTF8 *title, const UTF8 *message, MBButtons buttons, MBIcons icon)
{
   Platform::AlertOK(title, message);
   return 0;
}

//------------------------------------------------------------------------------
void Platform::enableKeyboardTranslation(void)
{
#ifndef DEDICATED
   // JMQ: not sure if this is needed for i18n keyboards
   //SDL_EnableUNICODE( 1 );
//    SDL_EnableKeyRepeat(
//       SDL_DEFAULT_REPEAT_DELAY, 
//       SDL_DEFAULT_REPEAT_INTERVAL);
#endif
}

//------------------------------------------------------------------------------
void Platform::disableKeyboardTranslation(void)
{
#ifndef DEDICATED
   //SDL_EnableUNICODE( 0 );
   //   SDL_EnableKeyRepeat(0, 0);
#endif
}

//------------------------------------------------------------------------------
void Platform::minimizeWindow()
{
#ifndef DEDICATED
   // if (x86UNIXState->windowCreated())
   //    SDL_WM_IconifyWindow();
#endif
}

void Platform::restoreWindow()
{
}

//------------------------------------------------------------------------------
void Platform::process()
{
   PROFILE_START(XUX_PlatformProcess);
   stdConsole->process();

   if (x86UNIXState->windowCreated())
   {
#ifndef DEDICATED
      // process window events
      PROFILE_START(XUX_ProcessMessages);
      bool quit = !ProcessMessages();
      PROFILE_END();
      if(quit)
      {
         // generate a quit event
         Event quitEvent;
         quitEvent.type = QuitEventType;
         Game->postEvent(quitEvent);
      }

      // process input events
      PROFILE_START(XUX_InputProcess);
      Input::process();
      PROFILE_END();
#endif
   }

   PROFILE_END();
}

//------------------------------------------------------------------------------
const Point2I &Platform::getWindowSize()
{
   return x86UNIXState->getWindowSize();
}


//------------------------------------------------------------------------------
void Platform::setWindowSize( U32 newWidth, U32 newHeight )
{
   x86UNIXState->setWindowSize( (S32) newWidth, (S32) newHeight );
}

//------------------------------------------------------------------------------
void Platform::shutdown()
{
   Cleanup();
}

//------------------------------------------------------------------------------
void Platform::init()
{
   // Set the platform variable for the scripts
   Con::setVariable( "$platform", "x86UNIX" );
#if defined(__linux__)
   Con::setVariable( "$platformUnixType", "Linux" );
#elif defined(__OpenBSD__)
   Con::setVariable( "$platformUnixType", "OpenBSD" );
#else
   Con::setVariable( "$platformUnixType", "Unknown" );
#endif

   StdConsole::create();
   
#ifndef DEDICATED
   // if we're not dedicated do more initialization
   if (!x86UNIXState->isDedicated())
   {
      // init SDL
      if (!InitSDL())
      {
         DisplayErrorAlert("Unable to initialize SDL.");
         ImmediateShutdown(1);
      } else {
         Con::printf("SDL Initialized");
      }

      // initialize input
      Input::init();

      Con::printf( "Video Init:" );

      // load gl library
      if (!GLLoader::OpenGLInit())
      {
         DisplayErrorAlert("Unable to initialize OpenGL.");
         ImmediateShutdown(1);
      }

      // initialize video
      Video::init();
      if ( Video::installDevice( OpenGLDevice::create() ) )
         Con::printf( "   OpenGL display device detected." );
      else
         Con::printf( "   OpenGL display device not detected." );
      
      Con::printf(" ");
   }
#endif
   // if we are dedicated, do sleep timing and display results
   if (x86UNIXState->isDedicated())
   {
      const S32 MaxSleepIter = 10;
      U32 totalSleepTime = 0;
      U32 start;
      for (S32 i = 0; i < MaxSleepIter; ++i)
      {
         start = Platform::getRealMilliseconds();
         Sleep(0, 1000000);
         totalSleepTime += Platform::getRealMilliseconds() - start;
      }
      U32 average = static_cast<U32>(totalSleepTime / MaxSleepIter);

      Con::printf("Sleep latency: %ums", average);
      // dPrintf as well, since console output won't be visible yet
      dPrintf("Sleep latency: %ums\n", average);
      if (!x86UNIXState->getDSleep() && average < 10)
      {
         const char* msg = "Sleep latency ok, enabling dsleep for lower cpu " \
            "utilization";
         Con::printf("%s", msg);
         dPrintf("%s\n", msg);
         x86UNIXState->setDSleep(true);
      }
   }
}

//------------------------------------------------------------------------------
void Platform::initWindow(const Point2I &initialSize, const char *name)
{
#ifndef DEDICATED
   // initialize window
   InitWindow(initialSize, name);
   if (!InitOpenGL())
      ImmediateShutdown(1);
#endif
}

//------------------------------------------------------------------------------
// Web browser function:
//------------------------------------------------------------------------------
bool Platform::openWebBrowser( const char* webAddress )
{
   if (!webAddress || dStrlen(webAddress)==0)
      return false;

   // look for a browser preference variable
   // JMQTODO: be nice to implement some UI to customize this
   const char* webBrowser = Con::getVariable("Pref::Unix::WebBrowser");
   if (dStrlen(webBrowser) == 0)
      webBrowser = NULL;

   pid_t pid = fork();
   if (pid == -1)
   {
      Con::printf("WARNING: Platform::openWebBrowser failed to fork");
      return false;
   }
   else if (pid != 0)
   {
      // parent
      if (Video::isFullScreen())
         Video::toggleFullScreen();

      return true;
   }
   else if (pid == 0)
   {
      // child
      char* argv[3];
      argv[0] = 0;
      argv[1] = const_cast<char*>(webAddress);
      argv[2] = 0;

      int ok = -1;

      // if execvp returns, it means it couldn't execute the program
      if (webBrowser != NULL)
         ok = execvp(webBrowser, argv);

      ok = execvp("xdg-open", argv);
      ok = execvp("firefox", argv);
      ok = execvp("konqueror", argv);
      ok = execvp("mozilla", argv);
      // use dPrintf instead of Con here since we're now in another process, 
      dPrintf("WARNING: Platform::openWebBrowser: couldn't launch a web browser\n");
      _exit(-1);     
      return false;
   }
   else
   {
      Con::printf("WARNING: Platform::openWebBrowser: forking problem");
      return false;
   }
}

void Platform::setMouseLock(bool locked)
{
  // Not implemented
}

//------------------------------------------------------------------------------
int main(S32 argc, const char **argv)
{
   // init platform state
   x86UNIXState = new x86UNIXPlatformState;

   // parse the command line for unix-specific params
   Vector<char *> newCommandLine;
   S32 returnVal = ParseCommandLine(argc, argv, newCommandLine);
   if (returnVal != 0)
      return returnVal;

   // init lastTimeTick for TimeManager::process()
   x86UNIXState->lastTimeTick = Platform::getRealMilliseconds();

   // init process control stuff 
   ProcessControlInit();

   Game->mainInitialize(argc, argv);
  
   // run the game
   while ( Game->isRunning() )
   {
      Game->mainLoop();
   }

   Game->mainShutdown();

   // dispose of command line
   for(U32 i = 0; i < newCommandLine.size(); i++)
      delete [] newCommandLine[i];

   // dispose of state
   delete x86UNIXState;

   return returnVal;
}

void Platform::setWindowTitle( const char* title )
{
#ifndef DEDICATED
   x86UNIXState->setWindowName(title);
#endif
}

Resolution Video::getDesktopResolution()
{
   Resolution  result;
   result.h   = x86UNIXState->getDesktopSize().x;
   result.w   = x86UNIXState->getDesktopSize().y;
   result.bpp = x86UNIXState->getDesktopBpp();

  return result;
}


