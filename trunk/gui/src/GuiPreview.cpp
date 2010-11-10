#include "GuiPreview.h"

#include <wx/sizer.h>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "GuiPlayer.h"
#include "UtilLog.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

GuiPreview::GuiPreview(wxWindow* parent)
:   wxPanel(parent)
,   mPlayer()
,   mPlayers()
{
    LOG_INFO;

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
}

GuiPreview::~GuiPreview()
{
    mPlayer.reset();
    mPlayers.clear();
}

//////////////////////////////////////////////////////////////////////////
// TO/FROM OTHER WIDGETS
//////////////////////////////////////////////////////////////////////////

PlayerPtr GuiPreview::openTimeline(timeline::GuiTimeLine* timeline)
{
    ASSERT(mPlayers.find(timeline) == mPlayers.end());
    PlayerPtr newplayer = boost::make_shared<GuiPlayer>(this,timeline);
    mPlayers[timeline] = newplayer;
    GetSizer()->Add(newplayer.get(),wxSizerFlags(1).Expand());
    selectTimeline(timeline);
    return newplayer;
}

void GuiPreview::closeTimeline(timeline::GuiTimeLine* timeline)
{
    ASSERT(mPlayer);
    ASSERT(mPlayers.find(timeline) != mPlayers.end());
    PlayerPtr player = mPlayers[timeline];
    mPlayers.erase(timeline);
    GetSizer()->Detach(player.get());
}

void GuiPreview::selectTimeline(timeline::GuiTimeLine* timeline)
{
    if (mPlayer)
    {
        mPlayer->stop();
        GetSizer()->Hide(mPlayer.get());
    }

    if (timeline != 0)
    {
        ASSERT(mPlayers.find(timeline) != mPlayers.end());
        mPlayer = mPlayers[timeline];
        GetSizer()->Show(mPlayer.get());
    }

    GetSizer()->Layout();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

void GuiPreview::play()
{
    if (mPlayer)
    {
        mPlayer->play();
    }
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiPreview::serialize(Archive & ar, const unsigned int version)
{
}
template void GuiPreview::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiPreview::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace
