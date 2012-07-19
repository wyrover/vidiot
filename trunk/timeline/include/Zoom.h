#ifndef ZOOM_H
#define ZOOM_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "Part.h"
#include "UtilEvent.h"

namespace gui { namespace timeline {

typedef boost::rational<int> rational;

DECLARE_EVENT(ZOOM_CHANGE_EVENT, ZoomChangeEvent, rational);

class Zoom
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Zoom(Timeline* timeline);
	virtual ~Zoom();

    /// Change the current zoom level
    /// \param steps number of steps to change (>0 zoom in, <0 zoom out)
    void change(int steps);

    //////////////////////////////////////////////////////////////////////////
    // ZOOM RESULT
    //////////////////////////////////////////////////////////////////////////

    int timeToPixels(int time) const;       ///< @param time time duration in milliseconds
    int pixelsToTime(int pixels) const;     ///< @return time duration in milliseconds
    int pixelsToPts(int pixels) const;
    int ptsToPixels(int pts) const;

private:

    rational mZoom; ///< Number of pixels per frame

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
BOOST_CLASS_VERSION(gui::timeline::Zoom, 1)

#endif // ZOOM_H