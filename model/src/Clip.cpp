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
    :   mTrack()
    ,   mPrev()
    ,   mNext()
    ,   mIndex(0)
    ,   mLeftPtsInTrack(0)
    ,   mLink()
    ,   mLastSetPosition(boost::none)
    ,   mSelected(false)
    ,   mDragged(false)
    ,   mGeneratedPts(0)
    ,   mDescription("")
{
    VAR_DEBUG(*this);
}

Clip::Clip(const Clip& other)
    :   mTrack(model::TrackPtr())   // Clone is not automatically part of same track!!!
    ,   mPrev()                     // Clone is not automatically part of same track!!!
    ,   mNext()                     // Clone is not automatically part of same track!!!
    ,   mIndex(0)                   // Clone is not automatically part of same track!!!
    ,   mLeftPtsInTrack(0)          // Clone is not automatically part of same track!!!
    ,   mLink()                     // Clone is not automatically linked to same clip, since it will typically be used in ClipEdit derived classes, using link mapping for maintaining the links
    ,   mLastSetPosition(boost::none)
    ,   mSelected(other.mSelected)
    ,   mDragged(false)             // Clone is not automatically also dragged!!!
    ,   mGeneratedPts(0)
    ,   mDescription(other.mDescription)
{
    VAR_DEBUG(*this)(other);
}

Clip::~Clip()
{
    VAR_DEBUG(*this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

wxString Clip::getDescription() const
{
    return "";
}

void Clip::clean()
{
    VAR_DEBUG(this);
    mLastSetPosition = boost::none;
    mGeneratedPts = 0;
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

bool Clip::hasTrack() const
{
    return const_cast<Clip*>(this)->getTrack();
}

void Clip::setNext(IClipPtr next)
{
    mNext = next;
}

void Clip::setPrev(IClipPtr prev)
{
    mPrev = prev;
}

IClipPtr Clip::getNext()
{
    return mNext.lock();
}

IClipPtr Clip::getPrev()
{
    return mPrev.lock();
}

ConstIClipPtr Clip::getNext() const
{
    return mNext.lock();
}

ConstIClipPtr Clip::getPrev() const
{
    return mPrev.lock();
}

pts Clip::getLeftPts() const
{
    return mLeftPtsInTrack;
}

pts Clip::getRightPts() const
{
    return mLeftPtsInTrack + getLength();
}

void Clip::setLink(IClipPtr link)
{
    mLink = WeakIClipPtr(link);
}

IClipPtr Clip::getLink() const
{
    return mLink.lock();
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
    if (Config::getShowDebugInfo() && mGeneratedPts != progress) // todo isnt this obsolete, since for rendering a clone is made? or do via thread::ismain?
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

void Clip::setLastSetPosition(pts position)
{
    mLastSetPosition.reset(position);
}

std::set<pts> Clip::getCuts(const std::set<IClipPtr>& exclude) const
{
    std::set<pts> result;
    result.insert(getLeftPts());
    result.insert(getRightPts());
    return result;
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
    // Keep order same as Transition for 'dump' method
    os  << std::setfill(' ') << std::setw(3) << obj.mIndex << '|'
        << obj.getType() << '|'
        << &obj << '|'
        << obj.mTrack.lock() << '|'
        << obj.mLink.lock() << '|'
        << std::setw(6) << obj.mLeftPtsInTrack << '|'
        << obj.mSelected;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Clip::serialize(Archive & ar, const unsigned int version)
{
    boost::serialization::void_cast_register<Clip, IClip>(static_cast<Clip *>(0), static_cast<IClip *>(0));
    ar & mLink;
    ar & mTrack;
    ar & mNext;
    ar & mPrev;
    ar & mLeftPtsInTrack;
    ar & mIndex;
    // NOT: mSelected. After loading, nothing is selected.
}
template void Clip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace