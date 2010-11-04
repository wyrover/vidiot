#ifndef MODEL_CLIP_H
#define MODEL_CLIP_H

#include "IControl.h"
#include "UtilLogGeneric.h"

namespace model {

class Clip
    :   public IControl
    //,   public ILoggable // todo move to base classes
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Clip();

    Clip(IControlPtr clip);

    Clip(const Clip& other);

    virtual Clip* clone();

    virtual ~Clip();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual boost::int64_t getNumberOfFrames();
    virtual void moveTo(boost::int64_t position);

    //////////////////////////////////////////////////////////////////////////
    // TRACK
    //////////////////////////////////////////////////////////////////////////

    void setTrack(TrackPtr track, boost::int64_t trackPosition);
    TrackPtr getTrack();

    boost::int64_t getLeftPts() const;
    boost::int64_t getRightPts() const;

    //////////////////////////////////////////////////////////////////////////
    // LINK
    //////////////////////////////////////////////////////////////////////////

    void setLink(ClipPtr link);
    ClipPtr getLink() const;

    //////////////////////////////////////////////////////////////////////////
    // FOR DETERMINING THE TYPE OF CLIP
    //////////////////////////////////////////////////////////////////////////

    template <typename Derived>
    bool isA()
    {
        return (typeid(Derived) == typeid(*this));
    }

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    boost::int64_t getOffset();
    void setOffset(boost::int64_t offset);

    void setLength(boost::int64_t length);

    /**
    * If adjustment is positive then move the begin point of the clip backwards
    * in time (increase the start pts). If adjustment is negative then move the
    * begin point of the clip forward in time (decrease the start pts).
    **/
    void adjustBeginPoint(boost::int64_t adjustment);

    /**
    * If adjustment is positive then move the end point of the clip backwards
    * in time (increase the end pts). If adjustment is negative then move the
    * end point of the clip forward in time (decrease the end pts).
    **/
    void adjustEndPoint(boost::int64_t adjustment);

protected:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IControlPtr mRender;
    TrackPtr mTrack;
    ClipPtr mLink;

private:

    boost::int64_t mOffset;         // Offset inside the original media file (start point)
    boost::int64_t mLength;         // Length of the clip
    boost::int64_t mTrackPosition;  // Position inside the track. 0 if not in a track.

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Clip& obj );

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
BOOST_CLASS_VERSION(model::Clip, 1)
BOOST_CLASS_EXPORT(model::Clip)

#endif // MODEL_CLIP_H
