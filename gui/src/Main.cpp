#include "Application.h"

//    bool wxEntryStart(int& argc, wxChar **argv)
//        instead of wxEntry. It doesn't call your app's OnInit() or run the main loop.
//        You can call wxTheApp->CallOnInit() to invoke OnInit() when needed in your tests.
//        You'll need to use
//        void wxEntryCleanup()
//when you're done.
/// todo unit tests: bool wxEntryStart 	( 	int &  	argc, wxChar **  	argv	  )
/// and void wxEntryCleanup 	( 		 )  	



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
