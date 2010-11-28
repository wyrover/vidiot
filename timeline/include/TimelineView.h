#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "View.h"

namespace gui { namespace timeline {

class TimelineView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    TimelineView(Timeline* timeline);
    virtual ~TimelineView();

    //////////////////////////////////////////////////////////////////////////
    // PROPAGATE UPDATES UPWARD
    //////////////////////////////////////////////////////////////////////////

    void onViewUpdated( ViewUpdateEvent& event ); ///< @see View::onViewUpdated()

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    VideoView& getVideo();
    AudioView& getAudio();

    int requiredWidth();  ///< @see View::requiredWidth()
    int requiredHeight(); ///< @see View::requiredHeight()

    int getDividerPosition() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    VideoView* mVideoView;
    AudioView* mAudioView;

    int mDividerPosition; ///< Y-position of audio-video divider

    void draw(wxBitmap& bitmap); ///< @see View::draw()

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(gui::timeline::TimelineView, 1)

#endif // TIMELINEVIEW_H
