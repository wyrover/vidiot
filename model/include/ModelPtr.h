#ifndef MODEL_PTR_H
#define MODEL_PTR_H

// This file contains all forward declarations of model pointers/classes.

#include <list>
#include <boost/shared_ptr.hpp>

namespace model {

//////////////////////////////////////////////////////////////////////////

class Project;

//////////////////////////////////////////////////////////////////////////

class AProjectViewNode;
typedef AProjectViewNode* ProjectViewId;

typedef boost::shared_ptr<AProjectViewNode> ProjectViewPtr;
typedef std::list<ProjectViewPtr> ProjectViewPtrs;

//////////////////////////////////////////////////////////////////////////

class AudioClip;
typedef boost::shared_ptr<AudioClip> AudioClipPtr;

//////////////////////////////////////////////////////////////////////////

class AudioFile;
typedef boost::shared_ptr<AudioFile> AudioFilePtr;

//////////////////////////////////////////////////////////////////////////

class AudioTrack;
typedef boost::shared_ptr<AudioTrack> AudioTrackPtr;
typedef std::list<AudioTrackPtr> AudioTracks;

//////////////////////////////////////////////////////////////////////////

class AutoFolder;
typedef boost::shared_ptr<AutoFolder> AutoFolderPtr;

//////////////////////////////////////////////////////////////////////////

class Clip;
typedef boost::shared_ptr<Clip> ClipPtr;
typedef std::list<ClipPtr> Clips;

//////////////////////////////////////////////////////////////////////////

class File;
typedef boost::shared_ptr<File> FilePtr;

//////////////////////////////////////////////////////////////////////////

class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;

//////////////////////////////////////////////////////////////////////////

class IAudio;
typedef boost::shared_ptr<IAudio> IAudioPtr;

//////////////////////////////////////////////////////////////////////////

class IControl;
typedef boost::shared_ptr<IControl> IControlPtr;

//////////////////////////////////////////////////////////////////////////

class IVideo;
typedef boost::shared_ptr<IVideo> IVideoPtr;

//////////////////////////////////////////////////////////////////////////

class Properties;
typedef boost::shared_ptr<Properties> PropertiesPtr;

//////////////////////////////////////////////////////////////////////////

class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;

//////////////////////////////////////////////////////////////////////////

class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef std::list<TrackPtr> Tracks;

//////////////////////////////////////////////////////////////////////////

class VideoClip;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;

//////////////////////////////////////////////////////////////////////////

class VideoFile;
typedef boost::shared_ptr<VideoFile> VideoFilePtr;

//////////////////////////////////////////////////////////////////////////

class VideoTrack;
typedef boost::shared_ptr<VideoTrack> VideoTrackPtr;
typedef std::list<VideoTrackPtr> VideoTracks;

//////////////////////////////////////////////////////////////////////////

class EmptyClip;
typedef boost::shared_ptr<EmptyClip> EmptyClipPtr;

//////////////////////////////////////////////////////////////////////////

class EmptyFile;
typedef boost::shared_ptr<EmptyFile> EmptyFilePtr;

//////////////////////////////////////////////////////////////////////////

} // namespace

#endif // MODEL_PTR_H
