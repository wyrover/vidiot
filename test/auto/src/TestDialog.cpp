// Copyright 2013,2014 Eric Raijmakers.
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

#include "TestDialog.h"

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

    bool mContinue;
    boost::barrier mBarrierStop;
    boost::scoped_ptr<boost::thread> mThread;
};

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
        TriggerMenu(wxID_PREFERENCES);
        waitForIdle();
    }
    {
        Escape e;
        TriggerMenu(wxID_ABOUT);
        waitForIdle();
    }
    {
        Escape e;
        model::FolderPtr root = createProject();
        addSequence("sequence", root);
        TriggerMenu(wxID_PROPERTIES);
    }
}
} // namespace