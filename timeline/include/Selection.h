#ifndef SELECTION_H
#define SELECTION_H

#include <set>
#include <list>
#include <wx/event.h>
#include "Part.h"

namespace model {
    class Clip;
    typedef boost::shared_ptr<Clip> ClipPtr;
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
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    /// Required for resetting mPreviouslyClicked in case that clip is deleted.
    void onClipsRemoved( model::EventRemoveClips& event );

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void updateOnLeftClick(model::ClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed);
    void updateOnRightClick(model::ClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed);

    /// Deletes all selected clips.
    void deleteClips();

    std::set<model::ClipPtr> getClips() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////
    
    model::ClipPtr mPreviouslyClicked;      ///< Clip which was previously (de)selected.

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void selectClipAndLink(model::ClipPtr clip, bool selected);
    void selectClip(model::ClipPtr clip, bool selected);
    void setPreviouslyClicked(model::ClipPtr clip);
};

}} // namespace

#endif // SELECTION_H
