#ifndef VIEW_H
#define VIEW_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/region.h>
#include "Part.h"
#include "UtilEvent.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

class View;
class ZoomChangeEvent;

class ViewUpdate
{
public:
    ViewUpdate(View& view, wxRegion area)
        :   mView(&view)
        ,   mArea(area)
    {
    }
    ~ViewUpdate()
    {
    }
    ViewUpdate(const ViewUpdate& other)
        :   mView(other.mView)
        ,   mArea(other.mArea)
    {
    }
    View& getView()
    {
        return *mView;
    }
    wxRegion getArea()
    {
        return mArea;
    }
private:
    View* mView;    // Stored as pointer, not reference for more info during debugging in IDE
    wxRegion mArea;
};

DECLARE_EVENT(VIEW_UPDATE_EVENT, ViewUpdateEvent, ViewUpdate);

class View
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    /// For initializing the topmost (sequence) view
    /// Also, for initializing top views that ignore all events. In the latter 
    /// case, do not call init() and deinit(). Then, that ignoring top view
    /// can be used for creating temporary View trees.
    View(Timeline* timeline);

    /// For initializing the topmost (sequence) view (separated into 
    /// constructor + this method, since it requires the parts of the 
    /// timeline to be created also)... at least for getZoom()
    void init();

    /// For destroying the topmost (sequence) view (separated into 
    /// destructor + this method, since it requires the parts of the 
    /// timeline to be destroyed afterwards)
    void deinit();      

    /// For initializing child views.
    View(View* parent); 

    virtual ~View();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    View& getParent() const;

    virtual pixel requiredWidth() const = 0;
    virtual pixel requiredHeight() const = 0;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onChildViewUpdated( ViewUpdateEvent& event );
    void onZoomChanged( ZoomChangeEvent& event );

    //////////////////////////////////////////////////////////////////////////
    // BITMAP
    //////////////////////////////////////////////////////////////////////////

    const wxBitmap& getBitmap() const;

protected:

    virtual void draw(wxBitmap& bitmap) const = 0;  ///< Is called whenever the bitmap is 'invalidated' and a new bitmap is required.

    void invalidateBitmap();                        ///< Should be called whenever the bitmap must be recreated.

private:


    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxEvtHandler mEvtHandler;
    View* mParent;
    mutable wxBitmap mBitmap;   ///< Mutable to avoid having to non-const all methods using getBitmap()
    mutable bool mBitmapValid;  ///< Mutable to avoid having to non-const all methods using getBitmap()
};

}} // namespace

#endif // VIEW_H
