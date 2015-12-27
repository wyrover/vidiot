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

#include "ClipInterval.h"

#include "ClipEvent.h"
#include "Config.h"
#include "Convert.h"
#include "File.h"
#include "KeyFrame.h"
#include "Track.h"
#include "Transition.h"
#include "UtilSerializeBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ClipInterval::ClipInterval()
    : Clip{}
    , mRender{}
    , mSpeed{ 1 }
    , mOffset{ 0 }
    , mLength{ -1 }
    , mDescription{ "" }
    , mKeyFrames{}
    , mDefaultKeyFrame{ nullptr }
{
    // NOT: VAR_DEBUG(*this); -- Log in most derived class. Avoids duplicate logging AND avoids pure virtual calls (implemented in most derived class).
}

ClipInterval::ClipInterval(const IFilePtr& render)
    : Clip{}
    , mRender{ render }
    , mSpeed{ 1 }
    , mOffset{ 0 }
    , mLength{ render->getLength() }
    , mDescription{ stripDescription(render->getDescription()) }
    , mDefaultKeyFrame{ nullptr } // Initialized in derived class
{
    // NOT: VAR_DEBUG(*this); -- Log in most derived class. Avoids duplicate logging AND avoids pure virtual calls (implemented in most derived class).
}

ClipInterval::ClipInterval(const ClipInterval& other)
    : Clip{ other }
    , mRender{ make_cloned<IFile>(other.mRender) }
    , mSpeed{ other.mSpeed }
    , mOffset{ other.mOffset }
    , mLength{ other.mLength }
    , mDescription{ other.mDescription }
    , mKeyFrames{ make_cloned<pts, KeyFrame>(other.mKeyFrames) }
    , mDefaultKeyFrame{ other.mDefaultKeyFrame == nullptr ? nullptr : make_cloned<KeyFrame>(other.mDefaultKeyFrame) } // todo remove after adding audio key frames
{
    // NOT: VAR_DEBUG(*this); -- Log in most derived class. Avoids duplicate logging AND avoids pure virtual calls (implemented in most derived class).
}

ClipInterval::~ClipInterval()
{
    // NOT: VAR_DEBUG(*this); -- Log in most derived class. Avoids duplicate logging AND avoids pure virtual calls (implemented in most derived class).
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts ClipInterval::getLength() const
{
    return mLength;
}

void ClipInterval::moveTo(pts position)
{
    VAR_DEBUG(position)(*this);
    ASSERT_LESS_THAN(position,mLength);
    ASSERT_MORE_THAN_EQUALS_ZERO(position);
    setNewStartPosition(position);
    mRender->moveTo(model::Convert::positionToNormalSpeed(mOffset + position, mSpeed));
}

wxString ClipInterval::getDescription() const
{
    return mDescription;
}

void ClipInterval::clean()
{
    VAR_DEBUG(this);
    invalidateNewStartPosition();
    mRender->clean();
    Clip::clean();
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

void ClipInterval::setSpeed(const rational64& speed)
{
    VAR_DEBUG(speed);
    if (speed != mSpeed)
    {
        rational64 oldSpeed = mSpeed;
        pts oldOffset = mOffset;
        pts oldLength = mLength;

        mSpeed = speed;

        // Adjust offset (start at same frame/sample)
        pts newOffset = model::Convert::positionToNewSpeed(oldOffset, speed, oldSpeed);
        ASSERT_IMPLIES(speed < oldSpeed, newOffset >= mOffset);
        ASSERT_IMPLIES(speed > oldSpeed, newOffset <= mOffset);
        mOffset = newOffset;

        // Adjust length (end at same frame/sample)
        pts newLength = model::Convert::positionToNewSpeed(oldLength /*+ oldOffset*/, speed, oldSpeed);//- newOffset;
        ASSERT_IMPLIES(speed < oldSpeed, newLength >= mLength);
        ASSERT_IMPLIES(speed > oldSpeed, newLength <= mLength);
        mLength = newLength;

        ASSERT_MORE_THAN_EQUALS_ZERO(mOffset);
        ASSERT_LESS_THAN_EQUALS(mLength, getRenderLength() - mOffset)(mLength)(mRender->getLength())(mSpeed)(getRenderLength())(mOffset)(*this);
    }
}

rational64 ClipInterval::getSpeed() const
{
    return mSpeed;
}

pts ClipInterval::getMinAdjustBegin() const
{
    ASSERT(hasTrack()); // Do not call when not part of a track: the algorithm doesn't work then (for instance, with clones)
    TransitionPtr inTransition = getInTransition();
    pts reservedForInTransition = inTransition ? inTransition->getLength() : 0; // Do not use right part only. The left part (if present) is also using frames from this clip!
    pts minAdjustBegin = -mOffset + reservedForInTransition;
    ASSERT_LESS_THAN_EQUALS_ZERO(minAdjustBegin)(mOffset)(reservedForInTransition);
    return minAdjustBegin;
}

pts ClipInterval::getMaxAdjustBegin() const
{
    ASSERT(hasTrack()); // Do not call when not part of a track: the algorithm doesn't work then (for instance, with clones)
    TransitionPtr outTransition = getOutTransition();
    pts maxAdjustBegin = mLength; // NOT: - reservedForOutTransition; The 'reserved' part is already incorporated in mLength when a possible out transition was created
    ASSERT_MORE_THAN_EQUALS_ZERO(maxAdjustBegin)(mLength);
    return maxAdjustBegin;
}

void ClipInterval::adjustBegin(pts adjustment)
{
    ASSERT(!hasTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mOffset += adjustment;
    mLength -= adjustment;
    ASSERT_MORE_THAN_EQUALS_ZERO(mOffset);
    ASSERT_LESS_THAN_EQUALS(mLength,getRenderLength() - mOffset)(adjustment)(mLength)(mRender->getLength())(mSpeed)(getRenderLength())(mOffset)(*this);
    pruneKeyFrames();
    VAR_DEBUG(adjustment)(*this);
}

pts ClipInterval::getMinAdjustEnd() const
{
    ASSERT(hasTrack()); // Do not call when not part of a track: the algorithm doesn't work then (for instance, with clones)
    TransitionPtr inTransition = getInTransition();
    pts minAdjustEnd = -mLength; // NOT: + reservedForInTransition; The 'reserved' part is already incorporated in mOffset when a possible in transition was created
    ASSERT_LESS_THAN_EQUALS_ZERO(minAdjustEnd)(mLength);
    return minAdjustEnd;
}

pts ClipInterval::getMaxAdjustEnd() const
{
    ASSERT(hasTrack()); // Do not call when not part of a track: the algorithm doesn't work then (for instance, with clones)
    TransitionPtr outTransition = getOutTransition();
    pts reservedForOutTransition = outTransition ? outTransition->getLength() : 0; // Do not use left part only. The right part (if present) is also using frames from this clip!
    pts maxAdjustEnd =  getRenderLength() - mLength - mOffset - reservedForOutTransition;
    ASSERT_MORE_THAN_EQUALS_ZERO(maxAdjustEnd)(getRenderLength())(mLength)(mOffset)(reservedForOutTransition)(*this);
    return maxAdjustEnd;
}

void ClipInterval::adjustEnd(pts adjustment)
{
    ASSERT(!hasTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mLength += adjustment;
    ASSERT_LESS_THAN_EQUALS(mLength,getRenderLength() - mOffset)(adjustment)(mLength)(mRender->getLength())(mSpeed)(getRenderLength())(mOffset)(*this);
    pruneKeyFrames();
    VAR_DEBUG(adjustment)(*this);
}

//////////////////////////////////////////////////////////////////////////
// KEY FRAMES
//////////////////////////////////////////////////////////////////////////

KeyFrameMap ClipInterval::getKeyFramesOfPerceivedClip() const
{
    KeyFrameMap result;
    pts lowerBound{ getPerceivedOffset() };
    pts upperBound{ getPerceivedOffset() + getPerceivedLength() };
    for (auto k : mKeyFrames)
    {
        pts adjustedForSpeed{ model::Convert::positionToNewSpeed(k.first, getSpeed(), 1) };
        if (adjustedForSpeed >= lowerBound && adjustedForSpeed <= upperBound)
        {
            result[adjustedForSpeed - lowerBound] = k.second;
        }
    }
    return result;
}

std::pair<pts, pts> ClipInterval::getKeyFrameBoundaries(size_t index) const
{
    KeyFrameMap keyFrames{ getKeyFramesOfPerceivedClip() };

    KeyFrameMap::const_iterator it{ std::next(keyFrames.begin(), index) };
    ASSERT(it != keyFrames.end())(index)(keyFrames)(*this);

    pts current{ it->first };

    pts left{ 0 }; // For first frame the default

    if (it != keyFrames.begin())
    {
        left = std::max(left, std::prev(it)->first + 1); // +1: Ensure that there always remains a diff of at least '1' between two consecutive key frames
    }

    pts right{ getPerceivedLength() - 1}; // For last frame the default.

    if (std::next(it) != keyFrames.end())
    {
        right = std::min(right, std::next(it)->first - 1); // -1: Ensure that there always remains a diff of at least '1' between two consecutive key frames
    }

    return std::make_pair(left, right);
}

pts ClipInterval::getKeyFramePosition(size_t index) const
{
    KeyFrameMap keyFrames{ getKeyFramesOfPerceivedClip() };
    KeyFrameMap::const_iterator it{ std::next(keyFrames.begin(), index) };
    ASSERT(it != keyFrames.end())(index)(*this);
    return it->first;
}

void ClipInterval::setKeyFramePosition(size_t index, pts offset)
{
    ASSERT_MORE_THAN_EQUALS_ZERO(offset)(*this);
    ASSERT_LESS_THAN_EQUALS(offset, getPerceivedOffset() + getPerceivedLength() - 1)(*this);

    // Convert back to offset '0' (input time == no offset)
    offset += getPerceivedOffset();

    // Convert back to 1:1 speed
    pts offsetWithSpeed{ model::Convert::positionToNewSpeed(offset, 1, getSpeed()) };

    size_t nKeyFrames{ mKeyFrames.size() };
    KeyFrameMap::const_iterator it{ std::next(mKeyFrames.begin(), index) };
    ASSERT(it != mKeyFrames.end())(index)(offset)(*this);

    if (it != mKeyFrames.begin())
    {
        ASSERT_MORE_THAN(offset, std::prev(it)->first)(index)(offset)(*this);
    }
    if (std::next(it) != mKeyFrames.end())
    {
        ASSERT_LESS_THAN(offset, std::next(it)->first)(index)(offset)(*this);
    }
    if (mKeyFrames.find(offset) != mKeyFrames.end())
    {
        // Can happen during moving from the timeline. Initial events with diff '0' are generated.
        return;
    }

    model::KeyFramePtr keyFrame{ it->second };
    mKeyFrames.erase(it);
    mKeyFrames[offset] = keyFrame;
    ASSERT_EQUALS(nKeyFrames, mKeyFrames.size())(index)(offset)(*this);
    EventChangeClipKeyFrames event(0);
    ProcessEvent(event);
}

KeyFramePtr ClipInterval::getDefaultKeyFrame() const
{
    ASSERT_NONZERO(mDefaultKeyFrame);
    return mDefaultKeyFrame;
}

KeyFramePtr ClipInterval::getFrameAt(pts offset) const
{
    KeyFramePtr result{ nullptr };

    ASSERT_MORE_THAN_EQUALS_ZERO(offset);

    KeyFrameMap keyFrames{ getKeyFramesOfPerceivedClip() };

    if (keyFrames.size() == 0)
    {
        // No key frames (visible) in current trim. Return the default frame.
        ASSERT_NONZERO(mDefaultKeyFrame);
        return make_cloned<KeyFrame>(mDefaultKeyFrame);
    }

    KeyFrameMap::const_iterator itExact{ keyFrames.find(offset) };
    if (itExact != keyFrames.end())
    {
        // Exact key frame found. Return that.
        return make_cloned<KeyFrame>(itExact->second);
    }

    // No exact frame possible.
    // Return an interpolated key frame or a clone of the nearest key frame (only at begin and end).
    KeyFrameMap::const_iterator it{ keyFrames.upper_bound(offset) }; // Points to first (visible) key frame 'beyond' position.
    if (it == keyFrames.end())
    {
        // Position is after last key frame.
        // Return clone of last key frame.
        result = make_cloned<KeyFrame>(keyFrames.rbegin()->second);
    }
    else if (it == keyFrames.begin())
    {
        // The position is before the first key frame.
        // This can happen after begin trimming a clip.
        // Return clone of first key frame.
        result = make_cloned<KeyFrame>(keyFrames.begin()->second);
    }
    else
    {
        // Interpolate between the two frames 'around' position.
        pts positionAfter{ it->first };
        KeyFramePtr after{ it->second };
        ASSERT_NONZERO(after)(offset)(keyFrames)(*this);

        --it;

        pts positionBefore{ it->first };
        KeyFramePtr before{ it->second };
        ASSERT_NONZERO(before)(offset)(keyFrames)(*this);

        result = interpolate(before, after, positionBefore, offset, positionAfter);
    }
    result->setInterpolated(true);
    return result;
}

void ClipInterval::addKeyFrameAt(pts offset, KeyFramePtr frame)
{
    ASSERT_MORE_THAN_EQUALS_ZERO(offset);
    frame->setInterpolated(false);

    // Convert back to offset '0' (input time == no offset)
    offset += getPerceivedOffset();

    // Ensure that a key frame position not returned by 'getKeyFrames' was used
    KeyFrameMap keyframes{ getKeyFramesOfPerceivedClip() };
    if (keyframes.find(offset) != keyframes.end())
    {
        // At least in the module test I succeeded in pressing the Add button twice consecutively,
        // without moving the cursor. To err on the safe side, ignore the second press here, in case
        // it's also 'user possible'.
        VAR_WARNING(keyframes)(offset)(*this);
        return;
    }

    // Convert back to 1:1 speed
    pts offsetWithSpeed{ model::Convert::positionToNewSpeed(offset, 1, getSpeed()) };

    // Always clone here. Sometimes, the frames used during editing are an exact frame
    // (and thus may be edited directly), but for adding a new frame a real clone is
    // required.
    // Example: Take a clip with only one key frame (thus, from a user point of view
    //          a clip without key frames) and add another key frame. This shouldn't
    //          be the same frame as the original only frame, but a new clone.
    //
    // Note: frame is stored with 'input' speed
    mKeyFrames[offsetWithSpeed] = make_cloned<KeyFrame>(frame);

    EventChangeClipKeyFrames event(0);
    ProcessEvent(event);
}

void ClipInterval::removeKeyFrameAt(pts offset)
{
    ASSERT_DIFFERS(offset, -1);
    ASSERT_MORE_THAN_EQUALS_ZERO(offset);

    if (mKeyFrames.size() == 1)
    {
        // Removing the only remaining non-default keyframe.
        // Ensure that the resulting default frame uses the same settings
        // as that frame. Otherwise, removing the last keyframe suddenly
        // restores 'very old' settings.
        mDefaultKeyFrame = mKeyFrames.begin()->second;
    }

    // Ensure that a key frame position returned by 'getKeyFramesOfPerceivedClip' was used
    KeyFrameMap keyframes{ getKeyFramesOfPerceivedClip() };
    ASSERT_MAP_CONTAINS(keyframes, offset)(*this);

    // Convert back to offset '0' (input time == no offset)
    offset += getPerceivedOffset();

    for (KeyFrameMap::const_iterator it{ mKeyFrames.begin() }; it != mKeyFrames.end(); ++it)
    {
        if (offset == model::Convert::positionToNewSpeed(it->first, getSpeed(), 1))
        {
            mKeyFrames.erase(it);
            EventChangeClipKeyFrames event(0);
            ProcessEvent(event);
            return;
        }
    }
    // At least in the module test I succeeded in pressing the Add button twice consecutively,
    // without moving the cursor. That may also be possible for the remove button.
    // To err on the safe side, ignore the second press here, in case it's also 'user possible'.
    VAR_WARNING(keyframes)(offset)(*this);
}

void ClipInterval::setDefaultKeyFrame(KeyFramePtr keyframe)
{
    mDefaultKeyFrame = keyframe;
}

void ClipInterval::pruneKeyFrames()
{
    KeyFrameMap::const_iterator it{ mKeyFrames.begin() };
    if (it != mKeyFrames.end())
    {
        pts lowerBound{ getPerceivedOffset() };
        pts upperBound{ getPerceivedOffset() + getPerceivedLength() };
        while (it != mKeyFrames.end())
        {
            pts adjustedForSpeed{ model::Convert::positionToNewSpeed(it->first, getSpeed(), 1) };
            if (adjustedForSpeed >= lowerBound && adjustedForSpeed <= upperBound)
            {
                // Key frame is ok
                ++it;
            }
            else
            {
                // Key frame no longer visible. Remove to ensure that interpolated frames at the begin and/or end
                // use the parameters from the first and/or last key frames, respectively.
                it = mKeyFrames.erase(it);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// OFFSET
//////////////////////////////////////////////////////////////////////////

pts ClipInterval::getOffset() const
{
    return mOffset;
}

pts ClipInterval::getPerceivedOffset() const
{
    pts result{ getOffset() };
    if (model::TransitionPtr inTransition{ getInTransition() })
    {
        result -= *(inTransition->getRight()); // See getInTransition: check for getRight() not needed
    }
    return result;
}

void ClipInterval::maximize()
{
    ASSERT(!hasTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mOffset = 0;
    mLength = getRenderLength();
}

//////////////////////////////////////////////////////////////////////////
// ACCESS DATA GENERATOR
//////////////////////////////////////////////////////////////////////////

FilePtr ClipInterval::getFile() const
{
    return boost::dynamic_pointer_cast<File>(mRender);
}

pts ClipInterval::getRenderLength() const
{
    return model::Convert::positionToNewSpeed(mRender->getLength(), mSpeed, 1);
}

pts ClipInterval::getRenderSourceLength() const
{
    return mRender->getLength();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

// static
wxString ClipInterval::stripDescription(const wxString& description)
{
    wxString result{ description };
    static wxString strip = Config::ReadString(Config::sPathTimelineStripFromClipNames);
    wxStringTokenizer t(strip, "|");
    while (t.HasMoreTokens())
    {
        wxString token = t.GetNextToken();
        if (!token.IsEmpty())
        {
            result.Replace(token,_T(""),false);
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const ClipInterval& obj)
{
    // Keep order same as Transition and EmptyClip for 'dump' method
    os  << static_cast<const Clip&>(obj) << '|'
        << obj.mSpeed << '|'
        << std::setw(6) << obj.mOffset << '|'
        << std::setw(6) << obj.mLength << '|'
        << obj.mKeyFrames;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void ClipInterval::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Clip);
        ar & BOOST_SERIALIZATION_NVP(mRender);
        if (version >= 3)
        {
            ar & BOOST_SERIALIZATION_NVP(mSpeed);
        }
        else if (version >= 2)
        {
            rational32 speed;
            ar & boost::serialization::make_nvp("mSpeed", speed);
            mSpeed = rational64{ speed.numerator(),speed.denominator() };
        }
        ar & BOOST_SERIALIZATION_NVP(mOffset);
        ar & BOOST_SERIALIZATION_NVP(mLength);
        if (Archive::is_loading::value)
        {
            mDescription = stripDescription(mRender->getDescription());
        }
        if (version >= 4)
        {
            ar & BOOST_SERIALIZATION_NVP(mKeyFrames);
            ar & BOOST_SERIALIZATION_NVP(mDefaultKeyFrame);
        }
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void ClipInterval::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void ClipInterval::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::ClipInterval)
