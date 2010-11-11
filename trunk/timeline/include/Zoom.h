#ifndef GUI_TIME_LINE_ZOOM_H
#define GUI_TIME_LINE_ZOOM_H

#include <boost/rational.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include "Part.h"

namespace gui { namespace timeline {

typedef boost::rational<int> rational;

class Zoom
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Zoom();
	virtual ~Zoom();

    //////////////////////////////////////////////////////////////////////////
    // ZOOM RESULT
    //////////////////////////////////////////////////////////////////////////

    /** /param time time duration in milliseconds. */
    int timeToPixels(int time) const;

    /** /return time duration in milliseconds. */
    int pixelsToTime(int pixels) const;

    int pixelsToPts(int pixels) const;

    int ptsToPixels(int pts) const;

    /**
    * Is static since it is independent of the chosen zoom factor.
    * /param time time duration in milliseconds.
    */
    static int timeToPts(int time);

    /**
    * Is static since it is independent of the chosen zoom factor.
    * /return time duration in milliseconds.
    */
    static int ptsToTime(int pts);

    /**
    * Is static since it is independent of the chosen zoom factor.
    * /return time duration in microseconds.
    */
    static int ptsToMicroseconds(int pts);

    /**
    * Is static since it is independent of the chosen zoom factor.
    */
    static int microsecondsToPts(int us);

private:

    /** Indicates the number of pixels per frame. */
    rational mZoom;

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
BOOST_CLASS_EXPORT(gui::timeline::Zoom)

#endif // GUI_TIME_LINE_ZOOM_H
