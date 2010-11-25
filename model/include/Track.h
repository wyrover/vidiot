#ifndef MODEL_TRACK_H
#define MODEL_TRACK_H

#include <wx/event.h>
#include <set>
#include <boost/serialization/split_member.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "IControl.h"
#include "UtilEvent.h"
#include "UtilLog.h"

namespace model {

struct MoveParameter
{
    /** Into this track the moved clips need to be inserted. */
    TrackPtr addTrack;

    /**
    * The moved clips must be inserted before this clip.
    * If this is an uninitialized pointer, then the clips need
    * to be inserted at the end of the track.
    */
    ClipPtr addPosition;

    /**
    * Consecutive list of clips to be added to this track
    * (they'll be joined together exactly in the same order).
    * These clips may also be a part of removeClips.
    * If multiple consecutive (but not directly connected) lists
    * need to be added, add multiple MoveParameter objects.
    */
    Clips addClips;

    /** From this track the moved clips need to be removed. */
    TrackPtr removeTrack;

    /**
    * In case of undo, the removed clips must be reinserted
    * before this clip.If this is an uninitialized pointer,
    * then the clips need to be inserted at the end of
    * the track.
    */
    ClipPtr removePosition;

    /**
    * Any clips to be removed from this track
    * These clips may also be a part of addClips.
    * This needs to be a consecutive list of clips,
    * thus without 'emptyness' in between. If multiple,
    * not directly connected clip lists need to be removed,
    * instantiate two MoveParameter objects (one for each
    * list).
    */
    Clips removeClips;

    /**
    * Empty constructor (used to avoid 'no appropriate default ctor' error messages after I added the other constructor).
    **/
    MoveParameter()
        :   addTrack()
        ,   addPosition()
        ,   addClips()
        ,   removeTrack()
        ,   removePosition()
        ,   removeClips()
    {
    }

    /**
    * Helper constructor to initialize all members in one statement.
    **/
    MoveParameter(TrackPtr _addTrack, ClipPtr _addPosition, Clips _addClips, TrackPtr _removeTrack = TrackPtr(), ClipPtr _removePosition = ClipPtr(), Clips _removeClips = Clips())
        :   addTrack(_addTrack)
        ,   addPosition(_addPosition)
        ,   addClips(_addClips)
        ,   removeTrack(_removeTrack)
        ,   removePosition(_removePosition)
        ,   removeClips(_removeClips)
    {
    }
};

DECLARE_EVENT(EVENT_ADD_CLIPS,      EventAddClips,      MoveParameter);
DECLARE_EVENT(EVENT_REMOVE_CLIPS,   EventRemoveClips,   MoveParameter);

class Track
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public IControl
    ,   public boost::enable_shared_from_this<Track>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

	Track();

    Track(const Track& other);

    virtual Track* clone();

    virtual ~Track();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual boost::int64_t getNumberOfFrames();

    /** 
    * @return the pts of the starting point (left) of the clip. 
    * @pre clip is part of this track.
    */
    virtual boost::int64_t getStartFrameNumber(ClipPtr clip) const;

    virtual void moveTo(boost::int64_t position);

    //////////////////////////////////////////////////////////////////////////
    // CLIPS
    //////////////////////////////////////////////////////////////////////////

    virtual void removeClips(Clips clips);
    virtual void addClips(Clips clips, ClipPtr position = ClipPtr());

    const Clips& getClips();

    /**
    * Find the clip which provides the frame at the given pts.
    * If pts is 'on a cut' then the clip AFTER the cut is returned. 
    * If there is no clip at this pts then an empty Ptr is returned.
    **/
    ClipPtr getClip(boost::int64_t pts);

    /**
    * Find the clip following 'clip'. 
    * Returns a '0' pointer if clip is not found.
    * @pre clip is a part of this track
    **/
    ClipPtr getNextClip(ClipPtr clip);

    /**
    * Find the clip preceding 'clip'. 
    * Returns a '0' pointer if clip is not found.
    * @pre clip is a part of this track
    **/
    ClipPtr getPreviousClip(ClipPtr clip);

    //////////////////////////////////////////////////////////////////////////
    // GET & SET
    //////////////////////////////////////////////////////////////////////////

    int getHeight();

protected:

    //////////////////////////////////////////////////////////////////////////
    // ITERATION
    //////////////////////////////////////////////////////////////////////////

    bool iterate_hasClip();
    ClipPtr iterate_getClip();
    void iterate_nextClip();

private:

    Clips::const_iterator mItClips;
    Clips mClips;

    int mHeight;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const;
    template<class Archive>
    void load(Archive & ar, const unsigned int version);
    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::Track, 1)
BOOST_CLASS_EXPORT(model::Track)

#endif // MODEL_TRACK_H
