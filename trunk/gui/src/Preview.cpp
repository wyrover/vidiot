#include "Preview.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/make_shared.hpp>
#include <wx/sizer.h>
#include "Player.h"
#include "Timeline.h"
#include "UtilLog.h"

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

Player* Preview::openTimeline(model::SequencePtr sequence, timeline::Timeline* timeline)
{
    ASSERT(mPlayers.find(timeline) == mPlayers.end());
    Player* newplayer = new Player(this,sequence);
    mPlayers[timeline] = newplayer;
    GetSizer()->Add(newplayer,wxSizerFlags(1).Expand());
    selectTimeline(timeline);
    return newplayer;
}

void Preview::closeTimeline(timeline::Timeline* timeline)
{
    ASSERT(mPlayer);
    ASSERT(mPlayers.find(timeline) != mPlayers.end());
    selectTimeline(0);
    Player* player = mPlayers[timeline];
    hide(player);
    mPlayers.erase(timeline);
    GetSizer()->Detach(player);
    if (mPlayers.size() > 0)
    {
        selectTimeline(mPlayers.begin()->first);
    }
    else
    {
        mPlayer = 0;
    }
    delete player;
}

void Preview::selectTimeline(timeline::Timeline* timeline)
{
    hide(mPlayer);
    if (timeline != 0)
    {
        ASSERT(mPlayers.find(timeline) != mPlayers.end());
        mPlayer = mPlayers[timeline];
        GetSizer()->Show(mPlayer);
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

void Preview::hide(Player* player)
{
    if (player)
    {
        player->stop();
        GetSizer()->Hide(player);
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
