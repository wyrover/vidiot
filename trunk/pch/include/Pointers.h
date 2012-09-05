#ifndef POINTERS_H
#define POINTERS_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <list>
#include <vector>
#include <wx/bitmap.h>
#include <wx/image.h>

class Work;
typedef boost::shared_ptr<Work> WorkPtr;
typedef boost::shared_ptr<wxBitmap> wxBitmapPtr;
typedef boost::shared_ptr<wxImage> wxImagePtr;

namespace model {

class AudioChunk;
class AudioClip;
class AudioFile;
class AudioTrack;
class AutoFolder;
class EmptyChunk;
class EmptyClip;
class EmptyFile;
class EmptyFrame;
class File;
class Folder;
class IAudio;
class IClip;
class IControl;
class IFile;
class INode;
class IPath;
class IVideo;
class Packet;
class Properties;
class Sequence;
class Track;
class Transition;
class VideoClip;
class VideoComposition;
class VideoComposition;
class VideoFile;
class VideoFrame;
class VideoTrack;
class VideoTransition;
struct MoveParameter;
typedef boost::shared_ptr<AudioChunk> AudioChunkPtr;
typedef boost::shared_ptr<AudioClip> AudioClipPtr;
typedef boost::shared_ptr<AudioFile> AudioFilePtr;
typedef boost::shared_ptr<AudioTrack> AudioTrackPtr;
typedef boost::shared_ptr<AutoFolder> AutoFolderPtr;
typedef boost::shared_ptr<const IClip> ConstIClipPtr;
typedef boost::shared_ptr<const Track> ConstTrackPtr;
typedef boost::shared_ptr<EmptyChunk> EmptyChunkPtr;
typedef boost::shared_ptr<EmptyClip> EmptyClipPtr;
typedef boost::shared_ptr<EmptyFile> EmptyFilePtr;
typedef boost::shared_ptr<EmptyFrame> EmptyFramePtr;
typedef boost::shared_ptr<File> FilePtr;
typedef boost::shared_ptr<Folder> FolderPtr;
typedef boost::shared_ptr<IAudio> IAudioPtr;
typedef boost::shared_ptr<IClip> IClipPtr;
typedef boost::shared_ptr<IControl> IControlPtr;
typedef boost::shared_ptr<IFile> IFilePtr;
typedef boost::shared_ptr<INode> NodePtr;
typedef boost::shared_ptr<IPath> IPathPtr;
typedef boost::shared_ptr<IVideo> IVideoPtr;
typedef boost::shared_ptr<MoveParameter> MoveParameterPtr;
typedef boost::shared_ptr<Packet> PacketPtr;
typedef boost::shared_ptr<Properties> PropertiesPtr;
typedef boost::shared_ptr<Sequence> SequencePtr;
typedef boost::shared_ptr<Track> TrackPtr;
typedef boost::shared_ptr<Transition> TransitionPtr;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;
typedef boost::shared_ptr<VideoComposition> VideoCompositionPtr;
typedef boost::shared_ptr<VideoFile> VideoFilePtr;
typedef boost::shared_ptr<VideoFrame> VideoFramePtr;
typedef boost::shared_ptr<VideoTrack> VideoTrackPtr;
typedef boost::shared_ptr<VideoTransition> VideoTransitionPtr;
typedef boost::weak_ptr<IClip> WeakIClipPtr;
typedef boost::weak_ptr<INode> WeakNodePtr;
typedef boost::weak_ptr<Sequence> WeakSequencPtr;
typedef boost::weak_ptr<Track> WeakTrackPtr;
typedef std::list<AudioChunkPtr> AudioChunks;
typedef std::list<FilePtr> Files;
typedef std::list<IClipPtr> IClips;
typedef std::list<IPathPtr> IPaths;
typedef std::list<MoveParameterPtr> MoveParameters; // std::list because moves must be done in a particular order.
typedef std::list<NodePtr> NodePtrs;
typedef std::list<TrackPtr> Tracks;
typedef std::list<VideoFramePtr> VideoFrames;

namespace render {

class AudioCodec;
class OutputFormat;
class Render;
class VideoCodec;
struct ICodecParameter;
typedef boost::shared_ptr<AudioCodec> AudioCodecPtr;
typedef boost::shared_ptr<ICodecParameter> ICodecParameterPtr;
typedef boost::shared_ptr<OutputFormat> OutputFormatPtr;
typedef boost::shared_ptr<Render> RenderPtr;
typedef boost::shared_ptr<VideoCodec> VideoCodecPtr;
typedef std::list<OutputFormatPtr> OutputFormatList;
typedef std::list<ICodecParameterPtr> ICodecParameters;

}}

namespace command {

typedef std::pair<model::NodePtr ,model::NodePtr>  ParentAndChildPair;
typedef std::vector<ParentAndChildPair> ParentAndChildPairs;

}

#endif // POINTERS_H