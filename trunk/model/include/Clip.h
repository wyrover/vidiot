#ifndef MODEL_CLIP_H
#define MODEL_CLIP_H

#include <list>
#include <boost/optional.hpp>
#include <boost/weak_ptr.hpp>
#include "IControl.h"
#include "UtilLogGeneric.h"
#include "UtilEvent.h"

namespace model {

DECLARE_EVENT(EVENT_SELECT_CLIP, EventSelectClip, bool);
DECLARE_EVENT(DEBUG_EVENT_RENDER_PROGRESS, DebugEventRenderProgress, pts);

class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef std::list<TrackPtr> Tracks;

class Clip;
typedef boost::shared_ptr<Clip> ClipPtr;
typedef boost::weak_ptr<Clip> WeakClipPtr;
typedef std::list<ClipPtr> Clips;

class Clip
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public IControl
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
    // TRACK
    //////////////////////////////////////////////////////////////////////////

    /// Set the track which contains this clip. Also sets the leftmost pts
    /// of the clip inside the track. When called without parameters
    /// (thus using the defaults), this information is 'reset'.
    void setTrack(TrackPtr track = TrackPtr(), pts trackPosition = 0, unsigned int index = 0);

    /// \return the track in which this clip is contained. A null ptr is returned if the clip is not in a track.
    TrackPtr getTrack();

    /// \return pts (in containing track) of begin point of clip.
    /// The frame at this position is the first frame of this clip.
    /// The frames of a clip are [ getLeftPts,getRightPts )
    pts getLeftPts() const;

    /// \return pts (in containing track) AFTER end point of clip.
    /// The frame at this position is AFTER the last frame of this clip
    /// The frames of a clip are [ getLeftPts,getRightPts )
    pts getRightPts() const; 

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
    // ADJUSTING OFFSET AND LENGTH
    //////////////////////////////////////////////////////////////////////////
    
    pts getMinAdjustBegin() const;  ///< \return Minimum allowed value for adjustBegin given the available data for mRender.
    pts getMaxAdjustBegin() const;  ///< \return Maximum allowed value for adjustBegin given the available data for mRender.
    
    /// If adjustment is positive then move the begin point of the clip backwards
    /// in time (increase the start pts). If adjustment is negative then move the
    /// begin point of the clip forward in time (decrease the start pts).
    /// \param adjustment pts count to add/subtract from the begin point
    virtual void adjustBegin(pts adjustment);

    pts getMinAdjustEnd() const;    ///< \return Minimum allowed value for adjustEnd given the available data for mRender.
    pts getMaxAdjustEnd() const;    ///< \return Maximum allowed value for adjustEnd given the available data for mRender.

    /// Set the new length of the clip.
    /// \param adjustment pts count to add/subtract from the length
    void adjustEnd(pts adjustment);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// \return Offset in the viewed video/audio data (that is, the diff between the 
    ///         starting point of the original and the starting point of the clip)
    pts getOffset() const;

    bool getSelected() const;           ///< /return true if this clip is selected
    void setSelected(bool selected);    ///< Select or deselect clip

    /// \return pts value of most recently returned audio/video in getNext*.
    pts getGenerationProgress() const;          

    /// \param delivered value of most recently returned audio/video in getNext*.
    /// Triggers DebugEventRenderProgress.
    void setGenerationProgress(pts progress);

    pts getLeftEmptyArea();     ///< /return size of area to the left of clip that is empty
    pts getRightEmptyArea();    ///< /return size of area to the right of clip that is empty

    //////////////////////////////////////////////////////////////////////////
    // STATIC HELPER METHOD
    //////////////////////////////////////////////////////////////////////////

    static pts getCombinedLength(model::Clips clips);

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

    //////////////////////////////////////////////////////////////////////////
    // CURRENT POSITION HANDLING
    //////////////////////////////////////////////////////////////////////////

    /// This method resets mLastSetPosition. This must be called whenever there
    /// is new playback progress.
    void invalidateLastSetPosition();

    /// Return the most recent position as specified in moveTo(). This is
    /// uninitialized when there was playback progress after the moveTo.
    /// \see invalidateLastSetPosition
    boost::optional<pts> getLastSetPosition() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IControlPtr mRender;    ///< The producer of audiovisual data for this clip
    TrackPtr mTrack;        ///< Track which holds this clip
    unsigned int mIndex;    ///< Index of this clip in the track (for debugging)
    WeakClipPtr mLink;      ///< Clip that this clip is linked with. Stored as weak_ptr to avoid circular dependency between two linked clips which causes memory leaks.

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
BOOST_CLASS_EXPORT(model::Clip)

#endif // MODEL_CLIP_H
