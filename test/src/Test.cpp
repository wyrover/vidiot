#include "Test.h"
#include "UtilList.h"
#include <list>
#include <wx/uiaction.h>
#include <boost/assign/list_of.hpp>
#include "UtilLogStl.h"
#include "UtilLog.h"
#include "VideoFile.h"
#include "Application.h"
#include "Window.h"
#include "ids.h"

//    bool wxEntryStart(int& argc, wxChar **argv)
//        instead of wxEntry. It doesn't call your app's OnInit() or run the main loop.
//        You can call wxTheApp->CallOnInit() to invoke OnInit() when needed in your tests.
//        You'll need to use
//        void wxEntryCleanup()





    void MyTestSuite::testAddition()
    {
        TS_ASSERT( 1 + 1 > 1 );
        TS_ASSERT_EQUALS( 1 + 1, 2 );

        model::VideoFile f;

        std::list<int> l;
        UtilList<int>(l).addElements(boost::assign::list_of(1)(2)(3),-1);
        TS_ASSERT(l.size() == 3);
        UtilList<int>(l).removeElements(boost::assign::list_of(1)(2)(3));
        TS_ASSERT(l.size() == 0);



    }


    void MyTestSuite::testStartup()
    {
        gui::Application* main = new gui::Application();
        wxApp::SetInstance(main);
        int argc = 1;
        char* arg = "arg";
        wxEntryStart(argc, &arg);
        wxTheApp->OnInit();

        mThread.reset(new boost::thread(boost::bind(&MyTestSuite::thread,this)));
        wxTheApp->OnRun();

        wxTheApp->OnExit();
        wxEntryCleanup();

        if (mThread)
        {
            mThread->join();
        }
    }

    void MyTestSuite::thread()
    {

        // you can create top level-windows here or in OnInit(). Do your testing here

        //wxUIActionSimulator simu();
        Sleep(5000);
        gui::Window::get().QueueModelEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,ID_OPTIONS));
        wxUIActionSimulator().MouseMove(100,100);
        wxUIActionSimulator().MouseClick();



        //wxTheApp->OnRun();
        //wxTheApp->OnExit();
        //wxEntryCleanup();
    }

