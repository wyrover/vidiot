#ifndef MODEL_TRACK_H
#define MODEL_TRACK_H

#include <wx/event.h>
#include <set>
#include <list>
#include <boost/serialization/split_member.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include "IControl.h"
#include "UtilEvent.h"
#include "UtilLog.h"

namespace model {

// FORWARD DECLARATIONS
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef std::list<TrackPtr> Tracks;
class Clip;
typedef boost::shared_ptr<Clip> ClipPtr;
typedef boost::weak_ptr<Clip> WeakClipPtr;
typedef std::list<ClipPtr> Clips;
struct MoveParameter;
typedef boost::shared_ptr<MoveParameter> MoveParameterPtr;
typedef std::list<MoveParameterPtr> MoveParameters; // std::list because moves must be done in a particular order.

struct MoveParameter
{
    /// Into this track the moved clips need to be inserted.
    TrackPtr addTrack;

    /// The moved clips must be inserted before this clip.
    /// If this is an uninitialized pointer, then the clips need
    /// to be inserted at the end of the track.
    ClipPtr addPosition;

    /// Consecutive list of clips to be added to this track
    /// (they'll be joined together exactly in the same order).
    /// These clips may also be a part of removeClips.
    /// If multiple consecutive (but not directly connected) lists
    /// need to be added, add multiple MoveParameter objects.
    Clips addClips;

    /// From this track the moved clips need to be removed.
    TrackPtr removeTrack;

    /// In case of undo, the removed clips must be reinserted
    /// before this clip.If this is an uninitialized pointer,
    /// then the clips need to be inserted at the end of
    /// the track.
    ClipPtr removePosition;

    /// Any clips to be removed from this track
    /// These clips may also be a part of addClips.
    /// This needs to be a consecutive list of clips,
    /// thus without 'emptyness' in between. If multiple,
    /// not directly connected clip lists need to be removed,
    /// instantiate two MoveParameter objects (one for each
    /// list).
    Clips removeClips;

    /// Empty constructor (used to avoid 'no appropriate default ctor' error messages after I added the other constructor).
    MoveParameter()
        :   addTrack()
        ,   addPosition()
        ,   addClips()
        ,   removeTrack()
        ,   removePosition()
        ,   removeClips()
    {
    }

    /// Helper constructor to initialize all members in one statement.
    MoveParameter(TrackPtr _addTrack, ClipPtr _addPosition, Clips _addClips, TrackPtr _removeTrack = TrackPtr(), ClipPtr _removePosition = ClipPtr(), Clips _removeClips = Clips())
        :   addTrack(_addTrack)
        ,   addPosition(_addPosition)
        ,   addClips(_addClips)
        ,   removeTrack(_removeTrack)
        ,   removePosition(_removePosition)
        ,   removeClips(_removeClips)
    {
    }

    /// \return new move object that is the inverse of this object.
    /// This means that all additions and removals are interchanged.
    MoveParameterPtr make_inverted()
    {
        return boost::make_shared<model::MoveParameter>(removeTrack,removePosition,removeClips,addTrack,addPosition,addClips);
    }
};

DECLARE_EVENT(EVENT_ADD_CLIPS,      EventAddClips,      MoveParameter);
DECLARE_EVENT(EVENT_REMOVE_CLIPS,   EventRemoveClips,   MoveParameter);
DECLARE_EVENT(EVENT_HEIGHT_CHANGED, EventHeightChanged, int);

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

    virtual Track* clone();

    virtual ~Track();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength();
    virtual void moveTo(pts position);
    virtual wxString getDescription() const;
    virtual void clean();

    //////////////////////////////////////////////////////////////////////////
    // CLIPS
    //////////////////////////////////////////////////////////////////////////

    virtual void removeClips(Clips clips);
    virtual void addClips(Clips clips, ClipPtr position = ClipPtr());

    const Clips& getClips();

    /// Find the clip which provides the frame at the given pts.
    /// If pts is 'on a cut' then the clip AFTER the cut is returned. 
    /// If there is no clip at this pts then an empty Ptr is returned.
    ClipPtr getClip(pts position);

    /// Find the clip following 'clip'. 
    /// Returns a '0' pointer if clip is not found.
    /// \pre clip is a part of this track
    ClipPtr getNextClip(ClipPtr clip);

    /// Find the clip preceding 'clip'. 
    /// Returns a '0' pointer if clip is not found.
    /// \pre clip is a part of this track
    ClipPtr getPreviousClip(ClipPtr clip);

    /// /return size of area to the left of clip that is empty
    pts getLeftEmptyArea(model::ClipPtr clip);

    /// /return size of area to the right of clip that is empty
    pts getRightEmptyArea(model::ClipPtr clip); 

    //////////////////////////////////////////////////////////////////////////
    // FOR DETERMINING THE TYPE OF TRACK
    //////////////////////////////////////////////////////////////////////////

    template <typename Derived>
    bool isA()
    {
        return (typeid(Derived) == typeid(*this));
    }

    //////////////////////////////////////////////////////////////////////////
    // GET & SET
    //////////////////////////////////////////////////////////////////////////

    int getHeight() const;          ///< @return height of this track in the timeline view
    void setHeight(int height);     ///< @param new height of this track in the timeline view

    int getIndex() const;           ///< @return index of this track in the list of video/audio tracks
    void setIndex(int index);       ///< @param index new index of this track

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    Track(const Track& other);

    //////////////////////////////////////////////////////////////////////////
    // ITERATION
    //////////////////////////////////////////////////////////////////////////

    bool iterate_hasClip();
    ClipPtr iterate_getClip();
    void iterate_nextClip();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    Clips mClips;
    Clips::const_iterator mItClips;

    int mHeight;    ///< Height of this track when viewed in a timeline
    int mIndex;     ///< Index in the list of video/audio tracks

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////
public://TODO
    /// Updates the clips after insertion/removal etc.
    /// - Updates the pts'es for all clips in this track
    /// - Updates the clip's track pointer to this track
    void updateClips();
private://TODO
    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Track& obj );

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

std::ostream& operator<<( std::ostream& os, const MoveParameter& obj );

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::Track, 1)
BOOST_CLASS_EXPORT(model::Track)

#endif // MODEL_TRACK_H
