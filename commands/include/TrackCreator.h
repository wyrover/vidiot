#ifndef TRACK_CREATOR_H
#define TRACK_CREATOR_H

#include <list>
#include <boost/make_shared.hpp>

namespace model {
class INode;
typedef boost::shared_ptr<INode> NodePtr;
typedef std::list<NodePtr> NodePtrs;
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
}

namespace command {

/// Helper class for converting a list of assets to a video and an audio track
class TrackCreator 
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit TrackCreator(model::NodePtrs assets);

    ~TrackCreator();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::TrackPtr getVideoTrack();
    model::TrackPtr getAudioTrack();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::NodePtrs mAssets;
    model::TrackPtr mVideo;
    model::TrackPtr mAudio;
};

} // namespace

#endif // TRACK_CREATOR_H
