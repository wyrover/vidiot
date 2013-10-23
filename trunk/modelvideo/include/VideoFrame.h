// Copyright 2013 Eric Raijmakers.
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
#include "UtilFrameRate.h"
#include "UtilInt.h"
#include "UtilRTTI.h"

struct AVFrame;

namespace model {

typedef boost::uint8_t** DataPointer;
typedef int* LineSizePointer;
typedef Fifo<VideoFramePtr> FifoVideo;
std::ostream& operator<< (std::ostream& os, const VideoFramePtr obj);

class VideoFrame
    :   public IRTTI
{
public:

    static const pixel sMinimumSize;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Initialization AND (optional) allocation.
    /// \param allocate if true, then allocate the buffer for decoding with avcodec
    /// Initialization without allocation is used for empty frames. Then, allocation
    /// is only needed when the data is needed for playback. During 'track combining'
    /// empty frames can be ignored. This avoids needless allocation.
    explicit VideoFrame(wxSize size, bool allocate = false);

    /// Initialization of a frame based on a generated wxImage (for instance, for
    /// compositing).
    explicit VideoFrame(wxImagePtr image);

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

    wxSize getSize() const;
    void setPosition(wxPoint position);
    wxPoint getPosition() const;
    void setRegionOfInterest(wxRect regionOfInterest);
    wxRect getRegionOfInterest() const;
    int getOpacity() const;
    void setOpacity(int opacity);
    void setForceKeyFrame(bool force);
    bool getForceKeyFrame() const;

    /// Return an image, using the frame's data clipped to the region of interest
    /// \note This method may return a 0 ptr if the region of interest is empty (basically, if a clip has been moved beyond the visible area)
    /// \pre !mBitmap
    /// \return this frame as a wxImage
    wxImagePtr getImage();

    /// Return a bitmap, using the frame's data clipped to the region of interest
    /// \note This method may return a 0 ptr if the region of interest is empty (basically, if a clip has been moved beyond the visible area)
    /// \return this frame as a wxBitmap
    wxBitmapPtr getBitmap();

    //////////////////////////////////////////////////////////////////////////
    // DATA ACCESS
    //////////////////////////////////////////////////////////////////////////

    /// Virtual and not const due to override in EmptyFrame
    virtual DataPointer getData();

    LineSizePointer getLineSizes() const;

protected:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    AVFrame* mFrame;
    wxImagePtr mImage;
    double mTimeStamp;
    wxSize mSize;
    wxPoint mPosition;
    wxRect mRegionOfInterest;
    pts mPts;
    int mOpacity;
    boost::uint8_t *mBuffer;
    int mBufferSize;
    bool mForceKeyFrame;

private:

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const VideoFrame& obj);
};

} // namespace

#endif // VIDEO_FRAME_H