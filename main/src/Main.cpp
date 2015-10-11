// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "Application.h"

#include "Dialog.h"
#include "UtilLog.h"
#include "UtilAssert.h"
#include "UtilStackWalker.h"

/// Only show first exception. When the exception occurs in a separate thread,
/// the delay in showing the dialog (via the main thread, not directly in the
/// crashing thread, since wxWidets only allows new windows in the main thread)
/// causes repeated exceptions.
bool exceptionShown = false;

#ifdef _MSC_VER
#include "UtilLogWindows.h"

#include <windows.h>

LONG __stdcall ExceptionFilter( EXCEPTION_POINTERS* exception )
{
    if (exceptionShown) return EXCEPTION_CONTINUE_EXECUTION;
    exceptionShown = true;
    VAR_ERROR(*exception);
    LOG_STACKTRACE;
    breakIntoDebugger();
    gui::Dialog::get().getDebugReport(); // Execution is aborted in getDebugReport(). May run in other thread.
    return EXCEPTION_CONTINUE_EXECUTION;
}

void InvalidParameterHandler(const wchar_t* expression_, const wchar_t* function_,  const wchar_t* file_,  unsigned int line,  uintptr_t reserved)
{
    if (exceptionShown) return;
    exceptionShown = true;
    wxString expression(expression_);
    wxString function(function_);
    wxString file(file_);
    VAR_ERROR(file)(line)(function)(expression)(reserved); // If all strings are empty in the logging: that is due to the absence of debug info in the build (release build without debug info?)
    LOG_STACKTRACE;
    breakIntoDebugger();
    gui::Dialog::get().getDebugReport(); // Execution is aborted in getDebugReport(). May run in other thread.
}

void PureVirtualCallHandler(void)
{
    if (exceptionShown) return;
    exceptionShown = true;
    LOG_ERROR;
    LOG_STACKTRACE;
    breakIntoDebugger();
    gui::Dialog::get().getDebugReport(); // Execution is aborted in getDebugReport(). May run in other thread.
}

// Thanks to:
// http://randomascii.wordpress.com/2012/07/05/when-even-crashing-doesnt-work/
void EnableCrashingOnCrashes()
{
    typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags);
    typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags);
    const DWORD EXCEPTION_SWALLOWING = 0x01;

    HMODULE kernel32 = ::LoadLibraryA("kernel32.dll");
    tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32, "GetProcessUserModeExceptionPolicy");
    tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32, "SetProcessUserModeExceptionPolicy");
    if (pGetPolicy && pSetPolicy)
    {
        DWORD dwFlags;
        if (pGetPolicy(&dwFlags))
        {
            // Turn off the filter
            pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
        }
    }
}

extern "C" int WINAPI WinMain(HINSTANCE hInstance,
                              HINSTANCE hPrevInstance,
                              wxCmdLineArgType lpCmdLine,
                              int nCmdShow)
{
    //wxDISABLE_DEBUG_SUPPORT();
    //wxDISABLE_ASSERTS_IN_RELEASE_BUILD();
    wxDISABLE_DEBUG_LOGGING_IN_RELEASE_BUILD();

 	SetUnhandledExceptionFilter(ExceptionFilter);
    _CrtSetReportMode(_CRT_ASSERT, 0); // Disable CRT message box
    _set_invalid_parameter_handler(InvalidParameterHandler);
    _set_purecall_handler(PureVirtualCallHandler);
    EnableCrashingOnCrashes();

    wxApp::SetInstance(new gui::Application());
    wxEntryStart(hInstance,hPrevInstance,lpCmdLine,nCmdShow);
    if (wxTheApp->OnInit())
    {
        wxTheApp->OnRun();
        wxTheApp->OnExit();
    }
    wxEntryCleanup();
    return 0;
}

#else

#include <signal.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include "UtilLogX.h"
#include "UtilLogLinux.h"

static void signalHandler(int sig, siginfo_t *signal, void *context)
{
    if (exceptionShown) return;
    exceptionShown = true;
    VAR_ERROR(signal);
    LOG_STACKTRACE;
    for ( auto sig : { SIGQUIT, SIGINT, SIGTERM } )
    {
        if (sig == signal->si_signo) { exit(0); }
    }
    breakIntoDebugger();
    gui::Dialog::get().getDebugReport(); // Execution is aborted in getDebugReport(). May run in other thread.

}


#include <X11/Xproto.h>
static XErrorHandler previousXErrorHandler = static_cast<XErrorHandler>(0);

/// Log the first x error, then pass on error handling to the
/// previous (wxWidgets) error handler.
static int onXError(Display *display, XErrorEvent *error)
{
    if (exceptionShown) return previousXErrorHandler(display,error);
    exceptionShown = true;
    VAR_ERROR(error);
    LOG_STACKTRACE;
    breakIntoDebugger();
    return previousXErrorHandler(display,error);
}

int main(int argc, char *argv[])
{
    wxDISABLE_DEBUG_LOGGING_IN_RELEASE_BUILD();

    std::vector<int> signals;
#ifdef NDEBUG
    signals = { SIGQUIT, SIGSEGV, SIGHUP, SIGINT, SIGILL, SIGTRAP, SIGABRT, SIGBUS, SIGFPE, SIGPIPE, SIGTERM, SIGALRM, SIGSTKFLT, SIGTSTP, SIGSYS };
#endif

    for (int signal : signals)
    {
        struct sigaction act;
        memset (&act, '\0', sizeof(act));
        act.sa_sigaction = &signalHandler; // sa_sigaction field used because of two additional parameters
        act.sa_flags = SA_SIGINFO; // use sa_sigaction field, not sa_handler
        sigaction(signal, &act, 0);
    }

    wxApp::SetInstance(new gui::Application());
    wxEntryStart(argc,argv);
    if (wxTheApp->OnInit())
    {
        previousXErrorHandler = XSetErrorHandler(onXError);
//        Display *tmpDisplay = XOpenDisplay(0);
//        XSynchronize(tmpDisplay, True);
        wxTheApp->OnRun();
        wxTheApp->OnExit();
    }
    wxEntryCleanup();
    return 0;
}

#endif // _MSC_VER