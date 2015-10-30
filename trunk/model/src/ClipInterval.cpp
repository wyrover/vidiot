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
#include "Track.h"
#include "Transition.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilLogBoost.h"
#include "UtilSerializeBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ClipInterval::ClipInterval()
    : Clip()
    , mRender()
    , mSpeed(1)
    , mOffset(0)
    , mLength(-1)
    , mDescription("")
{
    // NOT: VAR_DEBUG(*this); -- Log in most derived class. Avoids duplicate logging AND avoids pure virtual calls (implemented in most derived class).
}

ClipInterval::ClipInterval(const IFilePtr& render)
    : Clip()
    , mRender(render)
    , mSpeed(1)
    , mOffset(0)
    , mLength(mRender->getLength())
    , mDescription("")
{
    // NOT: VAR_DEBUG(*this); -- Log in most derived class. Avoids duplicate logging AND avoids pure virtual calls (implemented in most derived class).
}

ClipInterval::ClipInterval(const ClipInterval& other)
    : Clip(other)
    , mRender(make_cloned<IFile>(other.mRender))
    , mSpeed(other.mSpeed)
    , mOffset(other.mOffset)
    , mLength(other.mLength)
    , mDescription(other.mDescription)
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
    VAR_DEBUG(*this)(position);
    ASSERT_LESS_THAN(position,mLength);
    ASSERT_MORE_THAN_EQUALS_ZERO(position);
    setNewStartPosition(position);
    mRender->moveTo(model::Convert::positionToNormalSpeed(mOffset + position, mSpeed));
}

wxString ClipInterval::getDescription() const
{
    if (!mDescription.IsSameAs(""))
    {
        return mDescription;
    }
    mDescription = mRender->getDescription();
    wxString strip = Config::ReadString(Config::sPathTimelineStripFromClipNames); // todo only in main thread!!! get crash here when calling this for a video file which cannot be opened anymore (from VideoClip:146)

    wxStringTokenizer t(strip, "|");
    while (t.HasMoreTokens())
    {
        wxString token = t.GetNextToken();
        if (!token.IsEmpty())
        {
            mDescription.Replace(token,_T(""),false);
        }
    }
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

void ClipInterval::setSpeed(const boost::rational<int>& speed)
{
    VAR_DEBUG(speed);
    if (speed != mSpeed)
    {
        rational oldSpeed = mSpeed;
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

boost::rational<int> ClipInterval::getSpeed() const
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
    ASSERT_LESS_THAN_EQUALS(mLength,getRenderLength() - mOffset)(adjustment)(mLength)(mRender->getLength())(mSpeed)(getRenderLength())(mOffset)(*this);;
    VAR_DEBUG(*this)(adjustment);
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
    VAR_DEBUG(*this)(adjustment);
}

//////////////////////////////////////////////////////////////////////////
// FOR PREVIEWING
//////////////////////////////////////////////////////////////////////////

pts ClipInterval::getOffset()
{
    return mOffset;
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
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const ClipInterval& obj)
{
    // Keep order same as Transition and EmptyClip for 'dump' method
    os  << static_cast<const Clip&>(obj) << '|'
        << obj.mSpeed << '|'
        << std::setw(6) << obj.mOffset << '|'
        << std::setw(6) << obj.mLength;
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
        if (version >= 2)
        {
            ar & BOOST_SERIALIZATION_NVP(mSpeed);
        }
        ar & BOOST_SERIALIZATION_NVP(mOffset);
        ar & BOOST_SERIALIZATION_NVP(mLength);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void ClipInterval::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void ClipInterval::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::ClipInterval)