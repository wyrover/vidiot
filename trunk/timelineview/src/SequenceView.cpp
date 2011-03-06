#include "SequenceView.h"

#include <wx/dcclient.h>
#include <algorithm>
#include <wx/dcmemory.h>
#include "AudioView.h"
#include "Drag.h"
#include "Drop.h"
#include "Cursor.h"
#include "Intervals.h"
#include "Layout.h"
#include "VideoView.h"
#include "UtilLog.h"
#include "Divider.h"
#include "Zoom.h"
#include "Constants.h"
#include "Sequence.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

SequenceView::SequenceView(View* parent)
:   View(parent)
,   mVideoView(new VideoView(this))
,   mAudioView(new AudioView(this))
{
    VAR_DEBUG(this);
}

SequenceView::~SequenceView()
{
    VAR_DEBUG(this);

    delete mAudioView;      mAudioView = 0;
    delete mVideoView;      mVideoView = 0;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

VideoView& SequenceView::getVideo()
{
    return *mVideoView;
}

const VideoView& SequenceView::getVideo() const
{
    return *mVideoView;
}

AudioView& SequenceView::getAudio()
{
    return *mAudioView;
}

const AudioView& SequenceView::getAudio() const
{
    return *mAudioView;
}

pixel SequenceView::requiredWidth() const
{
    return
        std::max(
        getZoom().timeToPixels(5 * model::Constants::sMinute),         // Minimum width of 5 minutes
        getZoom().ptsToPixels(getSequence()->getLength()));    // At least enough to hold all clips
}

pixel SequenceView::requiredHeight() const
{
    return
        Layout::sTimeScaleHeight +
        Layout::sMinimalGreyAboveVideoTracksHeight +
        getVideo().requiredHeight() +
        Layout::sAudioVideoDividerHeight +
        getAudio().requiredHeight() +
        Layout::sMinimalGreyBelowAudioTracksHeight;                     // Height of all combined components
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void SequenceView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);

    // Get size of canvas
    int w = bitmap.GetWidth();
    int h = bitmap.GetHeight();

    // Set BG
    dc.SetPen(Layout::sBackgroundPen);
    dc.SetBrush(Layout::sBackgroundBrush);
    dc.DrawRectangle(0,0,w,h);

    // Draw timescale
    dc.SetBrush(wxNullBrush);
    dc.SetPen(Layout::sTimeScaleDividerPen);
    dc.DrawRectangle(0,0,w,Layout::sTimeScaleHeight);

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

    // Get video and audio bitmaps, possibly required for determining divider position
    const wxBitmap& videotracks = getVideo().getBitmap();
    const wxBitmap& audiotracks = getAudio().getBitmap();

    dc.DrawBitmap(videotracks,wxPoint(0,getDivider().getVideoPosition()));
    dc.DrawBitmap(audiotracks,wxPoint(0,getDivider().getAudioPosition()));

    getDivider().draw(dc);
    getIntervals().draw(dc);
    getDrag().draw(dc);
    getDrop().draw(dc);
    getCursor().draw(dc);
}

}} // namespace
