#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include "UtilFifo.h"
#include "UtilInt.h"
#include "UtilFrameRate.h"
#include "UtilRTTI.h"

struct AVFrame;

namespace model {

typedef boost::uint8_t** DataPointer;
typedef int* LineSizePointer;
typedef Fifo<VideoFramePtr> FifoVideo;
std::ostream& operator<< (std::ostream& os, const VideoFramePtr obj);

class VideoFrame
    :   public boost::noncopyable
    ,   public IRTTI
{
public:

    static const pixel sMinimumSize;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Initialization AND allocation.
    VideoFrame(wxSize size, pts position, int repeat);

    /// Initialization of a frame based on a generated wxImage (for instance, for
    /// compositing).
    VideoFrame(wxImagePtr image, pts position);

    /// Initialization of a frame based on a generated wxBitmap (for instance, for
    /// compositing).
    VideoFrame(wxBitmapPtr image, pts position);

    /// Initialization without allocation. Used for empty frames. Then, allocation only
    /// needed when the data is needed for playback. During 'track combining' empty
    /// frames can be ignored. This avoids needless allocation.
    VideoFrame(wxSize size, pts position);

    virtual ~VideoFrame();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int getRepeat() const;      ///< \return the number of times this frame should be displayed
    void setRepeat(int repeat); ///< \param new number of times to show this frame (used at end of clips)
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
    wxBitmapPtr mBitmap;
    wxImagePtr mImage;
    int mRepeat;
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