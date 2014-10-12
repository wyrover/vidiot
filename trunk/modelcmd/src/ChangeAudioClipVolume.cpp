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

#include "ChangeAudioClipVolume.h"

#include "AudioClip.h"
#include "UtilLog.h"
#include "UtilLogBoost.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ChangeAudioClipVolume::ChangeAudioClipVolume(const AudioClipPtr& audioclip)
    :   RootCommand()
    ,   mInitialized(false)
    ,   mAudioClip(audioclip)
    ,   mOldVolume(mAudioClip->getVolume())
    ,   mNewVolume(boost::none)
{
    mCommandName = _("Adjust volume for ") + audioclip->getDescription();
}

ChangeAudioClipVolume::~ChangeAudioClipVolume()
{
}

void ChangeAudioClipVolume::setVolume(int volume)
{
    mNewVolume = boost::optional<int>(volume);
    mAudioClip->setVolume(volume);
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool ChangeAudioClipVolume::Do()
{
    VAR_INFO(*this)(mInitialized);

    if (mInitialized)
    {
        // Only the second time that Do() is called (redo) something actually needs to be done.
        // The first time is handled by (possibly multiple calls to) setScaling etc.
        if (mNewVolume)
        {
            mAudioClip->setVolume(*mNewVolume);
        }
    }
    mInitialized = true;
    return true;
}

bool ChangeAudioClipVolume::Undo()
{
    VAR_INFO(*this);
    if (mNewVolume)
    {
        mAudioClip->setVolume(mOldVolume);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::AudioClipPtr ChangeAudioClipVolume::getAudioClip() const
{
    return mAudioClip;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const ChangeAudioClipVolume& obj)
{
    os  << &obj << '|'
        << typeid(obj).name()    << '|'
        << obj.mAudioClip        << '|'
        << obj.mOldVolume        << '|'
        << obj.mNewVolume;
    return os;
}

} // namespace