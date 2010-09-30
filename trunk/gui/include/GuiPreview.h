#ifndef GUI_PREVIEW_H
#define GUI_PREVIEW_H

#include <map>
#include <wx/panel.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include "GuiPtr.h"

namespace gui {

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

    PlayerPtr openTimeline(timeline::GuiTimeLinePtr timeline);
    void closeTimeline(timeline::GuiTimeLinePtr timeline);
    void selectTimeline(timeline::GuiTimeLinePtr timeline);

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    void play();

private:

    PlayerPtr mPlayer;
    std::map<timeline::GuiTimeLinePtr, PlayerPtr> mPlayers;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(gui::GuiPreview, 1)

#endif // GUI_PREVIEW_H
