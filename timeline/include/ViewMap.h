#ifndef VIEW_MAP_H
#define VIEW_MAP_H

#include <map>
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "Part.h"

namespace gui { namespace timeline {

typedef std::map< model::TrackPtr, TrackView* > TrackMap;
typedef std::map< model::ClipPtr, ClipView* > ClipMap;

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

    void registerView(model::ClipPtr clip, ClipView* view);
    void registerView(model::TrackPtr track, TrackView* view);
    void unregisterView(model::ClipPtr clip);
    void unregisterView(model::TrackPtr track);

    //////////////////////////////////////////////////////////////////////////
    // CONVERSION
    //////////////////////////////////////////////////////////////////////////

    virtual ClipView* getView(model::ClipPtr clip) const;
    virtual TrackView* getView(model::TrackPtr track) const;

private:

    TrackMap mTracks;
    ClipMap mClips;
};

}} // namespace

#endif // VIEW_MAP_H
