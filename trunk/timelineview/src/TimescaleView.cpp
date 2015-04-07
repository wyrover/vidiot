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

void TimescaleView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    wxPoint scrolledAndShiftedPosition = getPosition() - offset;
    wxRegion overlap(region);
    overlap.Intersect(wxRect(getPosition()-offset, getSize()));
    wxRegionIterator upd(overlap);
    if (upd)
    {
        // Determine what to show
        static wxString minutesFormat = "%M:%S";
        static wxString hoursFormat = "%H:%M:%S";

        // NOTE: Match with map used in Zoom!!
        static std::map< rational, TicksAndNumbers> zoomToSteps = {
            //                              Time between ticks               Time between shown times
            { rational(1, 120), TicksAndNumbers(60 * model::Constants::sSecond, 5 * model::Constants::sMinute) },
            { rational(1, 60), TicksAndNumbers(20 * model::Constants::sSecond, 2 * model::Constants::sMinute) },
            { rational(1, 45), TicksAndNumbers(10 * model::Constants::sSecond, 2 * model::Constants::sMinute) },
            { rational(1, 30), TicksAndNumbers(10 * model::Constants::sSecond, 1 * model::Constants::sMinute) },
            { rational(1, 20), TicksAndNumbers(5 * model::Constants::sSecond, 30 * model::Constants::sSecond) },
            { rational(1, 15), TicksAndNumbers(5 * model::Constants::sSecond, 30 * model::Constants::sSecond) },
            { rational(1, 10), TicksAndNumbers(2 * model::Constants::sSecond, 20 * model::Constants::sSecond) },
            { rational(1, 9), TicksAndNumbers(2 * model::Constants::sSecond, 20 * model::Constants::sSecond) },
            { rational(1, 8), TicksAndNumbers(2 * model::Constants::sSecond, 20 * model::Constants::sSecond) },
            { rational(1, 7), TicksAndNumbers(2 * model::Constants::sSecond, 20 * model::Constants::sSecond) },
            { rational(1, 6), TicksAndNumbers(2 * model::Constants::sSecond, 20 * model::Constants::sSecond) },
            { rational(1, 5), TicksAndNumbers(2 * model::Constants::sSecond, 10 * model::Constants::sSecond) },
            { rational(1, 4), TicksAndNumbers(model::Constants::sSecond, 10 * model::Constants::sSecond) },
            { rational(1, 3), TicksAndNumbers(model::Constants::sSecond, 10 * model::Constants::sSecond) },
            { rational(1, 2), TicksAndNumbers(model::Constants::sSecond, 5 * model::Constants::sSecond) },
            { rational(1, 1), TicksAndNumbers(model::Constants::sSecond, 5 * model::Constants::sSecond) },
            { rational(2, 1), TicksAndNumbers(model::Constants::sSecond, 1 * model::Constants::sSecond) },
        };

        rational zoom = getZoom().getCurrent();
        ASSERT(zoomToSteps.find(zoom) != zoomToSteps.end())(zoom)(zoomToSteps);
        TicksAndNumbers steps = zoomToSteps.find(zoom)->second;

        // Draw timescale background
        dc.SetBrush(Layout::get().TimeScaleBackgroundBrush);
        dc.SetPen(Layout::get().TimeScaleDividerPen);
        dc.DrawRectangle(scrolledAndShiftedPosition,getSize());

        // Prepare for drawing times
        dc.SetFont(Layout::get().TimeScaleFont);
        dc.SetTextForeground(Layout::get().TimeScaleFontColour);

        // Draw seconds and minutes lines
        while (upd)
        {
            wxRect r(upd.GetRect());
            int leftMostMs = model::Convert::ptsToTime(getZoom().pixelsToPts(r.GetLeft() + offset.x - getTimeline().getShift()));
            int rightMostMs = model::Convert::ptsToTime(getZoom().pixelsToPts(r.GetRight() + offset.x - getTimeline().getShift()));

            // Besides redrawing 'ticks' the numbers must also be redrawn. Since the
            // extend of the text is beyond the tick (both to the left and right) the totally 
            // redrawn area must be extended to ensure that the time text 'under' the redrawn area
            // is also redrawn. As an example, consder a redraw for a moved timeline. Then, the update
            // region consists only of the 'previous cursor position' and the 'current cursor position'.
            //
            //   T T T T T T T T T T
            //     |   |
            //         |
            //  00:00:00
            //         |
            //        (cursor)
            // When moving the cursor to the position at the end of the time text, the time text needs to
            // be redrawn as well.
            leftMostMs -= leftMostMs % steps.NumberStep;                        // Move to 'tick above number' to the left
            if (leftMostMs < 0) { leftMostMs = 0; }
            rightMostMs += steps.NumberStep - (rightMostMs % steps.NumberStep); // Move to 'tick above number' to the right
            milliseconds max = getZoom().pixelsToTime(getW());
            if (rightMostMs > max) { rightMostMs = max; }

            for (int ms = leftMostMs; ms <= rightMostMs; ms += steps.TickStep)
            {
                int position = getZoom().timeToPixels(ms);
                bool showTime = (ms % steps.NumberStep == 0);

                int linePosition = scrolledAndShiftedPosition.x + position;
                int lineHeight = showTime ? Layout::TimeScaleMinutesHeight : Layout::TimeScaleSecondHeight;
                dc.DrawLine(linePosition, 0, linePosition, lineHeight);

                if (ms == 0)
                {
                    dc.DrawText( "0", scrolledAndShiftedPosition + wxPoint(5, Layout::TimeScaleMinutesHeight));
                }
                else
                {
                    if (showTime)
                    {
                        unsigned short hours = ms / model::Constants::sHour ;
                        wxDateTime t(hours, (ms % model::Constants::sHour) / model::Constants::sMinute, (ms % model::Constants::sMinute) / model::Constants::sSecond, ms % model::Constants::sSecond);
                        wxString format = (hours == 0) ? minutesFormat : hoursFormat; // Don't show hours for the first hour
                        wxString s = t.Format(ms / model::Constants::sHour == 0 ? minutesFormat : hoursFormat);
                        wxSize ts = dc.GetTextExtent(s);
                        dc.DrawText( s, scrolledAndShiftedPosition + wxPoint(position - ts.GetX() / 2, Layout::TimeScaleMinutesHeight));
                    }
                }
            }
            upd++;
        }

        if (Config::getShowDebugInfo())
        {
            dc.SetTextForeground(Layout::get().DebugColour);
            dc.SetFont(Layout::get().DebugFont);
            wxString s; s << "Zoom:" << zoom.numerator() << "/" << zoom.denominator();
            dc.DrawText( s, scrolledAndShiftedPosition + wxPoint(25, Layout::TimeScaleSecondHeight - 5));
        }
    }
}

}} // namespace
