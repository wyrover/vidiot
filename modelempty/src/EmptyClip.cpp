// Copyright 2013-2016 Eric Raijmakers.
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

#include "EmptyClip.h"

#include "AudioChunk.h"
#include "Calculate.h"
#include "Transition.h"
#include "EmptyChunk.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "AudioCompositionParameters.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EmptyClip::EmptyClip()
    : Clip()
    , mLength(0)
    , mProgress(0)
{
    VAR_DEBUG(*this);
}

EmptyClip::EmptyClip(pts length)
    : Clip()
    , mLength(length)
    , mProgress(0)
{
    VAR_DEBUG(length)(*this);
}

EmptyClip::EmptyClip(const EmptyClip& other)
    : Clip(other)
    , mLength(other.mLength)
    , mProgress(0)
{
    VAR_DEBUG(other)(*this);
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
EmptyClipPtr EmptyClip::replace(const IClipPtr& original)
{
    EmptyClipPtr clip = boost::make_shared<EmptyClip>(original->getLength());
    ASSERT_EQUALS(clip->getLength(), original->getLength());
    return clip;
}

//static
EmptyClipPtr EmptyClip::replace(const model::IClips& clips)
{
    ASSERT_MORE_THAN_ZERO(clips.size());

    model::TrackPtr track = clips.front()->getTrack(); // Any clip will do, they're all part of the same track
    pts length = calculate::combinedLength(clips);
    ASSERT_MORE_THAN_ZERO(length);

    // Ensure that for regions the 'extra' space for transitions is added.
    // Basically the 'extra' space at the beginning of the first clip and the extra
    // space at the ending of the last clip must be added to the region.
    return boost::make_shared<model::EmptyClip>(length);
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
    VAR_DEBUG(position)(*this);
    ASSERT_LESS_THAN(position, mLength);
    mProgress = position;
}

void EmptyClip::setLink(const IClipPtr& link)
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
    VAR_DEBUG(adjustment)(*this);
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

const char* EmptyClip::getType() const
{
    return "Empty";
}

FilePtr EmptyClip::getFile() const
{
    return FilePtr();
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr EmptyClip::getNextAudio(const AudioCompositionParameters& parameters)
{
    if (mProgress >= getLength())
    {
        return AudioChunkPtr();
    }

    mProgress++;

    return
        boost::static_pointer_cast<AudioChunk>(
        boost::make_shared<EmptyChunk>(parameters.getNrChannels(), parameters.getChunkSize()));
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr EmptyClip::getNextVideo(const VideoCompositionParameters& parameters)
{
    if (mProgress >= getLength())
    {
        return VideoFramePtr();
    }

    mProgress++;

    return boost::static_pointer_cast<VideoFrame>(boost::make_shared<EmptyFrame>(parameters));
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const EmptyClip& obj)
{
    // Keep order same as Transition and ClipInterval for 'dump' method
    os << static_cast<const Clip&>(obj) << '|'
        << std::setw(10) << ' ' << '|'
        << std::setw(6) << obj.mLength;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void EmptyClip::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Clip);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IAudio);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IVideo);
        ar & BOOST_SERIALIZATION_NVP(mLength);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void EmptyClip::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void EmptyClip::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::EmptyClip)
