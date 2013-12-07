// Copyright 2013 Eric Raijmakers.
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
#include "File.h"
#include "Track.h"
#include "Transition.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilSerializeBoost.h"

namespace model {

    //////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ClipInterval::ClipInterval()
    :	Clip()
    ,   mRender()
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mDescription("")
{
    // NOT: VAR_DEBUG(*this); -- Log in most derived class. Avoids duplicate logging AND avoids pure virtual calls (implemented in most derived class).
}

ClipInterval::ClipInterval(IFilePtr render)
    :	Clip()
    ,   mRender(render)
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mDescription("")
{
    mLength = mRender->getLength() - mOffset;
    // NOT: VAR_DEBUG(*this); -- Log in most derived class. Avoids duplicate logging AND avoids pure virtual calls (implemented in most derived class).
}

ClipInterval::ClipInterval(const ClipInterval& other)
    :	Clip(other)
    ,   mRender(make_cloned<IFile>(other.mRender))
    ,   mOffset(other.mOffset)
    ,   mLength(other.mLength)
    ,   mDescription(other.mDescription)
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
    setGenerationProgress(0);
    mRender->moveTo(mOffset + position);
}

wxString ClipInterval::getDescription() const
{
    if (!mDescription.IsSameAs(""))
    {
        return mDescription;
    }
    mDescription = mRender->getDescription();
    wxString strip = Config::ReadString(Config::sPathStrip);

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
    setGenerationProgress(0);
    mRender->clean();
    Clip::clean();
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

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
    ASSERT_LESS_THAN_EQUALS(mLength,mRender->getLength() - mOffset);
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
    pts maxAdjustEnd =  mRender->getLength() - mLength - mOffset - reservedForOutTransition;
    ASSERT_MORE_THAN_EQUALS_ZERO(maxAdjustEnd)(mRender->getLength())(mLength)(mOffset)(reservedForOutTransition);
    return maxAdjustEnd;
}

void ClipInterval::adjustEnd(pts adjustment)
{
    ASSERT(!hasTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mLength += adjustment;
    ASSERT_LESS_THAN_EQUALS(mLength,mRender->getLength() - mOffset);
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
    mLength = mRender->getLength();
}

//////////////////////////////////////////////////////////////////////////
// ACCESS DATA GENERATOR
//////////////////////////////////////////////////////////////////////////

FilePtr ClipInterval::getFile() const
{
    return boost::dynamic_pointer_cast<File>(mRender);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ClipInterval& obj )
{
    // Keep order same as Transition and EmptyClip for 'dump' method
    os  << static_cast<const Clip&>(obj) << '|'
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
        ar & BOOST_SERIALIZATION_NVP(mOffset);
        ar & BOOST_SERIALIZATION_NVP(mLength);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void ClipInterval::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void ClipInterval::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::ClipInterval)