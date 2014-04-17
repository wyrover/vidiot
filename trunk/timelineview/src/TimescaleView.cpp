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

#include "TimescaleView.h"

#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Intervals.h"
#include "Layout.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

TimescaleView::TimescaleView(View* parent)
:   View(parent)
,   mBitmap(boost::none)
{
    VAR_DEBUG(this);

}

TimescaleView::~TimescaleView()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// VIEW
//////////////////////////////////////////////////////////////////////////

pixel TimescaleView::getX() const
{
    return getParent().getX();
}

pixel TimescaleView::getY() const
{
    return 0;
}

pixel TimescaleView::getW() const
{
    return getParent().getW();
}

pixel TimescaleView::getH() const
{
    return Layout::TimeScaleHeight;
}

void TimescaleView::invalidateRect()
{
    mBitmap.reset();
}

void TimescaleView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (!mBitmap)
    {
        mBitmap.reset(wxBitmap(getSize()));
        draw(*mBitmap);
    }
    getTimeline().copyRect(dc,region,offset, *mBitmap, getRect());
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

struct TicksAndNumbers
{
    TicksAndNumbers(int tickStep, int numberStep)
        : TickStep(tickStep)
        , NumberStep(numberStep)
    {
    }
    int TickStep;
    int NumberStep;

    friend std::ostream& operator<<(std::ostream& os, const TicksAndNumbers& obj)
    {
        os << obj.TickStep << '|' << obj.NumberStep;
        return os;
    }
};

void TimescaleView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);

    // Get size of canvas
    int w = bitmap.GetWidth();
    int h = bitmap.GetHeight();

    // Determine what to show
    rational zoom = getZoom().getCurrent();
    wxString minutesFormat = "%M:%S";
    wxString hoursFormat = "%H:%M:%S";

    static std::map< rational, TicksAndNumbers> zoomToSteps =  // NOTE: Match with map used in Zoom!!!
        boost::assign::map_list_of      // Time between ticks          Time between shown times
        (rational(1,120), TicksAndNumbers( 60 * model::Constants::sSecond,  5 * model::Constants::sMinute ))
        (rational(1,60),  TicksAndNumbers( 20 * model::Constants::sSecond,  2 * model::Constants::sMinute ))
        (rational(1,45),  TicksAndNumbers( 10 * model::Constants::sSecond,  2 * model::Constants::sMinute ))
        (rational(1,30),  TicksAndNumbers( 10 * model::Constants::sSecond,  1 * model::Constants::sMinute ))
        (rational(1,20),  TicksAndNumbers(  5 * model::Constants::sSecond, 30 * model::Constants::sSecond ))
        (rational(1,15),  TicksAndNumbers(  5 * model::Constants::sSecond, 30 * model::Constants::sSecond ))
        (rational(1,10),  TicksAndNumbers(  2 * model::Constants::sSecond, 20 * model::Constants::sSecond ))
        (rational(1,9),   TicksAndNumbers(  2 * model::Constants::sSecond, 20 * model::Constants::sSecond ))
        (rational(1,8),   TicksAndNumbers(  2 * model::Constants::sSecond, 20 * model::Constants::sSecond ))
        (rational(1,7),   TicksAndNumbers(  2 * model::Constants::sSecond, 20 * model::Constants::sSecond ))
        (rational(1,6),   TicksAndNumbers(  2 * model::Constants::sSecond, 20 * model::Constants::sSecond ))
        (rational(1,5),   TicksAndNumbers(  2 * model::Constants::sSecond, 10 * model::Constants::sSecond ))
        (rational(1,4),   TicksAndNumbers(      model::Constants::sSecond, 10 * model::Constants::sSecond ))
        (rational(1,3),   TicksAndNumbers(      model::Constants::sSecond, 10  * model::Constants::sSecond ))
        (rational(1,2),   TicksAndNumbers(      model::Constants::sSecond,  5 * model::Constants::sSecond ))
        (rational(1,1),   TicksAndNumbers(      model::Constants::sSecond,  5 * model::Constants::sSecond ))
        (rational(2,1),   TicksAndNumbers(      model::Constants::sSecond,  1 * model::Constants::sSecond ));

    ASSERT(zoomToSteps.find(zoom) != zoomToSteps.end())(zoom)(zoomToSteps);
    TicksAndNumbers steps = zoomToSteps.find(zoom)->second;

    // Draw timescale
    dc.SetBrush(wxNullBrush);
    dc.SetPen(Layout::get().TimeScaleDividerPen);
    dc.DrawRectangle(0,0,w,h);

    // Draw seconds and minutes lines
    dc.SetFont(Layout::get().TimeScaleFont);
    dc.SetTextForeground(Layout::get().TimeScaleFontColour);
    for (int ms = 0; getZoom().timeToPixels(ms) <= w; ms += steps.TickStep)
    {
        int position = getZoom().timeToPixels(ms);
        bool showTime = (ms % steps.NumberStep == 0);

        dc.DrawLine(position,0,position, showTime ? Layout::TimeScaleMinutesHeight : Layout::TimeScaleSecondHeight);

        if (ms == 0)
        {
            dc.DrawText( "0", 5, Layout::TimeScaleMinutesHeight );
        }
        else
        {
            if (showTime)
            {
                unsigned short hours = ms / model::Constants::sHour ;
                wxDateTime t(hours, (ms % model::Constants::sHour) / model::Constants::sMinute, (ms % model::Constants::sMinute) / model::Constants::sSecond, ms % model::Constants::sSecond);
                wxString format = hoursFormat;
                if (hours == 0)
                {
                    // Don't show hours for the first hours
                    format = minutesFormat;
                }
                wxString s = t.Format(ms / model::Constants::sHour == 0 ? minutesFormat : hoursFormat);
                wxSize ts = dc.GetTextExtent(s);
                dc.DrawText( s, position - ts.GetX() / 2, Layout::TimeScaleMinutesHeight );
            }
        }
    }

    if (Config::getShowDebugInfo())
    {
        dc.SetTextForeground(Layout::get().DebugColour);
        dc.SetFont(Layout::get().DebugFont);
        wxString s; s << "Zoom:" << zoom.numerator() << "/" << zoom.denominator();
        dc.DrawText( s, 25, Layout::TimeScaleSecondHeight - 5 );
    }

}

}} // namespace