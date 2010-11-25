#ifndef MODEL_SEQUENCE_H
#define MODEL_SEQUENCE_H

#include <wx/event.h>
#include <boost/optional.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "AProjectViewNode.h"
#include "IAudio.h"
#include "IControl.h"
#include "IVideo.h"
#include "UtilEvent.h"

namespace model {

struct TrackChange
{
    Tracks addedTracks;

    /**
    * The moved tracks must be inserted before this clip.
    * If this is an uninitialized pointer, then the tracks need
    * to be inserted at the end.
    */
    TrackPtr addPosition;

    Tracks removedTracks;

    /**
    * In case of undo, the removed tracks must be reinserted
    * before this track.If this is an uninitialized pointer,
    * then the tracks need to be inserted at the end.
    */
    TrackPtr removePosition;

    /**
    * Empty constructor (used to avoid 'no appropriate default ctor' error messages after I added the other constructor).
    **/
    TrackChange()
        :   addedTracks()
        ,   addPosition()
        ,   removedTracks()
        ,   removePosition()
    {
    }

    /**
    * Helper constructor to initialize all members in one statement.
    * Per default, when only supplying a list of tracks to be added, these
    * are added to the end.
    **/
    TrackChange(Tracks _addedTracks, TrackPtr _addPosition = TrackPtr(), Tracks _removedTracks = Tracks(), TrackPtr _removePosition = TrackPtr())
        :   addedTracks(_addedTracks)
        ,   addPosition(_addPosition)
        ,   removedTracks(_removedTracks)
        ,   removePosition(_removePosition)
    {
    }
};

DECLARE_EVENT(EVENT_ADD_VIDEO_TRACK,      EventAddVideoTracks,      TrackChange);
DECLARE_EVENT(EVENT_REMOVE_VIDEO_TRACK,   EventRemoveVideoTracks,   TrackChange);
DECLARE_EVENT(EVENT_ADD_AUDIO_TRACK,      EventAddAudioTracks,      TrackChange);
DECLARE_EVENT(EVENT_REMOVE_AUDIO_TRACK,   EventRemoveAudioTracks,   TrackChange);

class Sequence
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public IControl
    ,   public IVideo
    ,   public IAudio
    ,   public AProjectViewNode
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Sequence();

    Sequence(wxString name);

    Sequence(const Sequence& other);

    virtual Sequence* clone();

	virtual ~Sequence();

    //////////////////////////////////////////////////////////////////////////
    // SEQUENCE SPECIFIC
    //////////////////////////////////////////////////////////////////////////

    void addVideoTracks(Tracks tracks, TrackPtr position = TrackPtr());
    void addAudioTracks(Tracks tracks, TrackPtr position = TrackPtr());
    void removeVideoTracks(Tracks tracks);
    void removeAudioTracks(Tracks tracks);

    Tracks getVideoTracks();
    Tracks getAudioTracks();
    Tracks getTracks();

    //////////////////////////////////////////////////////////////////////////
    // IPROJECTVIEW
    //////////////////////////////////////////////////////////////////////////

    void            Delete();

    wxString        getName() const;
    void            setName(wxString name);

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual boost::int64_t getNumberOfFrames();
    virtual void moveTo(boost::int64_t position);

	//////////////////////////////////////////////////////////////////////////
	// IVIDEO
	//////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(int requestedWidth, int requestedHeight, bool alpha = true);

	//////////////////////////////////////////////////////////////////////////
	// IAUDIO
	//////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels);

private:

    wxString mName;
    Tracks mVideoTracks;
    Tracks mAudioTracks;

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
BOOST_CLASS_VERSION(model::Sequence, 1)
BOOST_CLASS_EXPORT(model::Sequence)
BOOST_CLASS_TRACKING(model::Sequence, boost::serialization::track_always)

#endif // MODEL_SEQUENCE_H
