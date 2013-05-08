#include "TestDialog.h"

#include "Window.h"
#include "HelperApplication.h"
#include "HelperWindow.h"
#include "Dialog.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestDialog::setUp()
{
}

void TestDialog::tearDown()
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

class Escape
{
public:
    Escape()
        :   mContinue(true)
        ,   mBarrierStop(2)
    {
        // Wait for the main window to lose focus before starting to hit Escape
        gui::Window::get().Bind( wxEVT_ACTIVATE, &Escape::onActivate, this );
    }

    ~Escape()
    {
        gui::Window::get().Unbind( wxEVT_ACTIVATE, &Escape::onActivate, this );
        mContinue = false;
        mBarrierStop.wait();
    }

private:

    void onActivate( wxActivateEvent& event )
    {
        VAR_INFO(event.GetActive());
        if (!event.GetActive() || !mThread)
        {
            mThread.reset(new boost::thread(boost::bind(&Escape::mainThread,this)));
        }
    }

    void mainThread()
    {
        while (mContinue)
        {
            pause(10);
            wxUIActionSimulator().Char(WXK_ESCAPE);
        }
        mBarrierStop.wait();
    }

    bool mContinue;
    boost::barrier mBarrierStop;
    boost::scoped_ptr<boost::thread> mThread;
};

//RUNONLY(testEscape);
void TestDialog::testEscape()
{
    StartTestSuite();
    {
        Escape e;
        wxStrings result = gui::Dialog::get().getFiles("message", _("Movie clips (*.avi)|*.avi|All files (%s)|%s"));
        ASSERT_ZERO(result.size());
    }
    {
        Escape e;
        int result = gui::Dialog::get().getConfirmation("title","message", wxOK | wxCANCEL);
        ASSERT_EQUALS(result,wxCANCEL);
    }
    {
        Escape e;
        wxString result = gui::Dialog::get().getDir("message","default");
        ASSERT_EQUALS(result,"");
    }
    {
        Escape e;
        wxString result = gui::Dialog::get().getText("title", "message","default");
        ASSERT_EQUALS(result,"");
    }
    {
        Escape e;
        triggerMenu(wxID_PREFERENCES);
        waitForIdle();
    }
    {
        Escape e;
        triggerMenu(wxID_ABOUT);
        waitForIdle();
    }
}
} // namespace