#ifndef VIEW_MAP_H
#define VIEW_MAP_H

#include <map>
#include "ModelPtr.h"
#include "GuiPtr.h"

namespace gui { namespace timeline {

typedef std::map< model::TrackPtr, GuiTimeLineTrackPtr > TrackMap;
typedef std::map< model::ClipPtr, GuiTimeLineClipPtr > ClipMap;

class ViewMap
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ViewMap();

    ~ViewMap();

    //////////////////////////////////////////////////////////////////////////
    // REGISTRATION
    //////////////////////////////////////////////////////////////////////////

    void add(model::ClipPtr modelClip, GuiTimeLineClipPtr clipView);
    void add(model::TrackPtr modelTrack, GuiTimeLineTrackPtr trackView);
    void remove(model::ClipPtr modelClip);
    void remove(model::TrackPtr modelTrack);

    //////////////////////////////////////////////////////////////////////////
    // CONVERSION
    //////////////////////////////////////////////////////////////////////////

    GuiTimeLineClipPtr ModelToView(model::ClipPtr modelClip) const;
    GuiTimeLineTrackPtr ModelToView(model::TrackPtr modelTrack) const;

private:

    TrackMap mTracks;
    ClipMap mClips;
};

}} // namespace

#endif // VIEW_MAP_H
