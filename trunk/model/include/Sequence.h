#ifndef MODEL_SEQUENCE_H
#define MODEL_SEQUENCE_H

#include <map>
#include <set>
#include <list>
#include <wx/event.h>
#include <boost/optional.hpp>
#include "Node.h"
#include "IAudio.h"
#include "IControl.h"
#include "IVideo.h"

namespace model {

class VideoFrame;
typedef boost::shared_ptr<VideoFrame> VideoFramePtr;
typedef std::list<VideoFramePtr> VideoFrames;
class VideoFrameComposition;
typedef boost::shared_ptr<VideoFrameComposition> VideoFrameCompositionPtr;
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef std::list<TrackPtr> Tracks;
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;

class Sequence
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public IControl
    ,   public IVideo
    ,   public IAudio
    ,   public Node
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Sequence();

    Sequence(wxString name);

    virtual Sequence* clone() const override;

	virtual ~Sequence();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const override;
    virtual void moveTo(pts position) override;
    virtual wxString getDescription() const override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(const VideoParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels) override;

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

    TrackPtr getVideoTrack(int index);
    TrackPtr getAudioTrack(int index);

    pixel getDividerPosition() const;
    void setDividerPosition(pixel position);

    std::set<IClipPtr> getSelectedClips();

    VideoFrameCompositionPtr getVideoComposition(const VideoParameters& parameters);

    //////////////////////////////////////////////////////////////////////////
    // NODE
    //////////////////////////////////////////////////////////////////////////

    wxString        getName() const override;
    void            setName(wxString name) override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    Sequence(const Sequence& other);

private:

    wxString mName;
    pixel mDividerPosition;
    Tracks mVideoTracks;
    Tracks mAudioTracks;
    std::map<int, TrackPtr> mVideoTrackMap;
    std::map<int, TrackPtr> mAudioTrackMap;
    pts mPosition;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Update the various tracks upon insertion/removal etc.
    void updateTracks();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Sequence& obj );

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
//BOOST_CLASS_TRACKING(model::Sequence, boost::serialization::track_always)

#endif // MODEL_SEQUENCE_H