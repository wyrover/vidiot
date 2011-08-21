#include "Application.h"

extern "C" int WINAPI WinMain(HINSTANCE hInstance,                      
                              HINSTANCE hPrevInstance,                  
                              wxCmdLineArgType lpCmdLine,     
                              int nCmdShow)                             
{                           
    //wxDISABLE_DEBUG_SUPPORT();
    //wxDISABLE_ASSERTS_IN_RELEASE_BUILD();
    wxDISABLE_DEBUG_LOGGING_IN_RELEASE_BUILD();
    gui::Application* main;
    try
    {
        main = new gui::Application();
        wxApp::SetInstance(main);
        wxEntryStart(hInstance,hPrevInstance,lpCmdLine,nCmdShow);
        wxTheApp->OnInit();
        wxTheApp->OnRun();
        wxTheApp->OnExit();
        wxEntryCleanup();
    }
    catch (...)
    {
        main->OnUnhandledException();
    }
    return 0;
}                                                                       
