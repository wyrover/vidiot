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

#include "TestMenu.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestMenu::setUp()
{
    mProjectFixture.init();
}

void TestMenu::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestMenu::testTogglingPanes()
{
    StartTestSuite();

    auto ASSERT_PANE_SHOWN = [](const wxString& name)
    {
        wxAuiPaneInfo& pane = gui::Window::get().getUiManager().GetPane(name);
        ASSERT(pane.IsOk() && pane.IsShown());
    };
    auto ASSERT_PANE_HIDDEN = [](const wxString& name)
    {
        wxAuiPaneInfo& pane = gui::Window::get().getUiManager().GetPane(name);
        ASSERT(pane.IsOk() && !pane.IsShown());
    };

    WindowTriggerMenu(wxID_HELP); // hidden by default
    ASSERT_PANE_SHOWN(gui::Window::sPaneNameProject);
    ASSERT_PANE_SHOWN(gui::Window::sPaneNameDetails);
    ASSERT_PANE_SHOWN(gui::Window::sPaneNamePreview);
    ASSERT_PANE_SHOWN(gui::Window::sPaneNameTimelines);
    ASSERT_PANE_SHOWN(gui::Window::sPaneNameHelp);

    StartTest("Hiding and showing panes.");
    for (auto id_Name : gui::Window::sMapMenuIdToPaneName)
    {
        WindowTriggerMenu(id_Name.first);
        ASSERT_PANE_HIDDEN(id_Name.second);
        WindowTriggerMenu(id_Name.first);
        ASSERT_PANE_SHOWN(id_Name.second);
    }
}

} // namespaceh