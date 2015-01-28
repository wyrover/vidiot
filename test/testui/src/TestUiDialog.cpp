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

#include "TestUiDialog.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestUiDialog::setUp()
{
}

void TestUiDialog::tearDown()
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

class Escape
{
public:
    Escape()
        : mContinue(true)
        , mBarrierStop(2)
    {
        util::thread::RunInMainAndWait([this]
        {
            // Wait for the main window to lose focus before starting to hit Escape
            gui::Window::get().Bind( wxEVT_ACTIVATE, &Escape::onActivate, this );
        });
    }

    ~Escape()
    {
        util::thread::RunInMainAndWait([this]
        {
            gui::Window::get().Unbind( wxEVT_ACTIVATE, &Escape::onActivate, this );
        });
        mContinue = false;
        mBarrierStop.wait();
    }

private:

    void onActivate( wxActivateEvent& event )
    {
        VAR_INFO(event.GetActive());
        if (!event.GetActive())
        {
            try
            {
                mThread.reset(new boost::thread(boost::bind(&Escape::mainThread,this)));
            }
            catch (boost::exception &e)
            {
                FATAL(boost::diagnostic_information(e));
            }
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

    std::atomic<bool> mContinue;
    boost::barrier mBarrierStop;
    boost::scoped_ptr<boost::thread> mThread;
};

void TestUiDialog::testEscape()
{
    StartTestSuite();
    wxUIActionSimulator().MouseMove(gui::Window::get().GetScreenPosition() + wxPoint(100, 100));
    wxUIActionSimulator().MouseClick();
    WaitForIdle;
    {
        Escape e;
        StartTest("Files dialog");
        wxStrings result = gui::Dialog::get().getFiles("message", _("Movie clips (*.avi)|*.avi|All files (%s)|%s"));
        ASSERT_ZERO(result.size());
    }
    {
        Escape e;
        StartTest("Confirmation dialog");
        int result = gui::Dialog::get().getConfirmation("title", "message", wxOK | wxCANCEL);
        ASSERT_EQUALS(result,wxCANCEL);
    }
    {
        Escape e;
        StartTest("Dir dialog");
        wxString result = gui::Dialog::get().getDir("message", "default");
        ASSERT_EQUALS(result,"");
    }
    {
        Escape e;
        StartTest("Text dialog");
        wxString result = gui::Dialog::get().getText("title", "message", "default");
        ASSERT_EQUALS(result,"");
    }
    {
        Escape e;
        StartTest("Preferences dialog");
        WindowTriggerMenu(wxID_PREFERENCES);
        WaitForIdle;
    }
    {
        Escape e;
        StartTest("About dialog");
        WindowTriggerMenu(wxID_ABOUT);
        WaitForIdle;
    }
    {
        Escape e;
        StartTest("Project properties dialog");
        model::FolderPtr root = WindowCreateProject();
        ProjectViewAddSequence("sequence", root);
        WindowTriggerMenu(wxID_PROPERTIES);
        WaitForIdle;
    }
}
} // namespace
