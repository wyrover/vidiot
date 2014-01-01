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

#include "ChangeVideoClipTransform.h"

#include "Enums.h"
#include "UtilLog.h"
#include "VideoClip.h"
#include "UtilLogWxwidgets.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ChangeVideoClipTransform::ChangeVideoClipTransform(model::VideoClipPtr videoclip)
    :   RootCommand()
    ,   mInitialized(false)
    ,   mVideoClip(videoclip)
    ,   mOldOpacity(mVideoClip->getOpacity())
    ,   mOldScaling(mVideoClip->getScaling())
    ,   mOldScalingFactor(mVideoClip->getScalingFactor())
    ,   mOldRotation(mVideoClip->getRotation())
    ,   mOldAlignment(mVideoClip->getAlignment())
    ,   mOldPosition(mVideoClip->getPosition())
    ,   mNewOpacity(boost::none)
    ,   mNewScaling(boost::none)
    ,   mNewScalingFactor(boost::none)
    ,   mNewRotation(boost::none)
    ,   mNewPosition(boost::none)
{
    mCommandName = _("Adjust properties for ") + videoclip->getDescription();
}

ChangeVideoClipTransform::~ChangeVideoClipTransform()
{
}

void ChangeVideoClipTransform::setOpacity(int opacity)
{
    mNewOpacity = boost::optional<int>(opacity);
    mVideoClip->setOpacity(opacity);
}

void ChangeVideoClipTransform::setScaling(VideoScaling scaling, boost::optional< boost::rational< int > > factor)
{
    mNewScaling = boost::optional<VideoScaling>(scaling);
    mNewScalingFactor = factor;
    mVideoClip->setScaling(*mNewScaling, mNewScalingFactor);
}

void ChangeVideoClipTransform::setRotation(boost::rational< int > rotation)
{
    mNewRotation.reset(rotation);
    mVideoClip->setRotation(rotation);
}

void ChangeVideoClipTransform::setAlignment(VideoAlignment alignment)
{
    mNewAlignment = boost::optional<VideoAlignment>(alignment);
    mVideoClip->setAlignment(*mNewAlignment);
}

void ChangeVideoClipTransform::setPosition(wxPoint position)
{
    mNewPosition = boost::optional<wxPoint>(position);
    mVideoClip->setPosition(*mNewPosition);
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool ChangeVideoClipTransform::Do()
{
    VAR_INFO(*this)(mInitialized);

    if (mInitialized)
    {
        // Only the second time that Do() is called (redo) something actually needs to be done.
        // The first time is handled by (possibly multiple calls to) setScaling etc.
        if (mNewOpacity)
        {
            mVideoClip->setOpacity(*mNewOpacity);
        }
        if (mNewScaling)
        {
            mVideoClip->setScaling(*mNewScaling, mNewScalingFactor);
        }
        if (mNewRotation)
        {
            mVideoClip->setRotation(*mNewRotation);
        }
        if (mNewAlignment)
        {
            mVideoClip->setAlignment(*mNewAlignment);
        }
        if (mNewPosition)
        {
            mVideoClip->setPosition(*mNewPosition);
        }
    }
    mInitialized = true;
    return true;
}

bool ChangeVideoClipTransform::Undo()
{
    VAR_INFO(*this);
    if (mNewOpacity)
    {
        mVideoClip->setOpacity(mOldOpacity);
    }
    if (mNewScaling)
    {
        mVideoClip->setScaling(mOldScaling, boost::optional< boost::rational< int > >(mOldScalingFactor));
    }
    if (mNewRotation)
    {
        mVideoClip->setRotation(mOldRotation);
    }
    if (mNewAlignment)
    {
        mVideoClip->setAlignment(mOldAlignment);
    }
    if (mNewPosition)
    {
        mVideoClip->setPosition(mOldPosition);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::VideoClipPtr ChangeVideoClipTransform::getVideoClip() const
{
    return mVideoClip;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const ChangeVideoClipTransform& obj )
{
    os  << &obj << '|'
        << typeid(obj).name()    << '|'
        << obj.mVideoClip        << '|'
        << obj.mOldOpacity       << '|'
        << obj.mOldScaling       << '|'
        << obj.mOldScalingFactor << '|'
        << obj.mOldAlignment     << '|'
        << obj.mOldPosition      << '|'
        << obj.mNewOpacity       << '|'
        << obj.mNewScaling       << '|'
        << obj.mNewScalingFactor << '|'
        << obj.mNewPosition;
    return os;
}

} // namespace