#ifndef MODEL_CLIP_H
#define MODEL_CLIP_H

#include <list>
#include <boost/optional.hpp>
#include <boost/weak_ptr.hpp>
#include "IClip.h"
#include "UtilLogGeneric.h"
#include "UtilEvent.h"

namespace model {

DECLARE_EVENT(EVENT_SELECT_CLIP, EventSelectClip, bool);
DECLARE_EVENT(DEBUG_EVENT_RENDER_PROGRESS, DebugEventRenderProgress, pts);

class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef boost::weak_ptr<Track> WeakTrackPtr;
typedef std::list<TrackPtr> Tracks;

class Clip
    :   public IClip
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Clip();                     ///< Constructor for recovery from disk

    Clip(IControlPtr render);   ///< Constructor for creating new clip from other asset

    virtual Clip* clone();      ///< Used for making deep copies (clones) 

    virtual ~Clip();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength();
    virtual void moveTo(pts position);
    virtual wxString getDescription() const;
    virtual void clean();
    
    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    virtual void setTrack(TrackPtr track = TrackPtr(), pts trackPosition = 0, unsigned int index = 0);
    virtual TrackPtr getTrack();
    virtual pts getLeftPts() const;
    virtual pts getRightPts() const; 

    virtual void setLink(IClipPtr link);
    virtual IClipPtr getLink() const;

    virtual pts getMinAdjustBegin() const;
    virtual pts getMaxAdjustBegin() const;
    virtual void adjustBegin(pts adjustment);

    virtual pts getMinAdjustEnd() const;
    virtual pts getMaxAdjustEnd() const;
    virtual void adjustEnd(pts adjustment);

    virtual bool getSelected() const;
    virtual void setSelected(bool selected);

    virtual pts getGenerationProgress() const;          
    virtual void setGenerationProgress(pts progress);

    void invalidateLastSetPosition();
    boost::optional<pts> getLastSetPosition() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \note the clone is not automatically part of the track!!!
    /// \see make_cloned
    Clip(const Clip& other);

    //////////////////////////////////////////////////////////////////////////
    // ACCESS RENDERING OBJECT
    //////////////////////////////////////////////////////////////////////////

    /// Method needed to convert the IControl instance mRender to an IVideo
    /// or an IAudio instance.
    template <class GENERATOR>
    boost::shared_ptr<GENERATOR> getDataGenerator()
    {
        return boost::static_pointer_cast<GENERATOR>(mRender);
    }

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IControlPtr mRender;    ///< The producer of audiovisual data for this clip
    WeakTrackPtr mTrack;    ///< Track which holds this clip
    unsigned int mIndex;    ///< Index of this clip in the track (for debugging)
    WeakIClipPtr mLink;     ///< Clip that this clip is linked with. Stored as weak_ptr to avoid circular dependency between two linked clips which causes memory leaks.

    pts mOffset;            ///< Offset inside the original media file (start point)
    pts mLength;            ///< Length of the clip
    pts mLeftPtsInTrack;    ///< Position inside the track. 0 if not in a track.

    boost::optional<pts> mLastSetPosition;  ///< The most recent position as specified in 'moveTo()'.
    pts mGeneratedPts;                      ///< (approximate) pts value of last video/audio returned with getNext*

    bool mSelected;                         ///< True if this clip is currently selected

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

#endif // MODEL_CLIP_H
