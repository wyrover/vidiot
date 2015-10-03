// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef POINTERS_H
#define POINTERS_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <vector>
#include <wx/bitmap.h>
#include <wx/image.h>

typedef boost::shared_ptr<wxBitmap> wxBitmapPtr;
typedef boost::shared_ptr<wxImage> wxImagePtr;
typedef std::vector<wxString> wxStrings;
typedef std::vector<wxFileName> wxFileNames;

namespace model {

class AudioChunk;
class AudioClip;
class AudioComposition;
class AudioFile;
class AudioTrack;
class AutoFolder;
class ClipInterval;
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
class TransitionParameter;
class VideoClip;
class VideoComposition;
class VideoFile;
class VideoFrame;
class VideoFrameLayer;
class VideoTrack;
class VideoTransition;
class WximageClip;
class WximageFile;
struct MoveParameter;
typedef boost::shared_ptr<AudioChunk> AudioChunkPtr;
typedef boost::shared_ptr<AudioClip> AudioClipPtr;
typedef boost::shared_ptr<AudioComposition> AudioCompositionPtr;
typedef boost::shared_ptr<AudioFile> AudioFilePtr;
typedef boost::shared_ptr<AudioTrack> AudioTrackPtr;
typedef boost::shared_ptr<AutoFolder> AutoFolderPtr;
typedef boost::shared_ptr<ClipInterval> ClipIntervalPtr;
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
typedef boost::shared_ptr<TransitionParameter> TransitionParameterPtr;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;
typedef boost::shared_ptr<VideoComposition> VideoCompositionPtr;
typedef boost::shared_ptr<VideoFile> VideoFilePtr;
typedef boost::shared_ptr<VideoFrame> VideoFramePtr;
typedef boost::shared_ptr<VideoFrameLayer> VideoFrameLayerPtr;
typedef boost::shared_ptr<VideoTrack> VideoTrackPtr;
typedef boost::shared_ptr<VideoTransition> VideoTransitionPtr;
typedef boost::shared_ptr<WximageClip> WximageClipPtr;
typedef boost::shared_ptr<WximageFile> WximageFilePtr;
typedef boost::weak_ptr<IClip> WeakIClipPtr;
typedef boost::weak_ptr<INode> WeakNodePtr;
typedef boost::weak_ptr<Sequence> WeakSequencPtr;
typedef boost::weak_ptr<Track> WeakTrackPtr;
typedef std::vector<AudioChunkPtr> AudioChunks;
typedef std::vector<FilePtr> Files;
typedef std::vector<IClipPtr> IClips;
typedef std::vector<MoveParameterPtr> MoveParameters;
typedef std::vector<NodePtr> NodePtrs;
typedef std::vector<TrackPtr> Tracks;
typedef std::vector<VideoFramePtr> VideoFrames;
typedef std::vector<VideoFrameLayerPtr> VideoFrameLayers;
typedef std::vector<IPathPtr> IPaths;

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
typedef std::vector<OutputFormatPtr> OutputFormatList;
typedef std::vector<ICodecParameterPtr> ICodecParameters;

}}

namespace command {

typedef std::pair<model::NodePtr ,model::NodePtr>  ParentAndChildPair;
typedef std::vector<ParentAndChildPair> ParentAndChildPairs;

}

namespace worker {

class Work;
typedef boost::shared_ptr<Work> WorkPtr;

}

#endif
