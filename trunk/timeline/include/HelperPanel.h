#ifndef TIMELINEHELPERPANEL_H
#define TIMELINEHELPERPANEL_H

#include <wx/sizer.h>
#include <wx/panel.h>
#include "UtilLog.h"

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
            player->onHide();
            GetSizer()->Hide(player);
        }
    }
};

}} // namespace

#endif // TIMELINEHELPERPANEL_H