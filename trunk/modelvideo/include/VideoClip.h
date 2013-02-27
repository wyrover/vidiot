#ifndef MODEL_VIDEO_CLIP_H
#define MODEL_VIDEO_CLIP_H

#include "Clip.h"
#include "Enums.h"
#include "IVideo.h"

namespace model {

class VideoClip
    :   public Clip
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    VideoClip();
    VideoClip(VideoFilePtr clip);
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
    virtual char* getType() const override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(const VideoCompositionParameters& parameters) override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize getInputSize(); ///< \return size of input video

    int getOpacity() const;
    void setOpacity(int opacity);

    VideoScaling getScaling() const;
    boost::rational<int> getScalingFactor() const;
    VideoAlignment getAlignment() const;
    wxPoint getPosition() const; ///< \return the logical position as observed by the user. That is the combination of the alignment offset and the shift because of the region of interest.

    wxPoint getMinPosition();
    wxPoint getMaxPosition();

    void setScaling(VideoScaling scaling, boost::optional< boost::rational< int > > factor = boost::none);
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

    int mOpacity;

    VideoScaling mScaling;
    boost::rational<int> mScalingFactor; ///< Constants::scalingPrecisionFactor as denominator. Avoid rounding errors with doubles (leads to small diffs which cause test asserts to fail).

    VideoAlignment mAlignment;
    wxPoint mPosition;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateAutomatedScaling();
    void updateAutomatedPositioning();

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

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
#include  <boost/preprocessor/slot/counter.hpp>
#include BOOST_PP_UPDATE_COUNTER()
#line BOOST_PP_COUNTER
BOOST_CLASS_VERSION(model::VideoClip, 1)

#endif // MODEL_VIDEO_CLIP_H