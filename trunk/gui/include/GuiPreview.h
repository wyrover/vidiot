#ifndef GUI_PREVIEW_H
#define GUI_PREVIEW_H

#include <map>
#include <wx/panel.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include "GuiPlayer.h"
#include "GuiPtr.h"

class GuiPreview
:   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    GuiPreview(wxWindow* parent);
    virtual ~GuiPreview();

    //////////////////////////////////////////////////////////////////////////
    // TO/FROM OTHER WIDGETS
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr openTimeline(GuiTimeLinePtr timeline);
    void closeTimeline(GuiTimeLinePtr timeline);
    void selectTimeline(GuiTimeLinePtr timeline);

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    void play();

private:

    PlayerPtr mPlayer;
    std::map<GuiTimeLinePtr, PlayerPtr> mPlayers;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION 
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

BOOST_CLASS_VERSION(GuiPreview, 1)

#endif // GUI_PREVIEW_H