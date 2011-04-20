#include "Application.h"

extern "C" int WINAPI WinMain(HINSTANCE hInstance,                      
                              HINSTANCE hPrevInstance,                  
                              wxCmdLineArgType lpCmdLine,     
                              int nCmdShow)                             
{                           
    wxDISABLE_DEBUG_SUPPORT();    
    
    gui::Application* main = new gui::Application();
    wxApp::SetInstance(main);
    wxEntryStart(hInstance,hPrevInstance,lpCmdLine,nCmdShow);
    wxTheApp->OnInit();
    // you can create top level-windows here or in OnInit(). Do your testing here
    wxTheApp->OnRun();
    wxTheApp->OnExit();
    wxEntryCleanup();
    return 0;
}                                                                       
