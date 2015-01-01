// Copyright 2013-2015 Eric Raijmakers.
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

#ifndef CHANGE_VIDEO_CLIP_TRANSFORM_H
#define CHANGE_VIDEO_CLIP_TRANSFORM_H

#include "RootCommand.h"
#include "Enums.h"

namespace model {

class ChangeVideoClipTransform
    :   public ::command::RootCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit ChangeVideoClipTransform(const VideoClipPtr& videoclip);
    virtual ~ChangeVideoClipTransform();

    void setOpacity(int opacity);
    void setScaling(const VideoScaling& scaling, const boost::optional< boost::rational< int > >& factor = boost::none);
    void setRotation(const boost::rational< int >& rotation);
    void setAlignment(const VideoAlignment& alignment);
    void setPosition(const wxPoint& position);

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    model::VideoClipPtr getVideoClip() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mInitialized;

    model::VideoClipPtr mVideoClip;

    int mOldOpacity;

    VideoScaling mOldScaling;
    boost::rational< int > mOldScalingFactor;

    boost::rational< int > mOldRotation;

    VideoAlignment mOldAlignment;
    wxPoint mOldPosition;

    boost::optional<int> mNewOpacity;
    boost::optional<VideoScaling> mNewScaling;
    boost::optional< boost::rational< int > > mNewScalingFactor;
    boost::optional< boost::rational< int > > mNewRotation;
    boost::optional<VideoAlignment> mNewAlignment;
    boost::optional<wxPoint> mNewPosition;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const ChangeVideoClipTransform& obj);
};

} // namespace

#endif
