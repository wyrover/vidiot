#ifndef VIDEO_FRAME_H
#define VIDEO_FRAME_H

#include <wx/bitmap.h>
#include <wx/image.h>
#include <boost/shared_ptr.hpp>
#include "UtilFifo.h"
#include "UtilInt.h"
#include "UtilFrameRate.h"
#include "UtilRTTI.h"

struct AVFrame;

namespace model {

typedef boost::uint8_t** DataPointer;
typedef int* LineSizePointer;
class VideoFrame;
typedef boost::shared_ptr<VideoFrame> VideoFramePtr;
typedef std::list<VideoFramePtr> VideoFrames;
typedef Fifo<VideoFramePtr> FifoVideo;
std::ostream& operator<< (std::ostream& os, const VideoFramePtr obj);
typedef boost::shared_ptr<wxBitmap> wxBitmapPtr;
typedef boost::shared_ptr<wxImage> wxImagePtr;

class VideoFrame
    :   public boost::noncopyable
    ,   public IRTTI
{
public:

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

private:

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<< (std::ostream& os, const VideoFrame& obj);
};

} // namespace

#endif // VIDEO_FRAME_H