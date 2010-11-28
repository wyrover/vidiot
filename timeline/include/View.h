#ifndef VIEW_H
#define VIEW_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/region.h>
#include "Part.h"
#include "UtilEvent.h"

namespace gui { namespace timeline {

class View;

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

class View
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    View(Timeline* timeline); ///< For initializing the topmost (sequence) view
    View(View* parent); ///< For initializing child views.
    virtual ~View();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    View& getParent() const;

    virtual int requiredWidth() = 0;
    virtual int requiredHeight() = 0;

    //////////////////////////////////////////////////////////////////////////
    // PROPAGATE UPDATES UPWARD
    //////////////////////////////////////////////////////////////////////////

    void onViewUpdated( ViewUpdateEvent& event ); ///< @see View::onViewUpdated()

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

    View* mParent;
    wxBitmap mBitmap;
    bool mBitmapValid;
};

}} // namespace

#endif // VIEW_H
