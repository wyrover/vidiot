#ifndef VIEW_MAP_H
#define VIEW_MAP_H

#include <boost/bimap.hpp>
#include "ModelPtr.h"
#include "GuiPtr.h"

namespace gui { namespace timeline {

typedef boost::bimap< model::TrackPtr, GuiTimeLineTrackPtr > TrackMap;
typedef boost::bimap< model::ClipPtr, GuiTimeLineClipPtr > ClipMap;

class ViewMap
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ViewMap();

    ~ViewMap();

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    void add(model::ClipPtr modelClip, GuiTimeLineClipPtr clipView);
    void add(model::TrackPtr modelTrack, GuiTimeLineTrackPtr trackView);
    void remove(model::ClipPtr modelClip);
    void remove(model::TrackPtr modelTrack);
    void remove(GuiTimeLineClipPtr clipView);
    void remove(GuiTimeLineTrackPtr trackView);

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    GuiTimeLineClipPtr ModelToView(model::ClipPtr modelClip);
    GuiTimeLineTrackPtr ModelToView(model::TrackPtr modelTrack);
    model::ClipPtr ViewToModel(GuiTimeLineClipPtr viewClip);
    model::TrackPtr ViewToModel(GuiTimeLineTrackPtr viewTrack);

private:

    TrackMap mTracks;
    ClipMap mClips;
};

}} // namespace

#endif // VIEW_MAP_H
