#include "Clip.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/foreach.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "ClipEvent.h"
#include "Config.h"
#include "File.h"
#include "Track.h"
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

Clip::Clip(IControlPtr render)
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
    :	IClip()
    ,   mRender(make_cloned<model::IControl>(other.mRender))
    ,   mOffset(other.mOffset)
    ,   mLength(other.mLength)
    ,   mTrack(model::TrackPtr())   // Clone is not automatically part of same track!!!
    ,   mIndex(0)                   // Clone is not automatically part of same track!!!
    ,   mLeftPtsInTrack(0)          // Clone is not automatically part of same track!!!
    ,   mLink(other.mLink)
    ,   mLastSetPosition(boost::none)
    ,   mSelected(other.mSelected)
    ,   mDragged(false)             // Clone is not automatically also dragged!!!
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
    wxString strip = wxConfigBase::Get()->Read(gui::Config::sPathStrip,"");
    if (!strip.IsSameAs(_T("")))
    {
        s.Replace(strip,_T(""),false);
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
    ASSERT_MORE_THAN_EQUALS(adjustment,getMinAdjustBegin());
    ASSERT_LESS_THAN_EQUALS(adjustment,getMaxAdjustBegin());
    ASSERT(!getTrack())(getTrack()); // Otherwise, this action needs an event indicating the change to the track(view). Instead, tracks are updated by replacing clips.
    mOffset += adjustment;
    mLength -= adjustment;
    ASSERT_LESS_THAN_EQUALS(mLength,mRender->getLength() - mOffset);
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
    ASSERT_MORE_THAN_EQUALS(adjustment,getMinAdjustEnd());
    ASSERT_LESS_THAN_EQUALS(adjustment,getMaxAdjustEnd());
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
    // Keep order same as Clip and EmptyClip for 'DumpSequence' method
    os << &obj << '|' << obj.mTrack.lock() << '|' << std::setw(3) << obj.mIndex << '|' << std::setw(6) << obj.mLeftPtsInTrack << '|' << std::setw(6) << obj.mOffset << '|' << std::setw(6) << obj.mLength << '|' << obj.mSelected;
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

