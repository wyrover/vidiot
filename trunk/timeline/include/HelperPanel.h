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

#pragma once

namespace gui { namespace timeline {
    class Timeline;

template <class PanelType>
class HelperPanel
    :   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    HelperPanel(wxWindow* parent)
        :   wxPanel(parent)
        ,   mCurrent()
        ,   mPanels()
    {
        LOG_INFO;

        wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
        SetSizer(sizer);
    }

    virtual ~HelperPanel()
    {
    }

    //////////////////////////////////////////////////////////////////////////
    // TO/FROM OTHER WIDGETS
    //////////////////////////////////////////////////////////////////////////

    PanelType* openTimeline(timeline::Timeline* timeline)
    {
        ASSERT_MAP_CONTAINS_NOT(mPanels,timeline);
        PanelType* newpanel = new PanelType(this,timeline);
        mPanels[timeline] = newpanel;
        GetSizer()->Add(newpanel,wxSizerFlags(1).Expand());
        selectTimeline(timeline);
        return newpanel;
    }

    void closeTimeline(timeline::Timeline* timeline)
    {
        ASSERT(mCurrent);
        ASSERT_MAP_CONTAINS(mPanels,timeline);
        selectTimeline(0);
        PanelType* player = mPanels[timeline];
        hide(player);
        mPanels.erase(timeline);
        GetSizer()->Detach(player);
        if (mPanels.size() > 0)
        {
            selectTimeline(mPanels.begin()->first);
        }
        else
        {
            mCurrent = 0;
        }
        delete player;
    }

    void selectTimeline(timeline::Timeline* timeline)
    {
        hide(mCurrent);
        if (timeline != 0)
        {
            ASSERT_MAP_CONTAINS(mPanels,timeline);
            mCurrent = mPanels[timeline];
            GetSizer()->Show(mCurrent);
            GetSizer()->SetDimension(wxPoint(0,0),GetSize());
            mCurrent->Layout();
        }

        GetSizer()->Layout();
    }

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    PanelType* mCurrent;
    std::map<timeline::Timeline*, PanelType*> mPanels;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void hide(PanelType* player)
    {
        if (player)
        {
            GetSizer()->Hide(player);
        }
    }
};

}} // namespace
