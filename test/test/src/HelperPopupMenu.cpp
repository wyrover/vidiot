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

#include "HelperPopupMenu.h"

#include <boost/thread.hpp>
#include <wx/uiaction.h>
#include "HelperWindow.h"

namespace test {

class WaitForPopup
{
public:
    WaitForPopup()
        :   mFound(false)
    {
        getTimeline().Bind(wxEVT_LEAVE_WINDOW, &WaitForPopup::onLeave, this);
    }
    ~WaitForPopup()
    {
        getTimeline().Unbind(wxEVT_LEAVE_WINDOW, &WaitForPopup::onLeave, this);
    }
    void onLeave(wxMouseEvent& event)
    {
        boost::mutex::scoped_lock lock(mMutex);
        mFound = true;
        mCondition.notify_all();
        event.Skip();
    }
    void wait()
    {
        boost::mutex::scoped_lock lock(mMutex);
        if (!mFound)
        {
            mCondition.wait(lock);
        }
    }
private:
    bool mFound;
    boost::condition_variable mCondition;
    boost::mutex mMutex;
};

void OpenPopupMenuAt(wxPoint position)
{
    Move(position);
    WaitForPopup w;
    wxUIActionSimulator().MouseClick(wxMOUSE_BTN_RIGHT);
    w.wait();
}

void ClosePopupMenu()
{
    Type(WXK_ESCAPE);
}

} // namespace