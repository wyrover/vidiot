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

    void setScaling(VideoScaling scaling, boost::optional<double> factor = boost::none);
    VideoScaling getScaling() const;
    double getScalingFactor() const;

    wxSize getSize() const; ///< \return size of output video

    VideoAlignment getAlignment() const;
    wxPoint getAlignmentOffset() const;
    wxRect getRegionOfInterest() const;

    /// Determine the size and region of interest.
    void determineTransform();

    double determineScalingFactor(VideoScaling scaling, boost::optional<double> factor = boost::none);
    wxRect determineRegionOfInterest(wxSize inputsize, wxSize outputsize, VideoAlignment alignment);

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
    double mScalingFactor;

    VideoAlignment mAlignment;
    wxPoint mAlignmentOffset;
    wxRect mRegionOfInterest;

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