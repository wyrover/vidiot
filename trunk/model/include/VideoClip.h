#ifndef MODEL_VIDEO_CLIP_H
#define MODEL_VIDEO_CLIP_H

#include <boost/optional.hpp>
#include "Clip.h"
#include "Enums.h"
#include "IVideo.h"

namespace model {
class VideoClip;
typedef boost::shared_ptr<VideoClip> VideoClipPtr;

class VideoClip
    :   public Clip
    ,   public IVideo
{
public:

    static const int sScalingOriginalSize;

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoClip();

    VideoClip(IControlPtr clip);

    virtual VideoClip* clone() const override;

    virtual ~VideoClip();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(wxSize size, bool alpha = true) override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize getInputSize(); ///< \return size of input video

    VideoScaling getScaling() const;
    int getScalingDigits() const;
    VideoAlignment getAlignment() const;
    wxPoint getPosition() const; ///< \return the logical position as observed by the user. That is the combination of the alignment offset and the shift because of the region of interest.

    wxPoint getMinPosition();
    wxPoint getMaxPosition();

    void setScaling(VideoScaling scaling, boost::optional<int> factor = boost::none);
    void setAlignment(VideoAlignment alignment);
    void setPosition(wxPoint position); ///< \param position the logical position as observed by the user. That is the combination of the alignment offset and the shift because of the region of interest.

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

    pts mProgress; ///< Current render position in pts units (delivered video frames count)

    VideoScaling mScaling;
    int mScalingDigits;     ///< mScalingDigits / Constants::scalingPrecisionFactor is the actual scaling to be applied. Stored as an int to avoid rounding errors with doubles (leads to small diffs which cause test asserts to fail).

    VideoAlignment mAlignment;
    wxPoint mPosition;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateAutomatedScaling();
    void updateAutomatedPositioning();
    double getScalingFactor() const;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const VideoClip& obj );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

typedef boost::shared_ptr<VideoClip> VideoClipPtr;
} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::VideoClip, 1)

#endif // MODEL_VIDEO_CLIP_H