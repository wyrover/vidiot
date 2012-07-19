#ifndef SELECTION_H
#define SELECTION_H

#include "Part.h"
#include "PositionInfo.h"

namespace model {
    class IClip;
    typedef boost::shared_ptr<IClip> IClipPtr;
    class EventRemoveClips;
    class Track;
    typedef boost::shared_ptr<Track> TrackPtr;
    typedef std::list<TrackPtr> Tracks;
    struct MoveParameter;
    typedef boost::shared_ptr<MoveParameter> MoveParameterPtr;
    typedef std::list<MoveParameterPtr> MoveParameters; // std::list because moves must be done in a particular order.
}

namespace gui { namespace timeline {

class Selection
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Selection(Timeline* timeline);
    virtual ~Selection();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void updateOnLeftClick(const PointerPositionInfo& info);
    void updateOnRightClick(model::IClipPtr clip);

    /// Deletes all selected clips.
    void deleteClips();

    /// Unselect all clips
    void unselectAll();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mPreviouslyClicked;      ///< Clip which was previously (de)selected.

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void selectClipAndLink(model::IClipPtr clip, bool selected);
    void selectClip(model::IClipPtr clip, bool selected);
    void setPreviouslyClicked(model::IClipPtr clip);
};

}} // namespace

#endif // SELECTION_H