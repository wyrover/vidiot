#ifndef MODEL_EMPTY_CLIP_H
#define MODEL_EMPTY_CLIP_H

#include "Clip.h"
#include "IAudio.h"
#include "IVideo.h"

namespace model {

class EmptyClip
    :   public Clip
    ,   public IAudio
    ,   public IVideo
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    EmptyClip();

    /// Create a new empty clip. The two 'extra*' parameters are used to make
    /// the contained EmptyFile larger at the beginning/end than the clip itselves.
    /// This is used for EmptyClips that replace clips which are adjacent to
    /// a transition in a track. The extra area used in the transition is also
    /// replaced with extra area in the empty clip. As a result, when replacing
    /// clips with empty clips, the adjacent transitions do not have to be
    /// changed.
    /// \param length size of the clip
    /// \param extraBegin extra size of the contained EmptyFile at the beginning (the truncated part at the beginning)
    /// \param extraEnd extra size of the contained EmptyFile at the end (the truncated part at the end)
    EmptyClip(pts length, pts extraBegin = 0, pts extraEnd = 0);

    virtual EmptyClip* clone() const override;

    virtual ~EmptyClip();

    /// Make an empty clip that has the same values as the original clip.
    /// When a clip is replaced with 'emptyness' the values for *adjustBegin
    /// and *adjustEnd of the emptyness should equal the original clip. That
    /// is required in case transitions are adjacent.
    ///
    /// This method ensures that, when replacing clips, the resulting empty space
    /// has (in its Render object) enough space to accommodate any adjacent transitions.
    /// \return empty clip that is a 'replica' of original , with the same offset/length etc, but only for non-transitions. For transitions empty space with the same length as original is returned.
    /// \param original clip to be cloned
    static EmptyClipPtr replace( IClipPtr original );

    /// Make an empty clip that has the same length as the original list of clips.
    /// \post resulting clip getMaxAdjustBegin() equals clips.front()->getMaxAdjustBegin()
    /// \post resulting clip getMinAdjustEnd() equals clips.back()->getMinAdjustEnd()
    /// \pre All clips are part of the same track
    static EmptyClipPtr replace(model::IClips clips);

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    virtual void setLink(IClipPtr link) override;
    virtual std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>()) const override;

    //////////////////////////////////////////////////////////////////////////
    // IAUDIO
    //////////////////////////////////////////////////////////////////////////

    virtual AudioChunkPtr getNextAudio(int audioRate, int nAudioChannels) override;

    //////////////////////////////////////////////////////////////////////////
    // IVIDEO
    //////////////////////////////////////////////////////////////////////////

    virtual VideoFramePtr getNextVideo(const VideoCompositionParameters& parameters) override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    explicit EmptyClip(const EmptyClip& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::EmptyClip, 1)

#endif // MODEL_EMPTY_CLIP_H