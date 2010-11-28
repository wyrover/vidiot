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

    /** @see View::onViewUpdated() */
    void onViewUpdated( ViewUpdateEvent& event );

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    VideoView& getVideo();
    AudioView& getAudio();

    /** @see View::requiredWidth() **/
    int requiredWidth();

    /** @see View::requiredHeight() **/
    int requiredHeight();

    int getDividerPosition() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    VideoView* mVideoView;
    AudioView* mAudioView;

    /** Y-position of audio-video divider */
    int mDividerPosition;

    /** @see View::draw() */
    void draw(wxBitmap& bitmap);

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
