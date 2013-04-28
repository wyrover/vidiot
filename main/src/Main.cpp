#include "Application.h"

#include "UtilLog.h"
#include "UtilLogWindows.h"
#include "Dialog.h"

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
    gui::Dialog::get().getDebugReport(); // Execution is aborted in getDebugReport(). May run in other thread.
}

void PureVirtualCallHandler(void)
{
    if (exceptionShown) return;
    exceptionShown = true;
    LOG_ERROR;
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