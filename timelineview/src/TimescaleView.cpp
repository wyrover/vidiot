// Copyright 2013-2016 Eric Raijmakers.
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
#include "Convert.h"
#include "Intervals.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Zoom.h"

namespace gui { namespace timeline {

const pixel TimeScaleMinutesHeight{ 10 };
const pixel TimeScaleSecondHeight{ 5 };
const pixel TimescaleView::TimeScaleHeight{ 25 };

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
    return TimescaleView::TimeScaleHeight;
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
        static std::map< rational64, TicksAndNumbers> zoomToSteps = {
            //                              Time between ticks               Time between shown times
            { rational64{1, 120}, TicksAndNumbers(60 * sSecond, 5 * sMinute) },
            { rational64{1, 60}, TicksAndNumbers(20 * sSecond, 2 * sMinute) },
            { rational64{1, 45}, TicksAndNumbers(10 * sSecond, 2 * sMinute) },
            { rational64{1, 30}, TicksAndNumbers(10 * sSecond, 1 * sMinute) },
            { rational64{1, 20}, TicksAndNumbers(5 * sSecond, 30 * sSecond) },
            { rational64{1, 15}, TicksAndNumbers(5 * sSecond, 30 * sSecond) },
            { rational64{1, 10}, TicksAndNumbers(2 * sSecond, 20 * sSecond) },
            { rational64{1, 9}, TicksAndNumbers(2 * sSecond, 20 * sSecond) },
            { rational64{1, 8}, TicksAndNumbers(2 * sSecond, 20 * sSecond) },
            { rational64{1, 7}, TicksAndNumbers(2 * sSecond, 20 * sSecond) },
            { rational64{1, 6}, TicksAndNumbers(2 * sSecond, 20 * sSecond) },
            { rational64{1, 5}, TicksAndNumbers(2 * sSecond, 10 * sSecond) },
            { rational64{1, 4}, TicksAndNumbers(sSecond, 10 * sSecond) },
            { rational64{1, 3}, TicksAndNumbers(sSecond, 10 * sSecond) },
            { rational64{1, 2}, TicksAndNumbers(sSecond, 5 * sSecond) },
            { rational64{1, 1}, TicksAndNumbers(sSecond, 5 * sSecond) },
            { rational64{2, 1}, TicksAndNumbers(sSecond, 1 * sSecond) },
        };

        rational64 zoom = getZoom().getCurrent();
        ASSERT(zoomToSteps.find(zoom) != zoomToSteps.end())(zoom)(zoomToSteps);
        TicksAndNumbers steps = zoomToSteps.find(zoom)->second;

        int timelineWidth{ getTimeline().GetClientSize().GetWidth() };
		
        // Draw timescale background
        // Note: Not use simply DrawRectangle. Given the size of the drawing (width of entire timeline)
        //       DrawRectangle sometimes fails to draw (particularly when zooming in fully).
        dc.SetBrush(wxBrush{ wxColour{ 255, 255, 255 } , wxBRUSHSTYLE_SOLID});
        dc.SetPen(wxPen{ wxColour{ 255, 255, 255 }, 1 });
        dc.DrawRectangle(wxPoint(0, -offset.y), wxSize(timelineWidth, getH())); // Fill with white, but only the update region
        dc.SetPen(wxPen{ wxColour{ 0, 0, 0 }, 1 });
        dc.DrawLine(wxPoint(0,0 - offset.y), wxPoint(timelineWidth,0 - offset.y));
        dc.DrawLine(wxPoint(0,getH() - 1 - offset.y), wxPoint(timelineWidth,getH() - 1 - offset.y));
        dc.DrawLine(wxPoint(0,0) - offset, wxPoint(0,getH()) - offset); // Left line of the bounding box
        dc.DrawLine(wxPoint(getW() - 1,0)  - offset, wxPoint(getW() - 1,getH()) - offset); // Right line of the bounding box

        // Prepare for drawing times
        dc.SetFont(wxFont(wxSize(0,11),wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL));
        dc.SetTextForeground(wxColour{ 0, 0, 0 });

        // Draw seconds and minutes lines
        while (upd)
        {
            wxRect r(upd.GetRect());
            int leftMostMs = model::Convert::ptsToTime(getZoom().pixelsToPts(r.GetLeft() + offset.x - getTimeline().getShift()));
            int rightMostMs = model::Convert::ptsToTime(getZoom().pixelsToPts(r.GetRight() + offset.x - getTimeline().getShift()));

            // Besides redrawing 'ticks' the numbers must also be redrawn. Since the
            // text is beyond the tick (both to the left and right) the totally
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

                int lineHeight = showTime ? TimeScaleMinutesHeight : TimeScaleSecondHeight;
                dc.DrawLine(scrolledAndShiftedPosition + wxPoint(position,0), scrolledAndShiftedPosition + wxPoint(position, lineHeight));

                if (ms == 0)
                {
                    dc.DrawText( "0", scrolledAndShiftedPosition + wxPoint(5, TimeScaleMinutesHeight));
                }
                else
                {
                    if (showTime)
                    {
                        unsigned short hours = ms / sHour ;
                        wxDateTime t(hours, (ms % sHour) / sMinute, (ms % sMinute) / sSecond, ms % sSecond);
                        wxString format = (hours == 0) ? minutesFormat : hoursFormat; // Don't show hours for the first hour
                        wxString s = t.Format(ms / sHour == 0 ? minutesFormat : hoursFormat);
                        wxSize ts = dc.GetTextExtent(s);
                        dc.DrawText( s, scrolledAndShiftedPosition + wxPoint(position - ts.GetX() / 2, TimeScaleMinutesHeight));
                    }
                }
            }
            upd++;
        }

        if (Config::getShowDebugInfo())
        {
            dc.SetTextForeground(wxColour{ 0, 255, 0 });
            dc.SetFont(wxFont(wxSize(0,11),wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL));
            wxString s; s << "Zoom:" << zoom.numerator() << "/" << zoom.denominator();
            dc.DrawText( s, scrolledAndShiftedPosition + wxPoint(25, TimeScaleSecondHeight - 5));
        }
    }
}

}} // namespace
