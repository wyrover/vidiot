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
// GET/SET
//////////////////////////////////////////////////////////////////////////

void TimescaleView::canvasResized()
{
    invalidateBitmap();
}

wxSize TimescaleView::requiredSize() const
{
    return wxSize(getParent().getSize().GetWidth(), Layout::TimeScaleHeight);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TimescaleView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);

    // Get size of canvas
    int w = bitmap.GetWidth();
    int h = bitmap.GetHeight();

    // Determine what to show
    rational zoom = getZoom().getCurrent();
    bool showHours = getZoom().pixelsToTime(w) >= model::Constants::sHour;
    wxString sHours = showHours ? "%H:" : "";
    wxString sMilliseconds = (zoom > rational(1,30)) ? ".%l" : "";
    wxString minutesFormat = sHours + "%M:%S" + sMilliseconds;

    int tickStep = 60 * model::Constants::sSecond; // One tick per minute
    int numberStep = model::Constants::sMinute; // Show a number for each minute
    if (zoom <= rational(1,120))
    {
        tickStep = model::Constants::sMinute;       // One tick per 1 minute
        numberStep = 5 * model::Constants::sMinute; // Show a number for each 5th minute
    }
    else if (zoom <= rational(1,60))
    {
        tickStep = model::Constants::sMinute;       // One tick per 1 minute
        numberStep = 2 * model::Constants::sMinute; // Show a number for each 2nd minute
    }
    else if (zoom <= rational(1,30))
    {
        tickStep = 10 * model::Constants::sSecond; // One tick per 20 seconds
    }
    else if (zoom <= rational(1,20))
    {
        tickStep = 5 * model::Constants::sSecond; // One tick per 10 seconds
    }
    else if (zoom <= rational(1,15))
    {
        tickStep = 5 * model::Constants::sSecond; // One tick per 5 seconds
    }
    else if (zoom <= rational(1,8))
    {
        tickStep = 2 * model::Constants::sSecond; // One tick per 2 seconds
    }
    else
    {
        tickStep = model::Constants::sSecond; // One tick per second
    }

    // Draw timescale
    dc.SetBrush(wxNullBrush);
    dc.SetPen(Layout::get().TimeScaleDividerPen);
    dc.DrawRectangle(0,0,w,h);

    // Draw seconds and minutes lines
    dc.SetFont(Layout::get().TimeScaleFont);
    dc.SetTextForeground(Layout::get().TimeScaleFontColour);
    for (int ms = 0; getZoom().timeToPixels(ms) <= w; ms += tickStep)
    {
        int position = getZoom().timeToPixels(ms);
        bool showTime = (ms % numberStep == 0);

        dc.DrawLine(position,0,position, showTime ? Layout::TimeScaleMinutesHeight : Layout::TimeScaleSecondHeight);

        if (ms == 0)
        {
            dc.DrawText( "0", 5, Layout::TimeScaleMinutesHeight );
        }
        else
        {
            if (showTime)
            {
                wxDateTime t(ms / model::Constants::sHour, (ms % model::Constants::sHour) / model::Constants::sMinute, (ms % model::Constants::sMinute) / model::Constants::sSecond, ms % model::Constants::sSecond);
                wxString s = t.Format(minutesFormat);
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