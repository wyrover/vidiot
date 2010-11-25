#ifndef VIEW_H
#define VIEW_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/window.h>
#include <wx/region.h>
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "Part.h"
#include "UtilEvent.h"

namespace gui { namespace timeline {

class View;
class Timeline;

class ViewUpdate
{
public:
    ViewUpdate(View& view, wxRegion area)
        :   mView(view)
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
        return mView;
    }
    wxRegion getArea()
    {
        return mArea;
    }
private:
    View& mView;
    wxRegion mArea;
};

DECLARE_EVENT(VIEW_UPDATE_EVENT, ViewUpdateEvent, ViewUpdate);

class IView
{
public:

    //////////////////////////////////////////////////////////////////////////
    // PROPAGATE UPDATES UPWARD
    //////////////////////////////////////////////////////////////////////////

    /**
    * Should be bound (using ::Bind) to all subviews that this view uses
    * to draw its bitmap. This is done in the View constructor (where
    * the child register events for the parent).
    **/
    virtual void onViewUpdated( ViewUpdateEvent& event ) = 0;
};

class View
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
    ,   public IView
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    View(IView* parent);
    virtual ~View();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    // @todo protected?
    virtual int requiredWidth() = 0;
    virtual int requiredHeight() = 0;

    //////////////////////////////////////////////////////////////////////////
    // PROPAGATE UPDATES UPWARD
    //////////////////////////////////////////////////////////////////////////

    void onViewUpdated( ViewUpdateEvent& event );

    //////////////////////////////////////////////////////////////////////////
    // BITMAP
    //////////////////////////////////////////////////////////////////////////

    const wxBitmap& getBitmap();

protected:

    /**
    * Is called whenever the bitmap is 'invalidated' and a new bitmap
    * is required.
    **/
    virtual void draw(wxBitmap& bitmap) = 0;

public: /** @todo should be protected, but couldn't yet due to use in 'Selection()' */
    /**
    * Should be called whenever the bitmap must be recreated.
    **/
    void invalidateBitmap();

private:


    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxBitmap mBitmap;
    bool mBitmapValid;
};

}} // namespace

#endif // VIEW_H
