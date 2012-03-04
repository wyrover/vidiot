#include "TimescaleView.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include "Constants.h"
#include "Intervals.h"
#include "Layout.h"
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

wxSize TimescaleView::requiredSize() const
{
    return wxSize(getTimeline().getSequenceView().minimumWidth(), Layout::sTimeScaleHeight);
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

    // Draw timescale
    dc.SetBrush(wxNullBrush);
    dc.SetPen(Layout::sTimeScaleDividerPen);
    dc.DrawRectangle(0,0,w,h);

    dc.SetFont(*Layout::sTimeScaleFont);

    // Draw seconds and minutes lines
    for (int ms = 0; getZoom().timeToPixels(ms) <= w; ms += model::Constants::sSecond)
    {
        int position = getZoom().timeToPixels(ms);
        bool isMinute = (ms % model::Constants::sMinute == 0);
        int height = Layout::sTimeScaleSecondHeight;

        if (isMinute)
        {
            height = Layout::sTimeScaleMinutesHeight;
        }

        dc.DrawLine(position,0,position,height);

        if (ms == 0)
        {
            dc.DrawText( "0", 5, Layout::sTimeScaleMinutesHeight );
        }
        else
        {
            if (isMinute)
            {
                wxDateTime t(ms / model::Constants::sHour, (ms % model::Constants::sHour) / model::Constants::sMinute, (ms % model::Constants::sMinute) / model::Constants::sSecond, ms % model::Constants::sSecond);
                wxString s = t.Format("%H:%M:%S.%l");
                wxSize ts = dc.GetTextExtent(s);
                dc.DrawText( s, position - ts.GetX() / 2, Layout::sTimeScaleMinutesHeight );
            }
        }
    }
}

}} // namespace