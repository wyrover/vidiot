#include "EmptyClip.h"

#include "AudioChunk.h"
#include "Calculate.h"
#include "EmptyFile.h"
#include "Transition.h"
#include "EmptyChunk.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "AudioCompositionParameters.h"
#include "UtilLog.h"
#include "VideoCompositionParameters.h"
#include "UtilLog.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

// todo remove emptyfile and remove inheritance of emptyclip to clip...

EmptyClip::EmptyClip()
    :	Clip()
    ,   mLength(0)
    ,   mProgress(0)
{
    VAR_DEBUG(this);
}

EmptyClip::EmptyClip(pts length)
    :	Clip()
    ,   mLength(length)
    ,   mProgress(0)
{
    VAR_DEBUG(this)(length);
}

EmptyClip::EmptyClip(const EmptyClip& other)
    :   Clip(other)
    ,   mLength(other.mLength)
    ,   mProgress(0)
{
    VAR_DEBUG(this);
}

EmptyClip* EmptyClip::clone() const
{
    return new EmptyClip(static_cast<const EmptyClip&>(*this));
}

EmptyClip::~EmptyClip()
{
    VAR_DEBUG(this);
}

// static
EmptyClipPtr EmptyClip::replace( IClipPtr original )
{
    EmptyClipPtr clip = boost::make_shared<EmptyClip>(original->getLength());
    ASSERT_EQUALS(clip->getLength(),original->getLength());
    return clip;
}

//static
EmptyClipPtr EmptyClip::replace(model::IClips clips)
{
    ASSERT_MORE_THAN_ZERO(clips.size());

    model::TrackPtr track = clips.front()->getTrack(); // Any clip will do, they're all part of the same track
    pts length = calculate::combinedLength(clips);
    ASSERT_MORE_THAN_ZERO(length);

    // Ensure that for regions the 'extra' space for transitions is added.
    // Basically the 'extra' space at the beginning of the first clip and the extra
    // space at the ending of the last clip must be added to the region.
    return boost::make_shared<model::EmptyClip>(length);//todo test , -1 * clips.front()->getMinAdjustBegin(), clips.back()->getMaxAdjustEnd());
}

//////////////////////////////////////////////////////////////////////////
// CLIP
//////////////////////////////////////////////////////////////////////////

pts EmptyClip::getLength() const
{
    return mLength;
}

void EmptyClip::moveTo(pts position)
{
    VAR_DEBUG(*this)(position);
    ASSERT_LESS_THAN(position,mLength);
    mProgress = position;
    setGenerationProgress(0);
}

void EmptyClip::setLink(IClipPtr link)
{
    // Empty clips may never be linked to anything
    ASSERT(!link)(link);
}

pts EmptyClip::getMinAdjustBegin() const
{
    return std::numeric_limits<pts>::min();
}

pts EmptyClip::getMaxAdjustBegin() const
{
    return mLength;
}

void EmptyClip::adjustBegin(pts adjustment)
{
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mLength -= adjustment;
    VAR_DEBUG(*this);
    ASSERT_MORE_THAN_EQUALS_ZERO(mLength)(adjustment);
}

pts EmptyClip::getMinAdjustEnd() const
{
    return mLength;
}

pts EmptyClip::getMaxAdjustEnd() const
{
    return std::numeric_limits<pts>::max();
}

void EmptyClip::adjustEnd(pts adjustment)
{
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mLength += adjustment;
    VAR_DEBUG(*this)(adjustment);
    ASSERT_MORE_THAN_EQUALS_ZERO(mLength)(adjustment);
}

std::set<pts> EmptyClip::getCuts(const std::set<IClipPtr>& exclude) const
{
    // EmptyClips are always adjacent to 'regular' clips. Thus, there is no need
    // to add the cuts for the empty clips.
    // Furthermore, an EmptyClip can be adjacent to an excluded clip. Therefore, the
    // EmptyClip should not cause the cuts of the adjacent excluded clips to be added.
    return std::set<pts>();
}

std::ostream& EmptyClip::dump(std::ostream& os) const
{
    os << *this;
    return os;
}

char* EmptyClip::getType() const
{
    return "Empty";
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr EmptyClip::getNextAudio(const AudioCompositionParameters& parameters)
{
    if (mProgress > getLength())
    {
        return AudioChunkPtr();
    }

    AudioChunkPtr audioChunk = boost::static_pointer_cast<AudioChunk>(boost::make_shared<EmptyChunk>(parameters.getNrChannels(), parameters.ptsToSamples(1), 0));
    audioChunk->setPts(mProgress);
    setGenerationProgress(mProgress);
    mProgress++;
    VAR_AUDIO(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr EmptyClip::getNextVideo(const VideoCompositionParameters& parameters)
{
    if (mProgress > getLength())
    {
        return VideoFramePtr();
    }

    VideoFramePtr videoFrame = boost::static_pointer_cast<VideoFrame>(boost::make_shared<EmptyFrame>(parameters.getBoundingBox(), 0));
    videoFrame->setPts(mProgress);
    setGenerationProgress(mProgress);
    mProgress++;
    VAR_VIDEO(videoFrame);
    VAR_ERROR(getLength())(videoFrame);
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const EmptyClip& obj )
{
    os << static_cast<const Clip&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void EmptyClip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Clip>(*this);
    ar & boost::serialization::base_object<IAudio>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
    ar & mLength;

}
template void EmptyClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void EmptyClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace