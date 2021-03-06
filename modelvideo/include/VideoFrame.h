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

#include "UtilFifo.h"
#include "UtilRTTI.h"

namespace model {

class VideoCompositionParameters;

typedef Fifo<VideoFramePtr> FifoVideo;

/// Division of functionality between VideoFrame and VideoFrameLayer:
/// VideoFrame holds
/// - time related attributes
/// - layers
/// - attributes that apply to all layers (parameters)
/// VideoFrameLayer holds
/// - data for each layer
class VideoFrame
    :   public IRTTI
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Initialization without allocation is used for empty frames. Then, allocation
    /// is only needed when the data is needed for playback. During 'track combining'
    /// empty frames can be ignored. This avoids needless allocation.
    explicit VideoFrame(const VideoCompositionParameters& parameters);

    /// Initialization of a frame based on a generated wxImage (for instance, for
    /// compositing).
    /// \note ownership of the layer (and its pixel data) is taken over by the frame.
    explicit VideoFrame(const VideoCompositionParameters& parameters, const VideoFrameLayerPtr& layer);

    /// Initialization of a frame based on multiple layers (for instance, for
    /// compositing in case of a transition).
    /// \note ownership of the layers (and their pixel data) is taken over by the frame.
    explicit VideoFrame(const VideoCompositionParameters& parameters, const VideoFrameLayers& layers);

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    VideoFrame(const VideoFrame& other);

    virtual VideoFrame* clone() const;

    virtual void onCloned();

    virtual ~VideoFrame();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    pts getPts() const;
    void setPts(pts position);

    rational64 getTime() const;
    void setTime(rational64 time);

    void setForceKeyFrame();
    bool getForceKeyFrame() const;

    void setError();
    bool getError() const;

    VideoCompositionParameters getParameters() const;

    virtual VideoFrameLayers getLayers();

    /// Add a new layer (at the bottom)
    void addLayer(const VideoFrameLayerPtr& layer);

    virtual wxImagePtr getImage() ;

    /// Return a bitmap, using the frame's data clipped to the region of interest
    /// and all layers flattened.
    /// The resulting bitmap is cached and returned upon any subsequent call to getBitmap().
    /// \note This method may return a 0 ptr if the region of interest is empty (basically, if a clip has been moved beyond the visible area)
    /// \return this frame as a wxBitmap
    wxBitmapPtr getBitmap();

    void draw(wxGraphicsContext* gc) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    VideoFrameLayers mLayers;
    boost::scoped_ptr<VideoCompositionParameters> mParameters;
    boost::optional<pts> mPts = boost::none;
    rational64 mTime = 0;
    boost::optional<wxBitmapPtr> mCachedBitmap = boost::none;
    bool mForceKeyFrame = false;
    bool mError = false; ///< True if this is an error frame.

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const VideoFrame& obj);
};

} // namespace
