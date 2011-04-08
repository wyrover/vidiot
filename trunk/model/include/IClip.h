#ifndef MODEL_I_CLIP_H
#define MODEL_I_CLIP_H

#include <wx/string.h>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/version.hpp>
#include "IControl.h"
#include "UtilInt.h"

namespace model {

// FORWARD DECLARATIONS
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef std::list<TrackPtr> Tracks;
class IClip;
typedef boost::shared_ptr<IClip> IClipPtr;
typedef boost::weak_ptr<IClip> WeakIClipPtr;
typedef std::list<IClipPtr> IClips;

class IClip
    :   public IControl
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual ~IClip() {};

    //////////////////////////////////////////////////////////////////////////
    // ICLONEABLE
    //////////////////////////////////////////////////////////////////////////

    virtual IClip* clone() = 0;

    //////////////////////////////////////////////////////////////////////////
    // TRACK
    //////////////////////////////////////////////////////////////////////////

    /// Set the track which contains this clip. Also sets the leftmost pts
    /// of the clip inside the track. When called without parameters
    /// (thus using the defaults), this information is 'reset'.
    virtual void setTrack(TrackPtr track = TrackPtr(), pts trackPosition = 0, unsigned int index = 0) = 0;

    /// \return the track in which this clip is contained. A null ptr is returned if the clip is not in a track.
    virtual TrackPtr getTrack() = 0;

    /// \return pts (in containing track) of begin point of clip.
    /// The frame at this position is the first frame of this clip.
    /// The frames of a clip are [ getLeftPts,getRightPts )
    virtual pts getLeftPts() const = 0;

    /// \return pts (in containing track) AFTER end point of clip.
    /// The frame at this position is AFTER the last frame of this clip
    /// The frames of a clip are [ getLeftPts,getRightPts )
    virtual pts getRightPts() const = 0; 

    //////////////////////////////////////////////////////////////////////////
    // LINK
    //////////////////////////////////////////////////////////////////////////

    virtual void setLink(IClipPtr link) = 0;
    virtual IClipPtr getLink() const = 0;

    //////////////////////////////////////////////////////////////////////////
    // POSITION
    //////////////////////////////////////////////////////////////////////////

    virtual pts getMinAdjustBegin() const = 0;  ///< \return Minimum allowed value for adjustBegin given the available data.
    virtual pts getMaxAdjustBegin() const = 0;  ///< \return Maximum allowed value for adjustBegin given the available data.

    /// If adjustment is positive then move the begin point of the clip backwards
    /// in time (increase the start pts). If adjustment is negative then move the
    /// begin point of the clip forward in time (decrease the start pts).
    /// \param adjustment pts count to add/subtract from the begin point
    virtual void adjustBegin(pts adjustment) = 0;

    virtual pts getMinAdjustEnd() const = 0;    ///< \return Minimum allowed value for adjustEnd given the available data.
    virtual pts getMaxAdjustEnd() const = 0;    ///< \return Maximum allowed value for adjustEnd given the available data.

    /// Set the new length of the clip.
    /// \param adjustment pts count to add/subtract from the length
    virtual void adjustEnd(pts adjustment) = 0;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    virtual bool getSelected() const = 0;           ///< /return true if this clip is selected
    virtual void setSelected(bool selected) = 0;    ///< Select or deselect clip

    /// \return pts value of most recently returned audio/video in getNext*.
    virtual pts getGenerationProgress() const = 0;          

    /// \param delivered value of most recently returned audio/video in getNext*.
    /// Triggers DebugEventRenderProgress.
    virtual void setGenerationProgress(pts progress) = 0;

protected:

    //////////////////////////////////////////////////////////////////////////
    // CURRENT POSITION HANDLING
    //////////////////////////////////////////////////////////////////////////

    /// This method resets mLastSetPosition. This must be called whenever there
    /// is new playback progress.
    virtual void invalidateLastSetPosition() = 0;

    /// Return the most recent position as specified in moveTo(). This is
    /// uninitialized when there was playback progress after the moveTo.
    /// \see invalidateLastSetPosition
    virtual boost::optional<pts> getLastSetPosition() const = 0;

private:

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & boost::serialization::base_object<IControl>(*this);
    }
};

} // namespace

BOOST_SERIALIZATION_ASSUME_ABSTRACT(model::IClip)
// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(model::IClip, 1)
BOOST_CLASS_EXPORT(model::IClip)
BOOST_CLASS_TRACKING(model::IClip, boost::serialization::track_always)

#endif // MODEL_I_CLIP_H
