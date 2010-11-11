#ifndef VIEW_MAP_H
#define VIEW_MAP_H

#include <map>
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "Part.h"

namespace gui { namespace timeline {

typedef std::map< model::TrackPtr, GuiTimeLineTrack* > TrackMap;
typedef std::map< model::ClipPtr, GuiTimeLineClip* > ClipMap;

class ViewMap
    :   public Part
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

    void registerView(model::ClipPtr clip, GuiTimeLineClip* view);
    void registerView(model::TrackPtr track, GuiTimeLineTrack* view);
    void unregisterView(model::ClipPtr clip);
    void unregisterView(model::TrackPtr track);

    //////////////////////////////////////////////////////////////////////////
    // CONVERSION
    //////////////////////////////////////////////////////////////////////////

    virtual GuiTimeLineClip* getView(model::ClipPtr clip) const;
    virtual GuiTimeLineTrack* getView(model::TrackPtr track) const;

private:

    TrackMap mTracks;
    ClipMap mClips;
};

}} // namespace

#endif // VIEW_MAP_H
