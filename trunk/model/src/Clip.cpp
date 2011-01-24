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

DEFINE_EVENT(EVENT_SELECT_CLIP, EventSelectClip, bool);
DEFINE_EVENT(DEBUG_EVENT_RENDER_PROGRESS, DebugEventRenderProgress, pts);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Clip::Clip()
    :   wxEvtHandler()
    ,	IControl()
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
    ,	IControl()
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
    mLength = mRender->getNumberOfFrames() - mOffset;
    VAR_DEBUG(*this);
}

Clip::Clip(const Clip& other)
    :   wxEvtHandler()
    ,	IControl()
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
    VAR_DEBUG(mRender.use_count());
    {
        // @todo this abort handling is a workaround
        // fix inheritance structure so that the destructor of file ~file
        // is called when all shared_ptr's go out of scope..
        FilePtr f = boost::dynamic_pointer_cast<File>(mRender);
        if (f)
        {
            f->abort();
        }
    }
    mRender.reset();
    VAR_DEBUG(mRender.use_count());
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts Clip::getNumberOfFrames()
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

//////////////////////////////////////////////////////////////////////////
// TRACK
//////////////////////////////////////////////////////////////////////////

void Clip::setTrack(TrackPtr track, pts trackPosition, unsigned int index)
{
    mIndex = index;
    mTrack = track;
    mLeftPtsInTrack = trackPosition;
}

TrackPtr Clip::getTrack()
{
    return mTrack;
}

pts Clip::getLeftPts() const
{
    return mLeftPtsInTrack;
}

pts Clip::getRightPts() const
{
    return mLeftPtsInTrack + mLength;
}

//////////////////////////////////////////////////////////////////////////
// LINK
//////////////////////////////////////////////////////////////////////////

void Clip::setLink(ClipPtr link)
{
    mLink = link;
}

ClipPtr Clip::getLink() const
{
    return mLink;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Clip::adjustBegin(pts adjustment)
{
    mOffset += adjustment;
    mLength -= adjustment;
    ASSERT(mLength <=  mRender->getNumberOfFrames() - mOffset)(mLength);
    VAR_DEBUG(*this)(adjustment);
}

void Clip::adjustEnd(pts length)
{
    VAR_INFO(this)(length);
    mLength = length;
    ASSERT(mLength <=  mRender->getNumberOfFrames() - mOffset)(mLength);
    VAR_DEBUG(*this)(length);
}

pts Clip::getOffset() const
{
    return mOffset;
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

//////////////////////////////////////////////////////////////////////////
// STATIC HELPER METHOD
//////////////////////////////////////////////////////////////////////////

//static 
pts Clip::getCombinedLength(model::Clips clips)
{
    int length = 0;
    BOOST_FOREACH( model::ClipPtr clip, clips )
    {
        length += clip->getNumberOfFrames();
    }
    return length;
}

//////////////////////////////////////////////////////////////////////////
// CURRENT POSITION HANDLING
//////////////////////////////////////////////////////////////////////////

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
    ar & boost::serialization::base_object<IControl>(*this);
    ar & mRender;
    ar & mOffset;
    ar & mLength;
    ar & mTrack;
    ar & mLeftPtsInTrack;
    ar & mLink;
    // NOT: mSelected. After loading, nothing is selected.
}
template void Clip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
