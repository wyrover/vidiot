#ifndef MODEL_TRACK_H
#define MODEL_TRACK_H

#include <wx/event.h>
#include <list>
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include "IControl.h"
#include "UtilLog.h"

namespace model {

class Track;
typedef boost::shared_ptr<Track> TrackPtr;
typedef boost::shared_ptr<const Track> ConstTrackPtr;
typedef boost::weak_ptr<Track> WeakTrackPtr;
typedef std::list<TrackPtr> Tracks;
class IClip;
typedef boost::shared_ptr<const IClip> ConstIClipPtr;
typedef boost::shared_ptr<IClip> IClipPtr;
typedef std::list<IClipPtr> IClips;

class Track
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public IControl
    ,   public boost::enable_shared_from_this<Track>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Track();

    virtual Track* clone() override;

    virtual ~Track();

    //////////////////////////////////////////////////////////////////////////
    // ICONTROL
    //////////////////////////////////////////////////////////////////////////

    virtual pts getLength() const override;
    virtual void moveTo(pts position) override;
    virtual wxString getDescription() const override;
    virtual void clean() override;

    //////////////////////////////////////////////////////////////////////////
    // CLIPS
    //////////////////////////////////////////////////////////////////////////

    virtual void removeClips(IClips clips);
    virtual void addClips(IClips clips, IClipPtr position = IClipPtr());

    const IClips& getClips();

    /// Find the clip which provides the frame at the given pts.
    /// If pts is 'on a cut' then the clip AFTER the cut is returned. 
    /// If there is no clip at this pts then an empty Ptr is returned.
    IClipPtr getClip(pts position);

    /// \return the index'th clip in the list of clips
    IClipPtr getClipByIndex(int index);

    /// /return size of area to the left of clip that is empty
    pts getLeftEmptyArea(IClipPtr clip);

    /// /return size of area to the right of clip that is empty
    pts getRightEmptyArea(IClipPtr clip); 

    //////////////////////////////////////////////////////////////////////////
    // STATIC HELPER METHOD
    //////////////////////////////////////////////////////////////////////////

    static pts getCombinedLength(IClips clips);

    //////////////////////////////////////////////////////////////////////////
    // GET & SET
    //////////////////////////////////////////////////////////////////////////

    int getHeight() const;          ///< @return height of this track in the timeline view
    void setHeight(int height);     ///< @param new height of this track in the timeline view

    int getIndex() const;           ///< @return index of this track in the list of video/audio tracks
    void setIndex(int index);       ///< @param index new index of this track

protected:

    //////////////////////////////////////////////////////////////////////////
    // COPY CONSTRUCTOR
    //////////////////////////////////////////////////////////////////////////

    /// Copy constructor. Use make_cloned for making deep copies of objects.
    /// \see make_cloned
    Track(const Track& other);

    //////////////////////////////////////////////////////////////////////////
    // ITERATION
    //////////////////////////////////////////////////////////////////////////

    bool iterate_atEnd();
    IClipPtr iterate_get();
    void iterate_next();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IClips mClips;

    IClips::const_iterator mItClips;

    int mHeight;    ///< Height of this track when viewed in a timeline
    int mIndex;     ///< Index in the list of video/audio tracks

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Updates the clips after insertion/removal etc.
    /// - Updates the pts'es for all clips in this track
    /// - Updates the clip's track pointer to this track
    void updateClips();

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const Track& obj );

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
BOOST_CLASS_VERSION(model::Track, 1)

#endif // MODEL_TRACK_H
