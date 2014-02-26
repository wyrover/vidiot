// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef TIMELINE_H
#define TIMELINE_H

namespace model {
    class EventLengthChanged;
}

namespace command {
    class RootCommand;
}

namespace gui {
    class Player;
    namespace timeline {
        class Cursor;
        class Details;
        class Drag;
        class Intervals;
        class Keyboard;
        class MenuHandler;
        class Mouse;
        class Scrolling;
        class Selection;
        class SequenceView;
        class Tooltip;
        class Trim;
        class ViewMap;
        class Zoom;
        namespace state {
            class Machine;
        }
    }
}

namespace gui { namespace timeline {

class Timeline
    :   public wxScrolledWindow
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Create a new timeline
    /// \param parent containing window (timelinesview)
    /// \param sequence sequence to be shown
    /// \param beginTransacted if true, begins in 'transaction' mode (no screen updates generated yet).
    Timeline(wxWindow *parent, model::SequencePtr sequence, bool beginTransacted = false);
    virtual ~Timeline();

    //////////////////////////////////////////////////////////////////////////
    // PART
    //////////////////////////////////////////////////////////////////////////

    Timeline& getTimeline();
    const Timeline& getTimeline() const;
    SequenceView& getSequenceView();
    const SequenceView& getSequenceView() const;
    Zoom& getZoom();
    const Zoom& getZoom() const;
    ViewMap& getViewMap();
    const ViewMap& getViewMap() const;
    Intervals& getIntervals();
    const Intervals& getIntervals() const;
    Keyboard& getKeyboard();
    const Keyboard& getKeyboard() const;
    Mouse& getMouse();
    const Mouse& getMouse() const;
    Scrolling& getScrolling();
    const Scrolling& getScrolling() const;
    Selection& getSelection();
    const Selection& getSelection() const;
    MenuHandler& getMenuHandler();
    const MenuHandler& getMenuHandler() const;
    Cursor& getCursor();
    const Cursor& getCursor() const;
    Drag& getDrag();
    const Drag& getDrag() const;
    Tooltip& getTooltip();
    const Tooltip& getTooltip() const;
    Trim& getTrim();
    const Trim& getTrim() const;
    state::Machine& getStateMachine();
    const state::Machine& getStateMachine() const;
    model::SequencePtr getSequence();
    const model::SequencePtr getSequence() const;
    Details& getDetails();
    const Details& getDetails() const;

    //////////////////////////////////////////////////////////////////////////
    // WX EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onIdle(wxIdleEvent& event);
    void onSize(wxSizeEvent& event);
    void onEraseBackground(wxEraseEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onSequenceLengthChanged(model::EventLengthChanged& event);

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    void onPaint( wxPaintEvent &event );

    /// Draw a vertical line over the entire timeline
    /// \param dc device context to be used
    /// \param region invalidated region of the timeline
    /// \param offset offset imposed on the drawing actions (caused by scrolling and shift)
    /// \param position horizontal position where the line is drawn
    /// \param pen pen to be used for drawing the line
    void drawLine(wxDC& dc, const wxRegion& region, const wxPoint& offset, pts position, const wxPen& pen) const;

    /// Draw a divider
    /// \param dc device context to be used
    /// \param region invalidated region of the timeline
    /// \param yPosition y position of the divider
    /// \param height height of the divider
    void drawDivider(wxDC& dc, pixel yPosition, pixel height) const;

    /// Fill a region in the timeline with the given overlapping area of this view's rectangle.
    /// \param dc device context to be used
    /// \param region invalidated region of the timeline
    /// \param offset offset imposed on the drawing actions (caused by scrolling and shift)
    /// \param roi rect of the timeline that must be copied from bitmap to region (in full timeline coordinates)
    /// \param mask use mask of the bitmap
    void copyRect(wxDC& dc, wxRegion region, const wxPoint& offset, const wxBitmap& bitmap, const wxRect& roi, bool mask = false) const;

    /// Clear a region in the timeline
    /// \param dc device context to be used
    /// \param region invalidated region of the timeline
    /// \param offset offset imposed on the drawing actions (caused by scrolling and shift)
    /// \param cleared region that is required to be clear
    void clearRect(wxDC& dc, wxRegion region, const wxPoint& offset, const wxRect& cleared) const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void activate(bool active); ///< To be called when the timeline becomes the active one

    Player* getPlayer() const;

    /// Refresh the rectangle for which
    /// [x,y,w,h] == [position,0,1,height],
    /// using logical coordinates and
    /// taking into account scrolling and zooming
    void refreshPts(pts position);

    /// Refresh part of the timeline
    /// \param rect rect to be refreshed, unscrolled position within the sequence view
    void repaint(wxRect rect);

    void setShift(pixel shift);

    pixel getShift() const;

    /// To be called whenever the widget contents is resized. This ensures proper virtual
    /// size of the scrolled areas and proper enabling/disabling of scrollbars.
    void resize();

    //////////////////////////////////////////////////////////////////////////
    // TRANSACTION
    //////////////////////////////////////////////////////////////////////////

    /// Start a 'transaction'. All operations within the transaction do not lead
    /// to screen updates. When the complete set of operations is done (typically,
    /// after a editing operation is done completely), call endTransaction() to
    /// enable screen updates again. This is done to avoid useless intermediate
    /// updates during clip edits (only the result after the final modification
    /// is interesting).
    void beginTransaction();

    /// End a 'transaction'. This enables screen updates again.
    /// Typically, call modelChanged after doing this.
    void endTransaction();

    //////////////////////////////////////////////////////////////////////////
    // CHANGE COMMANDS
    //////////////////////////////////////////////////////////////////////////

    /// Reset the mouse pointer, triggering an update of both the model (since
    /// iterators are reset via the moveTo method) and the timeline (since
    /// resetting the cursor causes an update).
    void modelChanged();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::SequencePtr mSequence;
    Player* mPlayer;

    bool mTransaction;

    pixel mShift;

    wxBitmapPtr mBufferBitmap;

    //////////////////////////////////////////////////////////////////////////
    // PART -> Must be AFTER MEMBERS
    //////////////////////////////////////////////////////////////////////////

    Trim* mTrim;
    Zoom* mZoom;
    ViewMap* mViewMap;
    Intervals* mIntervals;
    Keyboard* mKeyboard;
    Mouse* mMouse;
    Scrolling* mScroll;
    Selection* mSelection;
    Cursor* mCursor;  // Must be AFTER mPlayer and AFTER mScroll
    Drag* mDrag;
    Tooltip* mTooltip;
    state::Machine* mStateMachine; // Must be AFTER mViewMap due to constructor list.
    MenuHandler* mMenuHandler; // Init as last since it depends on other parts
    Details* mDetails; // Must be after all other Parts (since it uses these during the initialization)

    //////////////////////////////////////////////////////////////////////////
    // CHILDREN -> Must be AFTER PARTS
    //////////////////////////////////////////////////////////////////////////

    SequenceView* mSequenceView;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(gui::timeline::Timeline, 1)

#endif
