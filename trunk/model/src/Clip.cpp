// Copyright 2013,2014 Eric Raijmakers.
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
    ,   mLeftPtsInTrack(0)
    ,   mIndex(0)
    ,   mLink()
    ,   mNewStartPosition(boost::none)
    ,   mSelected(false)
    ,   mDragged(false)
    ,   mGeneratedPts(0)
    ,   mDescription("")
{
    // NOT: VAR_DEBUG(*this); -- Uses virtual methods that only have an implementation in derived classes (which are not yet initialized)
}

Clip::Clip(const Clip& other)
    :   mTrack(model::TrackPtr())   // Clone is not automatically part of same track!!!
    ,   mPrev()                     // Clone is not automatically part of same track!!!
    ,   mNext()                     // Clone is not automatically part of same track!!!
    ,   mLeftPtsInTrack(0)          // Clone is not automatically part of same track!!!
    ,   mIndex(0)                   // Clone is not automatically part of same track!!!
    ,   mLink()                     // Clone is not automatically linked to same clip, since it will typically be used in ClipEdit derived classes, using link mapping for maintaining the links
    ,   mNewStartPosition(boost::none)
    ,   mSelected(other.mSelected)
    ,   mDragged(false)             // Clone is not automatically also dragged!!!
    ,   mGeneratedPts(0)
    ,   mDescription(other.mDescription)
{
    // NOT: VAR_DEBUG(*this)(other); -- Uses virtual methods that only have an implementation in derived classes (which are not yet initialized)
}

Clip::~Clip()
{
    // NOT: VAR_DEBUG(*this); -- Log in most derived class. Avoids duplicate logging AND avoids pure virtual calls (implemented in most derived class).
}

void Clip::onCloned()
{
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
    mNewStartPosition = boost::none;
    mGeneratedPts = 0;
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

void Clip::setTrackInfo(
    const TrackPtr& track,
    const IClipPtr& prev,
    const IClipPtr& next,
    pts trackPosition,
    const unsigned int& index)
{
    mTrack = track;
    mPrev = prev;
    mNext = next;
    mLeftPtsInTrack = trackPosition;
    mIndex = index;
}

TrackPtr Clip::getTrack()
{
    return mTrack.lock();
}

bool Clip::hasTrack() const
{
    return const_cast<Clip*>(this)->getTrack();
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

void Clip::setLink(const IClipPtr& link)
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
        ASSERT(wxThread::IsMain()); // ProcessEvent in another thread (for instance, the rendering thread) causes threading issues: View classes in timeline may only be updated from the GUI thread.
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
    // Note: the condition "wxThread::IsMain()" was added to avoid
    //       generating these events in case a sequence is being rendered.
    //       If a sequence is rendered, these events generate updates of the
    //       sequence's timeline's view classes. That, in turn, causes all
    //       sorts of threading issues. In general: When a sequence is rendered
    //       no changes to the sequence (or its tracks/clips/etc.) may be
    //       made. That includes the 'render progress' event.
    if (wxThread::IsMain() && mGeneratedPts != progress)
    {
        mGeneratedPts = progress;
        ProcessEvent(DebugEventRenderProgress(mGeneratedPts));
    }
}

void Clip::invalidateNewStartPosition()
{
    mNewStartPosition = boost::none;
}

boost::optional<pts> Clip::getNewStartPosition() const
{
    return mNewStartPosition;
}

void Clip::setNewStartPosition(pts position)
{
    mNewStartPosition.reset(position);
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
    if (transition && transition->getRight())
    {
        return transition;
    }
    return model::TransitionPtr();
}

TransitionPtr Clip::getOutTransition() const
{
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(boost::const_pointer_cast<IClip>(getNext()));
    if (transition && transition->getLeft())
    {
        return transition;
    }
    return model::TransitionPtr();
}

pts Clip::getPerceivedLength() const 
{
    pts left = 0;
    model::TransitionPtr inTransition = getInTransition();
    if (inTransition)
    {
        left = *(inTransition->getRight()); // See getInTransition: check for getRight() not needed
    }
    pts right = 0;
    model::TransitionPtr outTransition = getOutTransition();
    if (outTransition)
    {
        right = *(outTransition->getLeft()); // See getInTransition: check for getLeft() not needed
    }

    return left + getLength() + right;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Clip& obj)
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
    try
    {
        boost::serialization::void_cast_register<Clip, IClip>(static_cast<Clip *>(0), static_cast<IClip *>(0));
        ar & BOOST_SERIALIZATION_NVP(mLink);

        if (version == 1)
        {
            ar & BOOST_SERIALIZATION_NVP(mTrack);
            ar & BOOST_SERIALIZATION_NVP(mNext);
            ar & BOOST_SERIALIZATION_NVP(mPrev);
            ar & BOOST_SERIALIZATION_NVP(mLeftPtsInTrack);
            ar & BOOST_SERIALIZATION_NVP(mIndex);
        }
        // NOT: mSelected. After loading, nothing is selected.
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Clip::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Clip::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::Clip)