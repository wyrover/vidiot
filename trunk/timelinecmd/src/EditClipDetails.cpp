// Copyright 2015 Eric Raijmakers.
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

#include "EditClipDetails.h"

#include "Constants.h"
#include "Convert.h"
#include "IClip.h"
#include "Selection.h"
#include "Sequence.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilClone.h"

namespace gui { namespace timeline { namespace command {

EditClipDetails::EditClipDetails(
    const model::SequencePtr& sequence,
        const wxString& message,
    const model::IClipPtr& clip,
    const model::IClipPtr& link,
    const model::IClipPtr& clipClone,
    const model::IClipPtr& linkClone)
    : AClipEdit(sequence)
    , mMessage(message)
    , mClip(clip)
    , mClipClone(clipClone)
    , mLink(link)
    , mLinkClone(linkClone)
{
    VAR_INFO(this)(mClip)(mLink);
    mCommandName = message + mClip->getDescription();
}

EditClipDetails::~EditClipDetails()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void EditClipDetails::initialize()
{
    replaceClip(mClip, { mClipClone });
    if (mLink != nullptr)
    {
        replaceClip(mLink, { mLinkClone }); 
    }
}

void EditClipDetails::doExtraAfter()
{
    // Ensure that this clip is selected again after Undo/Redo.
    // Otherwise, triggering Undo/Redo may cause more than one clip being
    // selected, which looks rather odd.
    getTimeline().getSelection().change({ mClipClone, mLinkClone });
}

void EditClipDetails::undoExtraAfter()
{
    // Ensure that this clip is selected again after Undo/Redo.
    // Otherwise, triggering Undo/Redo may cause more than one clip being
    // selected, which looks rather odd.
    getTimeline().getSelection().change({ mClip, mLink});
}

//void EditClipDetails::changeVideoClipSpeed(const boost::rational<int>& speed)
//{
//    ASSERT(isInitialized());
//    model::VideoClipPtr clip = getVideoClip();
//    pts originalLength = clip->getLength();
//    boost::rational<int> originalSpeed = clip->getSpeed();
//
//    // recalc original offset
//    pts originalOffset = clip->getOffset();
//    pts newOffset = model::Convert::positionToNewSpeed(clip->getOffset(), speed, clip->getSpeed());
//
//    mClipClone->adjustBegin(newOffset - originalLength); //todo is not always video
//}
//
//todo move
//void EditClipDetails::changeAudioClipSpeed(const boost::rational<int>& speed)
//{
//    // todo disable changing speed for very short clips?
//
//    ASSERT(isInitialized());
//    model::AudioClipPtr clip = getAudioClip();
//    pts originalOffset = clip->getOffset();
//    pts originalLength = clip->getLength();
//    boost::rational<int> originalSpeed = clip->getSpeed();
//
//    // recalc original offset
//    pts newOffset = model::Convert::positionToNewSpeed(clip->getOffset(), speed, clip->getSpeed());
//
//    // recalc original length
//
//    // add empty clip if required (newmove, add to do/undo system)
//    // adjust empty clip length if required
//    // no todo: if the speed causes the clip length to become too large, simply add black frames at end? same at begin if offset becomes too large???
//}

//boost::rational<int> EditClipDetails::getMaxVideoSpeed() const
//{
//    model::VideoClipPtr clip = getVideoClip();
//    if (!clip) 
//    { 
//        return 1; 
//    }
//    if (clip->getOffset() == 0)
//    {
//        return boost::rational<int>(model::Constants::sSpeedMax, model::Constants::sSpeedPrecisionFactor);
//    }
//    model::VideoClipPtr clone = make_cloned<model::VideoClip>(clip);
//    clone->maximize();
//    pts maxoffset = clone->getLength() - 10; // Always room for one frame to display (preview during trim) // todo how small may 10 be to be still safe
//    ASSERT_MORE_THAN_EQUALS_ZERO(maxoffset);
//    boost::rational<int> maxSpeed = boost::rational<int>(maxoffset, clip->getOffset()) * clip->getSpeed();
//    return maxSpeed;
//}
//
//boost::rational<int> EditClipDetails::getMaxAudioSpeed() const
//{
//    model::AudioClipPtr clip = getAudioClip();
//    if (!clip) 
//    { 
//        return 1; 
//    }
//    if (clip->getOffset() == 0)
//    {
//        return boost::rational<int>(model::Constants::sSpeedMax, model::Constants::sSpeedPrecisionFactor);
//    }
//    model::AudioClipPtr clone = make_cloned<model::AudioClip>(clip);
//    clone->maximize();
//    pts maxoffset = clone->getLength() - 10; // Always room for one frame to display (preview during trim) // todo how small may 10 be to be still safe
//    ASSERT_MORE_THAN_EQUALS_ZERO(maxoffset);
//    boost::rational<int> maxSpeed = boost::rational<int>(maxoffset, clip->getOffset()) * clip->getSpeed();
//    return maxSpeed;
//}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxString EditClipDetails::getMessage() const
{
    return mMessage;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const EditClipDetails& obj)
{
    os  << static_cast<const AClipEdit&>(obj) << '|' 
        << obj.mMessage << '|'
        << obj.mClip << '|' 
        << obj.mClipClone << '|'
        << obj.mLink << '|'
        << obj.mLinkClone;
    return os;
}

}}} // namespace