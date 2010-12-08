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
    :   public Part
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

    virtual pixel requiredWidth() const = 0;
    virtual pixel requiredHeight() const = 0;

    //////////////////////////////////////////////////////////////////////////
    // PROPAGATE UPDATES UPWARD
    //////////////////////////////////////////////////////////////////////////

    void onChildViewUpdated( ViewUpdateEvent& event );

    //////////////////////////////////////////////////////////////////////////
    // BITMAP
    //////////////////////////////////////////////////////////////////////////

    const wxBitmap& getBitmap() const;

protected:

    /**
    * Is called whenever the bitmap is 'invalidated' and a new bitmap
    * is required.
    **/
    virtual void draw(wxBitmap& bitmap) const = 0;

public: /** @todo should be protected, but couldn't yet due to use in 'Selection()' */
    /**
    * Should be called whenever the bitmap must be recreated.
    **/
    void invalidateBitmap();

private:


    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxEvtHandler mEvtHandler;
    View* mParent;
    mutable wxBitmap mBitmap; ///< Mutable to avoid having to non-const all methods using getBitmap()
    mutable bool mBitmapValid; ///< Mutable to avoid having to non-const all methods using getBitmap()
};

}} // namespace

#endif // VIEW_H
