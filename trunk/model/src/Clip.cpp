#include "Clip.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include "UtilLog.h"
#include "Track.h"
#include "GuiOptions.h"
#include "File.h"

namespace model {

    // todo implementation should be done somewhere else maybe in Event.h/Event.cpp?
DEFINE_EVENT(EVENT_SELECT_CLIP,             EventSelectClip,            bool);
DEFINE_EVENT(DEBUG_EVENT_RENDER_PROGRESS,   DebugEventRenderProgress,   pts);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Clip::Clip()
    :   wxEvtHandler()
    ,	IClip()
    ,   mRender()
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mTrack()
    ,   mIndex(0)
    ,   mLeftPtsInTrack(0)
    ,   mLink()
    ,   mLastSetPosition(boost::none)
    ,   mSelected(false)
    ,   mGeneratedPts(0)
{ 
    VAR_DEBUG(*this);
}

Clip::Clip(IControlPtr render)
    :   wxEvtHandler()
    ,	IClip()
    ,   mRender(render)
    ,   mOffset(0)
    ,   mLength(-1)
    ,   mTrack()
    ,   mIndex(0)
    ,   mLeftPtsInTrack(0)
    ,   mLink()
    ,   mLastSetPosition(boost::none)
    ,   mSelected(false)
    ,   mGeneratedPts(0)
{ 
    mLength = mRender->getLength() - mOffset;
    VAR_DEBUG(*this);
}

Clip::Clip(const Clip& other)
    :   wxEvtHandler()
    ,	IClip()
    ,   mRender(make_cloned<model::IControl>(other.mRender))
    ,   mOffset(other.mOffset)
    ,   mLength(other.mLength)
    ,   mTrack(model::TrackPtr())   // Clone is not automatically part of same track!!!
    ,   mIndex(0)                   // Clone is not automatically part of same track!!!
    ,   mLeftPtsInTrack(0)          // Clone is not automatically part of same track!!!
    ,   mLink(other.mLink)
    ,   mLastSetPosition(boost::none)
    ,   mSelected(other.mSelected)
    ,   mGeneratedPts(0)
{
    VAR_DEBUG(*this)(other);
}

Clip* Clip::clone()
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

pts Clip::getLength()
{
    return mLength; 
}

void Clip::moveTo(pts position)
{
    VAR_DEBUG(*this)(position);
    mLastSetPosition.reset(position);
    setGenerationProgress(0);
    mRender->moveTo(mOffset + position);
}

wxString Clip::getDescription() const
{
    wxString s = mRender->getDescription();
    if (!gui::GuiOptions::getTimelineStrip().IsSameAs(_T("")))
    {
        s.Replace(gui::GuiOptions::getTimelineStrip(),_T(""),false);
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

pts Clip::getMinAdjustBegin() const
{
    return -mOffset;
}

pts Clip::getMaxAdjustBegin() const
{
    return mLength;
}

void Clip::adjustBegin(pts adjustment)
{
    ASSERT(adjustment >= getMinAdjustBegin() && adjustment <= getMaxAdjustBegin())(adjustment)(getMinAdjustBegin())(getMaxAdjustBegin());
    mOffset += adjustment;
    mLength -= adjustment;
    if (getTrack())
    {
        getTrack()->updateClips(); // \todo this is needed to adjust mLeftPtsIntrack for all clips AFTER this clip. 
        // This is very inefficient....
        // we need an event signaling the changed length. THen, the track can update it's administration.
        // since mLeftPtsInTrack is TRACK administration, not clip administration.
    }
    ASSERT(mLength <=  mRender->getLength() - mOffset)(mLength);
    VAR_DEBUG(*this)(adjustment);
}

pts Clip::getMinAdjustEnd() const
{
    return -mLength;
}

pts Clip::getMaxAdjustEnd() const
{
    return mRender->getLength() - mLength - mOffset;
}

void Clip::adjustEnd(pts adjustment)
{
    ASSERT(adjustment >= getMinAdjustEnd() && adjustment <= getMaxAdjustEnd())(adjustment)(getMinAdjustEnd())(getMaxAdjustEnd());
    mLength += adjustment;
    if (getTrack())
    {
        getTrack()->updateClips(); // \see todo in adjustbegin
    }
    ASSERT(mLength <=  mRender->getLength() - mOffset)(mLength);
    VAR_DEBUG(*this)(adjustment);
}

bool Clip::getSelected() const
{
    return mSelected;
}

void Clip::setSelected(bool selected)
{
    mSelected = selected;
    ProcessEvent(EventSelectClip(selected));
}

pts Clip::getGenerationProgress() const
{
    return mGeneratedPts;
}

void Clip::setGenerationProgress(pts progress)
{
    if (mGeneratedPts != progress)
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
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Clip& obj )
{
    os << &obj << '|' << obj.mIndex << '|' << obj.mOffset << '|' << obj.mLength << '|' << obj.mLeftPtsInTrack << '|' << obj.mSelected;
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
    ar & mLeftPtsInTrack;
    ar & mIndex;

    // Links and tracks are stored as weak_ptr to avoid cyclic dependencies (leading to 
    // excessive memory leaks). Storing/reading is done via shared_ptr. Hence,
    // these conversions are required.
    if (Archive::is_loading::value)
    {
        IClipPtr link;
        ar & link;
        setLink(link);

        TrackPtr track;
        ar & track;
        setTrack(track, mLeftPtsInTrack, mIndex);
    }
    else
    {
        ar & mLink.lock();
        ar & mTrack.lock();
    }
    // NOT: mSelected. After loading, nothing is selected.
}
template void Clip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
