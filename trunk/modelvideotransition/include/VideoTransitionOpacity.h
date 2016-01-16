// Copyright 2013-2016 Eric Raijmakers.
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

#include "VideoTransition.h"
#include "IVideo.h"

namespace model {

/// Default base class for transitions that merely select pixels of the left
/// or right image by changing the clip's opacity values.
class VideoTransitionOpacity
    :   public VideoTransition
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoTransitionOpacity() = default;

    virtual ~VideoTransitionOpacity() = default;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoTransitionOpacity(const VideoTransitionOpacity& other) = default;

    //////////////////////////////////////////////////////////////////////////
    // IMPLEMENTATION OF TRANSITION
    //////////////////////////////////////////////////////////////////////////

    /// By default, the left image is layered 'on top of' the right image.
    /// By overriding this method, this order can be changed.
    virtual bool getLeftOnTop() { return true; }

    VideoFramePtr getVideo(pts position, const IClipPtr& leftClip, const IClipPtr& rightClip, const VideoCompositionParameters& parameters) override;

    /// Apply the given method to all pixels.
    /// \param image on the opacity of this image the given method is applied
    /// \param f given x and y, the opacity of the image is multiplied by the resulting value of this method
    void applyToAllPixels(const wxImagePtr& image, const std::function<float (int, int)>& f) const;

    /// Apply the given method to all pixels in the first line. Then, the first line
    /// is copied onto all subsequent lines. This is faster than applying the method
    /// to the opacity of all pixels.
    /// \param image on the opacity of this image the given method is applied
    /// \param f given x and y, the opacity of the image is multiplied by the resulting value of this method
    void applyToFirstLineThenCopy(const wxImagePtr& image, const std::function<float (int, int)>& f) const;

    /// To be implemented by derived classes.
    /// When the image is fully opaque a more efficient implementation than iterating
    /// over all pixels may be possible.
    /// \param image on the opacity of this image the given method is applied
    virtual void handleFullyOpaqueImage(const wxImagePtr& image, const std::function<float (int, int)>& f) const;

    /// To be implemented by derived classes.
    /// The image contains alpha data. Therefore, an optimized implementation might
    /// not be possible.
    /// \param image on the opacity of this image the given method is applied
    virtual void handleImageWithAlpha(const wxImagePtr& image, const std::function<float (int, int)>& f) const;

    /// To be implemented by derived classes.
    /// Resulting factor for pixels of the left image.
    /// Input factor of 0 indicates left image fully visible.
    /// Input factor of 1 indicates right image fully visible.
    /// \param image on the opacity of this image the given method is applied
    /// \param factor factor that indicates the progress of the transition 0.0 <= factor <= 1.0
    virtual std::function<float (int,int)> getLeftMethod(const wxImagePtr& image, const float& factor) const;

    /// To be implemented by derived classes.
    /// Resulting factor for pixels of the right image.
    /// Input factor of 0 indicates left image fully visible.
    /// Input factor of 1 indicates right image fully visible.
    /// \param image on the opacity of this image the given method is applied
    /// \param factor factor that indicates the progress of the transition 0.0 <= factor <= 1.0
    virtual std::function<float (int,int)> getRightMethod(const wxImagePtr& image, const float& factor) const = 0;
};

} // namespace
