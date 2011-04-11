#ifndef PREVIEW_H
#define PREVIEW_H

#include <map>
#include <wx/panel.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

namespace model {
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
}
namespace gui {
class Player;
typedef boost::shared_ptr<Player> PlayerPtr;

namespace timeline { 
    class Timeline; 
}

class Preview
    :   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Preview(wxWindow* parent);
    virtual ~Preview();

    //////////////////////////////////////////////////////////////////////////
    // TO/FROM OTHER WIDGETS
    //////////////////////////////////////////////////////////////////////////

    PlayerPtr openTimeline(model::SequencePtr sequence, timeline::Timeline* timeline);
    void closeTimeline(timeline::Timeline* timeline);
    void selectTimeline(timeline::Timeline* timeline);

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    void play();

private:

    PlayerPtr mPlayer;
    std::map<timeline::Timeline*, PlayerPtr> mPlayers;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void hide(PlayerPtr player);

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
BOOST_CLASS_VERSION(gui::Preview, 1)

#endif // PREVIEW_H
