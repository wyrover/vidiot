// Copyright 2013-2015 Eric Raijmakers.
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

#include "Cursor.h"

#include "Intervals.h"
#include "Player.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "VideoDisplayEvent.h"
#include "Zoom.h"

namespace gui { namespace timeline {

const int Cursor::EDGE_OFFSET = 20;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Cursor::Cursor(Timeline* timeline)
    :   wxEvtHandler()
    ,   Part(timeline)
    ,   mCursorPosition(0)
{
    VAR_DEBUG(this);
}

Cursor::~Cursor()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

pts Cursor::getLogicalPosition() const
{
    return mCursorPosition;
}

void Cursor::setLogicalPosition(pts position)
{
    VAR_DEBUG(position);
    if (position >= 0 && position <= getZoom().pixelsToPts(getSequenceView().getW())) // avoid out of bounds
    {
        moveTo(position);
        getPlayer()->moveTo(mCursorPosition);
    }
}

void Cursor::prevFrame()
{
    setLogicalPosition(getLogicalPosition() - 1);
    ensureCursorVisible();
}

void Cursor::nextFrame()
{
    setLogicalPosition(getLogicalPosition() + 1);
    ensureCursorVisible();
}

void Cursor::prevCut()
{
    std::set<pts> cuts = getSequence()->getCuts(); // std::set is stored in ordered fashion
    pts current = getLogicalPosition();
    for ( pts position : boost::adaptors::reverse( cuts ) )
    {
        if (position < current)
        {
            setLogicalPosition(position);
            ensureCursorVisible();
            return;
        }
    }
}

void Cursor::nextCut()
{
    pts current = getLogicalPosition();
    for ( pts position : getSequence()->getCuts() ) // std::set is stored in ordered fashion
    {
        if (position > current)
        {
            setLogicalPosition(position);
            ensureCursorVisible();
            return;
        }
    }
}

void Cursor::home()
{
    setLogicalPosition(0);
    ensureCursorVisible();
}

void Cursor::end()
{
    setLogicalPosition(getSequence()->getLength());
    ensureCursorVisible();
}

void Cursor::focus()
{
    wxPoint scroll = getScrolling().getOffset();
    wxSize size = getTimeline().GetClientSize();

    getScrolling().align(mCursorPosition, size.x / 4);

}

void Cursor::onPlaybackPosition(pts position)
{
    moveTo(position);

    // Ensure that the scrolling is adjusted if playback moves the cursor beyond the visible region.
    // Note: specifically located here since a slightly different adjustment mechanism is used for user actions.
    wxPoint scroll = getScrolling().getOffset();
    wxSize size = getTimeline().GetClientSize();

    pts lastVisibleFrame = getZoom().pixelsToPts(scroll.x + size.x - EDGE_OFFSET);
    if (mCursorPosition > lastVisibleFrame  &&
        mCursorPosition <= lastVisibleFrame + getZoom().pixelsToPts(4) )
    {
        // mCursorPosition > lastVisibleFrame:
        //    ensures automated scrolling starts when the cursor moves 'too far'
        // mCursorPosition <= lastVisibleFrame + getZoom().pixelsToPts(2):
        //    avoids automated scrolling to mess up manual scrolling during playback.
        //    pixelsToPts(4) is used for the maximum zoom level.
        getScrolling().align(mCursorPosition, size.x - EDGE_OFFSET);
    }
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Cursor::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    getTimeline().drawLine(dc, region, offset, mCursorPosition, wxPen{ wxColour{ 255, 0, 0 } });
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Cursor::moveTo(pts position)
{
    if (position != mCursorPosition)
    {
        VAR_DEBUG(mCursorPosition)(position);

        long oldPixelPos = getZoom().ptsToPixels(mCursorPosition);
        long newPixelPos = getZoom().ptsToPixels(position);
        mCursorPosition = position;

        if (oldPixelPos != newPixelPos)
        {
            // Refresh the old and new cursor position areas
            getTimeline().repaint(wxRect(oldPixelPos,0,1,getSequenceView().getH()));
            getTimeline().repaint(wxRect(newPixelPos,0,1,getSequenceView().getH()));
            getTimeline().Update(); // Use this for better feedback when dragging cursor..

            getIntervals().update(mCursorPosition);
        }
    }
}

void Cursor::ensureCursorVisible()
{
    wxPoint scroll = getScrolling().getOffset();
    wxSize size = getTimeline().GetClientSize();

    // Left side
    pts firstVisibleFrame = getZoom().pixelsToPts(scroll.x);
    if (firstVisibleFrame > mCursorPosition)
    {
        getScrolling().align(mCursorPosition, EDGE_OFFSET);
    }
    // Right side
    pts lastVisibleFrame = getZoom().pixelsToPts(scroll.x + size.x);
    if (lastVisibleFrame < mCursorPosition)
    {
        getScrolling().align(mCursorPosition, size.x - EDGE_OFFSET);
    }
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Cursor::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_NVP(mCursorPosition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Cursor::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Cursor::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
