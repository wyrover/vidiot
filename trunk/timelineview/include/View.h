// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef VIEW_H
#define VIEW_H

#include "Part.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

class ViewUpdateEvent;

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

    /// \return the size required to render this object.
    /// Is required for use in this base class to construct the bitmap.
    /// Use getSize() to retrieve the size of the created bitmap.
    virtual wxSize requiredSize() const = 0;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onChildViewUpdated( ViewUpdateEvent& event );

    //////////////////////////////////////////////////////////////////////////
    // BITMAP
    //////////////////////////////////////////////////////////////////////////

    const wxBitmap& getBitmap() const;

    wxSize getSize() const;

    /// Should be called whenever the bitmap must be recreated.
    /// Originally, this method was protected. However, there are scenarios
    /// where it is better (performance wise) to have this method public and let
    /// the invalidation be triggered 'from the outside'. An example of this is
    /// the changing of the height of a track. Instead of making all clips and
    /// thumbbnails listen to track height events (lots of administration) it is
    /// more practical to have one loop (in the code that updates the track height)
    /// iterating over all affected clips/thumbnails. Not only does this save
    /// some administration (each clip registered for track height events) also the
    /// redrawing can be made more efficient by using beginTransaction(); For Loop;
    /// endTransaction();
    void invalidateBitmap();

protected:

    virtual void draw(wxBitmap& bitmap) const = 0;  ///< Is called whenever the bitmap is 'invalidated' and a new bitmap is required.

    void drawDivider(wxDC& dc, pixel yPosition, pixel height) const; ///< Helper for drawing a divider

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