#include "Clip.h"

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

Clip::Clip()
    :	IClip()
    ,   mRender()
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mTrack()
    ,   mIndex(0)
    ,   mLeftPtsInTrack(0)
    ,   mLink()
    ,   mLastSetPosition(boost::none)
    ,   mSelected(false)
    ,   mDragged(false)
    ,   mGeneratedPts(0)
{
    VAR_DEBUG(*this);
}

Clip::Clip(IFilePtr render)
    :	IClip()
    ,   mRender(render)
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mTrack()
    ,   mIndex(0)
    ,   mLeftPtsInTrack(0)
    ,   mLink()
    ,   mLastSetPosition(boost::none)
    ,   mSelected(false)
    ,   mDragged(false)
    ,   mGeneratedPts(0)
{
    mLength = mRender->getLength() - mOffset;
    VAR_DEBUG(*this);
}

Clip::Clip(const Clip& other)
    :	IClip(other)
    ,   mRender(make_cloned<IFile>(other.mRender))
    ,   mOffset(other.mOffset)
    ,   mLength(other.mLength)
    ,   mTrack(model::TrackPtr())   // Clone is not automatically part of same track!!!
    ,   mIndex(0)                   // Clone is not automatically part of same track!!!
    ,   mLeftPtsInTrack(0)          // Clone is not automatically part of same track!!!
    ,   mLink()                     // Clone is not automatically linked to same clip, since it will typically be used in ClipEdit derived classes, using link mapping for maintaining the links
    ,   mLastSetPosition(boost::none)
    ,   mSelected(other.mSelected)
    ,   mDragged(false)             // Clone is not automatically also dragged!!!
    ,   mGeneratedPts(0)
{
    VAR_DEBUG(*this)(other);
}

Clip* Clip::clone() const
{
    return new Clip(static_cast<const Clip&>(*this));
}

Clip::~Clip()
{
    VAR_DEBUG(*this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts Clip::getLength() const
{
    return mLength;
}

void Clip::moveTo(pts position)
{
    VAR_DEBUG(*this)(position);
    ASSERT_LESS_THAN(position,mLength);
    mLastSetPosition.reset(position);
    setGenerationProgress(0);
    mRender->moveTo(mOffset + position);
}

wxString Clip::getDescription() const
{
    wxString s = mRender->getDescription();
    wxString strip = Config::ReadString(Config::sPathStrip);

    wxStringTokenizer t(strip, "|"); // PERF cache for performance
    while (t.HasMoreTokens())
    {
        wxString token = t.GetNextToken();
        if (!token.IsEmpty())
        {
            s.Replace(token,_T(""),false);
        }
    }
    return s;
}

void Clip::clean()
{
    VAR_DEBUG(this);
    mLastSetPosition = boost::none;
    mGeneratedPts = 0;
    mRender->clean();
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

void Clip::setTrack(TrackPtr track, pts trackPosition, unsigned int index)
{
    mIndex = index;
    mTrack = track;
    mLeftPtsInTrack = trackPosition;
}

TrackPtr Clip::getTrack()
{
    return mTrack.lock();
}

pts Clip::getLeftPts() const
{
    return mLeftPtsInTrack;
}

pts Clip::getRightPts() const
{
    return mLeftPtsInTrack + mLength;
}

void Clip::setLink(IClipPtr link)
{
    mLink = WeakIClipPtr(link);
}

IClipPtr Clip::getLink() const
{
    return mLink.lock();
}

pts Clip::getOffset() const
{
    return mOffset;
}

pts Clip::getMinAdjustBegin() const
{
    ASSERT(mTrack.lock()); // Do not call when not part of a track: the algorithm doesn't work then (for instance, with clones)
    TransitionPtr inTransition = getInTransition();
    pts reservedForInTransition = inTransition ? inTransition->getLength() : 0; // Do not use right part only. The left part (if present) is also using frames from this clip!
    pts minAdjustBegin = -mOffset + reservedForInTransition;
    ASSERT_LESS_THAN_EQUALS_ZERO(minAdjustBegin)(mOffset)(reservedForInTransition);
    return minAdjustBegin;
}

pts Clip::getMaxAdjustBegin() const
{
    ASSERT(mTrack.lock()); // Do not call when not part of a track: the algorithm doesn't work then (for instance, with clones)
    TransitionPtr outTransition = getOutTransition();
    pts maxAdjustBegin = mLength; // NOT: - reservedForOutTransition; The 'reserved' part is already incorporated in mLength when a possible out transition was created
    ASSERT_MORE_THAN_EQUALS_ZERO(maxAdjustBegin)(mLength);
    return maxAdjustBegin;
}

void Clip::adjustBegin(pts adjustment)
{
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mOffset += adjustment;
    mLength -= adjustment;
    ASSERT_LESS_THAN_EQUALS(mLength,mRender->getLength() - mOffset);
    VAR_DEBUG(*this)(adjustment);
}

pts Clip::getMinAdjustEnd() const
{
    ASSERT(mTrack.lock()); // Do not call when not part of a track: the algorithm doesn't work then (for instance, with clones)
    TransitionPtr inTransition = getInTransition();
    pts minAdjustEnd = -mLength; // NOT: + reservedForInTransition; The 'reserved' part is already incorporated in mOffset when a possible in transition was created
    ASSERT_LESS_THAN_EQUALS_ZERO(minAdjustEnd)(mLength);
    return minAdjustEnd;
}

pts Clip::getMaxAdjustEnd() const
{
    ASSERT(mTrack.lock()); // Do not call when not part of a track: the algorithm doesn't work then (for instance, with clones)
    TransitionPtr outTransition = getOutTransition();
    pts reservedForOutTransition = outTransition ? outTransition->getLength() : 0; // Do not use left part only. The right part (if present) is also using frames from this clip!
    pts maxAdjustEnd =  mRender->getLength() - mLength - mOffset - reservedForOutTransition;
    ASSERT_MORE_THAN_EQUALS_ZERO(maxAdjustEnd)(mRender->getLength())(mLength)(mOffset)(reservedForOutTransition);
    return maxAdjustEnd;
}

void Clip::adjustEnd(pts adjustment)
{
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mLength += adjustment;
    ASSERT_LESS_THAN_EQUALS(mLength,mRender->getLength() - mOffset);
    VAR_DEBUG(*this)(adjustment);
}

bool Clip::getSelected() const
{
    return mSelected;
}

void Clip::setSelected(bool selected)
{
    if (mSelected != selected)
    {
        mSelected = selected;
        ProcessEvent(EventSelectClip(selected));
    }
}

bool Clip::getDragged() const
{
    return mDragged;
}

void Clip::setDragged(bool dragged)
{
    mDragged = dragged;
    ProcessEvent(EventDragClip(dragged));
}

pts Clip::getGenerationProgress() const
{
    return mGeneratedPts;
}

void Clip::setGenerationProgress(pts progress)
{
    // Note: the condition "Config::getShowDebugInfo()" was added to avoid
    //       generating these events in case a sequence is being rendered.
    //       If a sequence is rendered, these events generate updates of the
    //       sequence's timeline's view classes. That, in turn, causes all
    //       sorts of threading issues. In general: When a sequence is rendered
    //       and no changes to the sequence (or its tracks/clips/etc.) may be
    //       made. That includes the 'render progress' event.
    if (Config::getShowDebugInfo() && mGeneratedPts != progress)
    {
        mGeneratedPts = progress;
        ProcessEvent(DebugEventRenderProgress(mGeneratedPts));
    }
}

void Clip::invalidateLastSetPosition()
{
    mLastSetPosition = boost::none;
}

boost::optional<pts> Clip::getLastSetPosition() const
{
    return mLastSetPosition;
}

//////////////////////////////////////////////////////////////////////////
// FOR PREVIEWING
//////////////////////////////////////////////////////////////////////////

void Clip::maximize()
{
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mOffset = 0;
    mLength = mRender->getLength();
}

//////////////////////////////////////////////////////////////////////////
// ADJACENT TRANSITION HANDLING
//////////////////////////////////////////////////////////////////////////

TransitionPtr Clip::getInTransition() const
{
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(boost::const_pointer_cast<IClip>(getPrev()));
    if (transition && transition->getRight() > 0)
    {
        return transition;
    }
    return model::TransitionPtr();
}

TransitionPtr Clip::getOutTransition() const
{
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(boost::const_pointer_cast<IClip>(getNext()));
    if (transition && transition->getLeft() > 0)
    {
        return transition;
    }
    return model::TransitionPtr();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Clip& obj )
{
    // Keep order same as Transition and EmptyClip for 'DumpSequence' method
    os << &obj << '|' << obj.mTrack.lock() << '|' << obj.mLink.lock() << '|' << std::setw(6) << obj.mIndex << '|' << std::setw(6) << obj.mLeftPtsInTrack << '|' << std::setw(6) << obj.mOffset << '|' << std::setw(6) << obj.mLength << '|' << obj.mSelected;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Clip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IClip>(*this);
    ar & mRender;
    ar & mOffset;
    ar & mLength;
    ar & mLink;
    ar & mTrack;
    ar & mLeftPtsInTrack;
    ar & mIndex;
    // NOT: mSelected. After loading, nothing is selected.
}
template void Clip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace