#include "ClipInterval.h"

#include "ClipEvent.h"
#include "Config.h"
#include "File.h"
#include "Track.h"
#include "Transition.h"
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
    VAR_DEBUG(*this);
}

ClipInterval::ClipInterval(IFilePtr render)
    :	Clip()
    ,   mRender(render)
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mDescription("")
{
    mLength = mRender->getLength() - mOffset;
    VAR_DEBUG(*this);
}

ClipInterval::ClipInterval(const ClipInterval& other)
    :	Clip(other)
    ,   mRender(make_cloned<IFile>(other.mRender))
    ,   mOffset(other.mOffset)
    ,   mLength(other.mLength)
    ,   mDescription(other.mDescription)
{
    VAR_DEBUG(*this)(other);
}

ClipInterval::~ClipInterval()
{
    VAR_DEBUG(*this);
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
    setLastSetPosition(position);
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
    invalidateLastSetPosition();
    setGenerationProgress(0);
    mRender->clean();
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
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ClipInterval& obj )
{
    // Keep order same as Transition for 'dump' method
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
    ar & boost::serialization::base_object<Clip>(*this);
    ar & mRender;
    ar & mOffset;
    ar & mLength;
}
template void ClipInterval::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void ClipInterval::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace