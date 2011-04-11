#include "Preview.h"

#include <wx/sizer.h>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "Player.h"
#include "UtilLog.h"
#include "Window.h"
#include "Timeline.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Preview::Preview(wxWindow* parent)
:   wxPanel(parent)
,   mPlayer()
,   mPlayers()
{
    LOG_INFO;

    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(sizer);
}

Preview::~Preview()
{
}

//////////////////////////////////////////////////////////////////////////
// TO/FROM OTHER WIDGETS
//////////////////////////////////////////////////////////////////////////

PlayerPtr Preview::openTimeline(model::SequencePtr sequence, timeline::Timeline* timeline)
{
    ASSERT(mPlayers.find(timeline) == mPlayers.end());
    PlayerPtr newplayer = boost::make_shared<Player>(this,sequence);
    mPlayers[timeline] = newplayer;
    GetSizer()->Add(newplayer.get(),wxSizerFlags(1).Expand());
    selectTimeline(timeline);
    return newplayer;
}

void Preview::closeTimeline(timeline::Timeline* timeline)
{
    ASSERT(mPlayer);
    ASSERT(mPlayers.find(timeline) != mPlayers.end());
    selectTimeline(0);
    PlayerPtr player = mPlayers[timeline];
    hide(player);
    mPlayers.erase(timeline);
    GetSizer()->Detach(player.get());
    if (mPlayers.size() > 0)
    {
        selectTimeline(mPlayers.begin()->first);
    }
    else
    {
        mPlayer.reset();
    }
}

void Preview::selectTimeline(timeline::Timeline* timeline)
{
    hide(mPlayer);
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

void Preview::play()
{
    if (mPlayer)
    {
        mPlayer->play();
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Preview::hide(PlayerPtr player)
{
    if (player)
    {
        player->stop();
        GetSizer()->Hide(player.get());
    }
}


//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Preview::serialize(Archive & ar, const unsigned int version)
{
}
template void Preview::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Preview::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace
