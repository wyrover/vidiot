// Copyright 2013 Eric Raijmakers.
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
#include "UtilLogWindows.h"
#include "UtilStackWalker.h"

#include <windows.h>

        /// Only show first exception. When the exception occurs in a separate thread,
        /// the delay in showing the dialog (via the main thread, not directly in the
        /// crashing thread, since wxWidets only allows new windows in the main thread)
        /// causes repeated exceptions.
bool exceptionShown = false;

LONG __stdcall ExceptionFilter( EXCEPTION_POINTERS* exception )
{
    if (exceptionShown) return EXCEPTION_CONTINUE_EXECUTION;
    exceptionShown = true;
    VAR_ERROR(*exception);
    LOG_STACKTRACE;
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
    gui::Dialog::get().getDebugReport(); // Execution is aborted in getDebugReport(). May run in other thread.
}

void PureVirtualCallHandler(void)
{
    if (exceptionShown) return;
    exceptionShown = true;
    LOG_ERROR;
    LOG_STACKTRACE;
    gui::Dialog::get().getDebugReport(); // Execution is aborted in getDebugReport(). May run in other thread.
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

    gui::Application* main;

    main = new gui::Application();
    wxApp::SetInstance(main);
    wxEntryStart(hInstance,hPrevInstance,lpCmdLine,nCmdShow);
    if (wxTheApp->OnInit())
    {
        wxTheApp->OnRun();
        wxTheApp->OnExit();
    }
    wxEntryCleanup();
    return 0;
}



// todo http://randomascii.wordpress.com/2012/07/05/when-even-crashing-doesnt-work/


    //void EnableCrashingOnCrashes()
    //{
    //    typedef BOOL (WINAPI *tGetPolicy)(LPDWORD lpFlags);
    //    typedef BOOL (WINAPI *tSetPolicy)(DWORD dwFlags);
    //    const DWORD EXCEPTION_SWALLOWING = 0×1;

    //    HMODULE kernel32 = LoadLibraryA(“kernel32.dll”);
    //    tGetPolicy pGetPolicy = (tGetPolicy)GetProcAddress(kernel32,
    //                “GetProcessUserModeExceptionPolicy”);
    //    tSetPolicy pSetPolicy = (tSetPolicy)GetProcAddress(kernel32,
    //                “SetProcessUserModeExceptionPolicy”);
    //    if (pGetPolicy && pSetPolicy)
    //    {
    //        DWORD dwFlags;
    //        if (pGetPolicy(&dwFlags))
    //        {
    //            // Turn off the filter
    //            pSetPolicy(dwFlags & ~EXCEPTION_SWALLOWING);
    //        }
    //    }
    //}
