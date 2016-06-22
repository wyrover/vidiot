// Copyright 2015-2016 Eric Raijmakers.
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

#pragma once

#include "KeyFrame.h"
#include "Enums.h"

namespace model {

typedef std::map<pts, VideoKeyFramePtr> VideoKeyFrameMap;

class VideoKeyFrame
    : public KeyFrame
{
public:

    static const rational64 sScalingMin;
    static const rational64 sScalingMax;
    static const int sOpacityMin;
    static const int sOpacityMax;
    // todo check that ever size 0 remains!

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoKeyFrame();

    VideoKeyFrame(const wxSize& size);

    VideoKeyFrame(VideoKeyFramePtr before, VideoKeyFramePtr after, pts positionBefore, pts position, pts positionAfter);

    virtual VideoKeyFrame* clone() const override;

    virtual ~VideoKeyFrame();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize getInputSize() const { return mInputSize; }
    wxSize getOutputSize() const;
    wxRect getCroppedRect() const;

    int getOpacity() const;
    VideoScaling getScaling() const;
    rational64 getScalingFactor() const;
    rational64 getRotation() const;
    wxPoint getRotationPositionOffset() const;
    VideoAlignment getAlignment() const;
    wxPoint getPosition() const; ///< \return the logical position as observed by the user. That is the combination of the alignment offset and the shift because of the region of interest.
    int getCropTop();
    int getCropTopMax();
    int getCropBottom();
    int getCropBottomMax();
    int getCropLeft();
    int getCropLeftMax();
    int getCropRight();
    int getCropRightMax();

    wxPoint getMinPosition();
    wxPoint getMaxPosition();

    void setOpacity(int opacity);
    void setCropTop(int crop);
    void setCropBottom(int crop);
    void setCropLeft(int crop);
    void setCropRight(int crop);
    void setScalingFactor(const rational64 & factor);
    void setScaling(const VideoScaling& scaling, const boost::optional< rational64 >& factor = boost::optional< rational64 >());
    void setRotation(const rational64& rotation);
    void setRotationPositionOffset(wxPoint position);
    void setAlignment(const VideoAlignment& alignment);
    void setPosition(const wxPoint& position); ///< \param position the logical position as observed by the user. That is the combination of the alignment offset and the shift because of the region of interest.

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoKeyFrame(const VideoKeyFrame& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxSize mInputSize = wxSize{ 0,0 };

    int mOpacity = sOpacityMax;

    VideoScaling mScaling = model::VideoScalingNone;

    /// Uses Constants::sScalingPrecisionFactor as denominator.
    /// Avoid rounding errors with doubles
    /// (leads to small diffs which cause test asserts to fail).
    rational64 mScalingFactor = 1;

    /// Uses Constants::sRotationPrecisionFactor as denominator.
    /// Avoid rounding errors with doubles
    /// (leads to small diffs which cause test asserts to fail).
    rational64 mRotation = 0;

    /// Offset added to the position to avoid the image being
    /// moved when rotating. Furthermore, guarantees that automated
    /// positioning also works correctly for rotated images.
    wxPoint mRotationPositionOffset = wxPoint{ 0,0 };

    VideoAlignment mAlignment = VideoAlignmentCenter;
    wxPoint mPosition = wxPoint{ 0,0 };
    int mCropTop = 0;
    int mCropBottom = 0;
    int mCropLeft = 0;
    int mCropRight = 0;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Determine the bounding box required for holding the clip.
    /// If mRotation == 0 then this equals the video size.
    /// If mRotation != 0 then this is larger than the video size.
    wxSize getBoundingBox();

    void updateAutomatedScaling();
    void updateAutomatedPositioning();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const VideoKeyFrame& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::VideoKeyFrame, 2)
BOOST_CLASS_EXPORT_KEY(model::VideoKeyFrame)
