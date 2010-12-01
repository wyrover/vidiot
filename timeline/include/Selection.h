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
    Selection(Timeline* timeline);

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onClipsRemoved( model::EventRemoveClips& event );

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void update(model::ClipPtr clip, bool ctrlPressed, bool shiftPressed, bool altPressed);
    bool isSelected(model::ClipPtr clip) const;
    //void setDrag(bool drag);

    /**
    * Deletes all selected clips.
    **/
    void deleteClips();

    void invalidateTracksWithSelectedClips(); ///< Ensures that all tracks with selected clips are redrawn

private:

    void selectClipAndLink(model::ClipPtr clip, bool selected);
    void selectClip(model::ClipPtr clip, bool selected);

    /**
    * Clip which was previously (de)selected.
    */
    model::ClipPtr mPreviouslyClicked;

    std::set<model::ClipPtr> mSelected;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void deleteFromTrack(model::MoveParameters& moves, model::Tracks tracks);
};

}} // namespace

#endif // SELECTION_H
