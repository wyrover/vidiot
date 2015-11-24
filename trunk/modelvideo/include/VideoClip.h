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

#pragma once

#include "ClipInterval.h"
#include "Enums.h"
#include "IVideo.h"

namespace model {

class VideoClip
    :   public ClipInterval
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoClip();

    VideoClip(const VideoFilePtr& clip);

    virtual VideoClip* clone() const override;

    virtual ~VideoClip();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    virtual std::ostream& dump(std::ostream& os) const override;
    virtual const char* getType() const override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(const VideoCompositionParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize getInputSize(); ///< \return size of input video

    int getOpacity() const;
    VideoScaling getScaling() const;
    boost::rational<int> getScalingFactor() const;
    boost::rational<int> getRotation() const;
    VideoAlignment getAlignment() const;
    wxPoint getPosition() const; ///< \return the logical position as observed by the user. That is the combination of the alignment offset and the shift because of the region of interest.

    wxPoint getMinPosition();
    wxPoint getMaxPosition();

    void setOpacity(int opacity);
    void setScaling(const VideoScaling& scaling, const boost::optional< boost::rational< int > >& factor = boost::optional< boost::rational< int > >());
    void setRotation(const boost::rational< int >& rotation);
    void setAlignment(const VideoAlignment& alignment);
    void setPosition(const wxPoint& position); ///< \param position the logical position as observed by the user. That is the combination of the alignment offset and the shift because of the region of interest.

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoClip(const VideoClip& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    /// Current render position in pts units (delivered video frames count)
    pts mProgress;

    int mOpacity;

    VideoScaling mScaling;

    /// Uses Constants::sScalingPrecisionFactor as denominator.
    /// Avoid rounding errors with doubles
    /// (leads to small diffs which cause test asserts to fail).
    boost::rational<int> mScalingFactor;

    /// Uses Constants::sRotationPrecisionFactor as denominator.
    /// Avoid rounding errors with doubles
    /// (leads to small diffs which cause test asserts to fail).
    boost::rational<int> mRotation;

    /// Offset added to the position to avoid the image being
    /// moved when rotating. Furthermore, guarantees that automated
    /// positioning also works correctly for rotated images.
    wxPoint mRotationPositionOffset;

    VideoAlignment mAlignment;
    wxPoint mPosition;

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

    friend std::ostream& operator<<(std::ostream& os, const VideoClip& obj);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(model::VideoClip, 3)
BOOST_CLASS_EXPORT_KEY(model::VideoClip)
