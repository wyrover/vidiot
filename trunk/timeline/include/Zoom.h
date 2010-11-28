#ifndef ZOOM_H
#define ZOOM_H

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

    Zoom(Timeline* timeline);
	virtual ~Zoom();

    //////////////////////////////////////////////////////////////////////////
    // ZOOM RESULT
    //////////////////////////////////////////////////////////////////////////

    int timeToPixels(int time) const;       ///< @param time time duration in milliseconds
    int pixelsToTime(int pixels) const;     ///< @return time duration in milliseconds
    int pixelsToPts(int pixels) const;
    int ptsToPixels(int pts) const;

    // Statics: independent of the chosen zoom factor.
    static int timeToPts(int time);         ///< @param time time duration in milliseconds 
    static int ptsToTime(int pts);          ///< @return time duration in milliseconds
    static int ptsToMicroseconds(int pts);  ///< @return time duration in microseconds.
    static int microsecondsToPts(int us);   ///< @param us time durinage in microseconds 

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
