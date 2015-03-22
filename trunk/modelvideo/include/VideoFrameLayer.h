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

#ifndef VIDEO_FRAME_LAYER_H
#define VIDEO_FRAME_LAYER_H

#include "UtilFifo.h"
#include "UtilFrameRate.h"
#include "UtilInt.h"
#include "UtilRTTI.h"

namespace model {

class VideoCompositionParameters;

class VideoFrameLayer
    :   public IRTTI
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit VideoFrameLayer(const wxImagePtr& image);

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoFrameLayer(const VideoFrameLayer& other);

    virtual VideoFrameLayer* clone() const;

    virtual void onCloned();

    virtual ~VideoFrameLayer();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void setPosition(const wxPoint& position);
    wxPoint getPosition() const;

    /// Sets the opacity for ALL the pixels to the given value
    /// \param opacity new opacity for all the pixels
    /// \pre mImage
    /// \pre opacity >= Constants::sOpacityMax (0)
    /// \pre opacity < Constants::sOpacityMax (255)
    /// \note any existing alpha data is removed
    void setOpacity(int opacity);
    int getOpacity() const;

    void setRotation(boost::rational<int> rotation);

    /// Return an image, using the frame's data clipped to the region of interest.
    /// \note This method may return a 0 ptr if the region of interest is empty
    ///       (basically, if a clip has been moved beyond the visible area)
    /// \return this frame as a wxImage
    virtual wxImagePtr getImage();

    void draw(wxGraphicsContext* gc, const VideoCompositionParameters& parameters);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxImagePtr mImage;
    boost::optional<wxImagePtr> mResultingImage; ///< Image with the changes (position, etc.) imposed by the layer
    wxPoint mPosition;
    int mOpacity;
    boost::optional< boost::rational<int> > mRotation;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const VideoFrameLayer& obj);
};

} // namespace

#endif
