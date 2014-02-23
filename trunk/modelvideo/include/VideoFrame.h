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

#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include "UtilFifo.h"
#include "UtilInt.h"
#include "UtilRTTI.h"

namespace model {

class VideoCompositionParameters;

typedef Fifo<VideoFramePtr> FifoVideo;
std::ostream& operator<< (std::ostream& os, const VideoFramePtr obj);

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
    explicit VideoFrame(const VideoCompositionParameters& parameters, VideoFrameLayerPtr layer);

    /// Initialization of a frame based on multiple layers (for instance, for
    /// compositing in case of a transition).
    /// \note ownership of the layers (and their pixel data) is taken over by the frame.
    explicit VideoFrame(const VideoCompositionParameters& parameters, VideoFrameLayers layers);

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

    void setForceKeyFrame(bool force);
    bool getForceKeyFrame() const;

    VideoCompositionParameters getParameters() const;

    virtual VideoFrameLayers getLayers();

    /// Add a new layer (at the bottom)
    void addLayer(VideoFrameLayerPtr layer);

    virtual wxImagePtr getImage() ;

    /// Return a bitmap, using the frame's data clipped to the region of interest
    /// and all layers flattened.
    /// The resulting bitmap is cached and returned upon any subsequent call to getBitmap().
    /// \note This method may return a 0 ptr if the region of interest is empty (basically, if a clip has been moved beyond the visible area)
    /// \return this frame as a wxBitmap
    wxBitmapPtr getBitmap();

    void draw(wxGraphicsContext* gc) const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    VideoFrameLayers mLayers;
    boost::scoped_ptr<VideoCompositionParameters> mParameters;
    pts mPts;
    bool mForceKeyFrame;

private:

    boost::optional<wxBitmapPtr> mCachedBitmap;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const VideoFrame& obj);
};

} // namespace

#endif
