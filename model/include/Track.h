#ifndef MODEL_TRACK_H
#define MODEL_TRACK_H

#include <wx/event.h>
#include <set>
#include <boost/serialization/split_member.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "IControl.h"
#include "UtilEvent.h"

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
};
typedef boost::shared_ptr<MoveParameter> MoveParameterPtr;
typedef std::list<MoveParameterPtr> MoveParameters; // std::list because moves must be done in a particular order.

DECLARE_EVENT(EVENT_ADD_CLIPS,      EventAddClips,      MoveParameter);
DECLARE_EVENT(EVENT_REMOVE_CLIPS,   EventRemoveClips,   MoveParameter);

class Track
    :   public IControl
    ,   public wxEvtHandler
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
    virtual void moveTo(boost::int64_t position);

    //////////////////////////////////////////////////////////////////////////
    // CLIPS
    //////////////////////////////////////////////////////////////////////////

    virtual void removeClips(Clips clips);
    virtual void addClips(Clips clips, ClipPtr position);

    const Clips& getClips();

protected:

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    Clips mClips;
    Clips::const_iterator mItClips;

private:

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
