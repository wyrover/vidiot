#ifndef MODEL_CLIP_H
#define MODEL_CLIP_H

#include "IClip.h"

namespace model {

class Clip
    :   public IClip
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Clip();                   ///< Constructor for recovery from disk
    virtual ~Clip();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual wxString getDescription() const override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // ICLIP
    //////////////////////////////////////////////////////////////////////////

    void setTrack(TrackPtr track = TrackPtr(), pts trackPosition = 0, unsigned int index = 0) override;
    TrackPtr getTrack() override;
    bool hasTrack() const override;

    void setNext(IClipPtr next) override; // todo make part of 'settrack (or set'cache)
    void setPrev(IClipPtr prev) override;
    IClipPtr getNext() override;
    IClipPtr getPrev() override;
    ConstIClipPtr getNext() const override;
    ConstIClipPtr getPrev() const override;

    pts getLeftPts() const override;
    pts getRightPts() const override;

    virtual void setLink(IClipPtr link) override;
    IClipPtr getLink() const override;

    TransitionPtr getInTransition() const override;
    TransitionPtr getOutTransition() const override;

    bool getSelected() const override;
    void setSelected(bool selected) override;

    bool getDragged() const override;
    void setDragged(bool dragged) override;

    pts getGenerationProgress() const override;
    void setGenerationProgress(pts progress) override;

    void invalidateLastSetPosition() override;
    boost::optional<pts> getLastSetPosition() const override;
    void setLastSetPosition(pts position); // todo rename all these to NewStartPosition

    virtual std::set<pts> getCuts(const std::set<IClipPtr>& exclude = std::set<IClipPtr>()) const override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // ACCESS RENDERING OBJECT
    //////////////////////////////////////////////////////////////////////////

    /// Method needed to convert the IControl instance mRender to an IVideo
    /// or an IAudio instance.
    template <class GENERATOR>
    boost::shared_ptr<GENERATOR> getDataGenerator()
    {
        return boost::static_pointer_cast<GENERATOR>(mRender);
    }

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \note the clone is not automatically part of the track!!!
    /// \see make_cloned
    Clip(const Clip& other);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    WeakTrackPtr mTrack;    ///< Track which holds this clip. Stored as weak_ptr to avoid cyclic dependencies (leading to memory leaks).
    WeakIClipPtr mNext;
    WeakIClipPtr mPrev;

    unsigned int mIndex;    ///< Index of this clip in the track (for debugging)
    WeakIClipPtr mLink;     ///< Clip that this clip is linked with. Stored as weak_ptr to avoid circular dependency between two linked clips which causes memory leaks.

    pts mLeftPtsInTrack;    ///< Position inside the track. 0 if not in a track.

    boost::optional<pts> mLastSetPosition;  ///< The most recent position as specified in 'moveTo()'.
    pts mGeneratedPts;                      ///< (approximate) pts value of last video/audio returned with getNext*

    bool mSelected;                         ///< True if this clip is currently selected
    bool mDragged;                          ///< True if this clip is currently dragged

    mutable wxString mDescription;  ///< Stored for performance (cached) and for easier debugging.

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Clip& obj );

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
BOOST_CLASS_VERSION(model::Clip, 1)

#endif // MODEL_CLIP_H