#ifndef DIVIDER_H
#define DIVIDER_H

#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include "Part.h"
#include "PositionInfo.h"

namespace gui { namespace timeline {
    class ViewUpdateEvent;

class Divider
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Divider(Timeline *timeline);
    virtual ~Divider();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int getPosition() const;
    void setPosition(int position);

    void getPositionInfo(wxPoint position, PointerPositionInfo& info ) const;

    int getVideoPosition() const;
    int getAudioPosition() const;

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    void draw(wxDC& dc) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    int mPosition; ///< Y-position of audio-video divider

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
BOOST_CLASS_VERSION(gui::timeline::Divider, 1)

#endif // DIVIDER_H
