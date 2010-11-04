#ifndef MODEL_SEQUENCE_H
#define MODEL_SEQUENCE_H

#include <boost/optional.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "AProjectViewNode.h"
#include "IAudio.h"
#include "IControl.h"
#include "IVideo.h"

namespace model {

class Sequence
    :   public IControl
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

    void addVideoTrack(VideoTrackPtr track);
    void addAudioTrack(AudioTrackPtr track);
    void removeVideoTrack(VideoTrackPtr track);
    void removeAudioTrack(AudioTrackPtr track);

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
