#include "TimelineView.h"
#include <algorithm>
#include <wx/dcmemory.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "Constants.h"
#include "UtilLog.h"
#include "Sequence.h"
#include "Zoom.h"
#include "VideoView.h"
#include "AudioView.h"
#include "Timeline.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

TimelineView::TimelineView(Timeline* timeline)
:   View(timeline)
,   mVideoView(new VideoView(this))
,   mAudioView(new AudioView(this))
,   mDividerPosition(0)
{
    LOG_INFO;
}

TimelineView::~TimelineView()
{
}

//////////////////////////////////////////////////////////////////////////
// PROPAGATE UPDATES UPWARD
//////////////////////////////////////////////////////////////////////////

void TimelineView::onViewUpdated( ViewUpdateEvent& event )
{
    invalidateBitmap(); 
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

VideoView& TimelineView::getVideo()
{
    return *mVideoView;
}

AudioView& TimelineView::getAudio()
{
    return *mAudioView;
}

int TimelineView::requiredWidth()
{
    return
        std::max(std::max(
        getTimeline().GetClientSize().GetWidth(),                       // At least the widget size
        getZoom().timeToPixels(5 * Constants::sMinute)),                 // Minimum width of 5 minutes
        getZoom().ptsToPixels(getSequence()->getNumberOfFrames()));    // At least enough to hold all clips
}

int TimelineView::requiredHeight()
{
    return
        std::max(
        getTimeline().GetClientSize().GetHeight(),                      // At least the widget size
        Constants::sTimeScaleHeight +
        Constants::sMinimalGreyAboveVideoTracksHeight +
        getVideo().requiredHeight() +
        Constants::sAudioVideoDividerHeight +
        getAudio().requiredHeight() +
        Constants::sMinimalGreyBelowAudioTracksHeight);                 // Height of all combined components
}

int TimelineView::getDividerPosition() const
{
    return mDividerPosition;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TimelineView::draw(wxBitmap& bitmap)
{
    LOG_DEBUG;
    wxMemoryDC dc(bitmap);

    if (mDividerPosition == 0)
    {
        // Initial, default position
        mDividerPosition =
            Constants::sTimeScaleHeight +
            Constants::sMinimalGreyAboveVideoTracksHeight +
            (requiredHeight() - Constants::sTimeScaleHeight - Constants::sMinimalGreyAboveVideoTracksHeight - Constants::sAudioVideoDividerHeight) / 2;
    }

    // Get size of canvas
    int w = bitmap.GetWidth();
    int h = bitmap.GetHeight();

    // Set BG
    dc.SetPen(Constants::sBackgroundPen);
    dc.SetBrush(Constants::sBackgroundBrush);
    dc.DrawRectangle(0,0,w,h);

    // Draw timescale
    dc.SetBrush(wxNullBrush);
    dc.SetPen(Constants::sTimeScaleDividerPen);
    dc.DrawRectangle(0,0,w,Constants::sTimeScaleHeight);

    dc.SetFont(*Constants::sTimeScaleFont);

    // Draw seconds and minutes lines
    for (int ms = 0; getZoom().timeToPixels(ms) <= w; ms += Constants::sSecond)
    {
        int position = getZoom().timeToPixels(ms);
        bool isMinute = (ms % Constants::sMinute == 0);
        int height = Constants::sTimeScaleSecondHeight;

        if (isMinute)
        {
            height = Constants::sTimeScaleMinutesHeight;
        }

        dc.DrawLine(position,0,position,height);

        if (ms == 0)
        {
            dc.DrawText( "0", 5, Constants::sTimeScaleMinutesHeight );
        }
        else
        {
            if (isMinute)
            {
                wxDateTime t(ms / Constants::sHour, (ms % Constants::sHour) / Constants::sMinute, (ms % Constants::sMinute) / Constants::sSecond, ms % Constants::sSecond);
                wxString s = t.Format("%H:%M:%S.%l");
                wxSize ts = dc.GetTextExtent(s);
                dc.DrawText( s, position - ts.GetX() / 2, Constants::sTimeScaleMinutesHeight );
            }
        }
    }

    const wxBitmap& videotracks = getVideo().getBitmap();
    dc.DrawBitmap(videotracks,wxPoint(0,mDividerPosition - videotracks.GetHeight()));

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxWHITE_PEN);
    dc.DrawRectangle(0,mDividerPosition - videotracks.GetHeight(),w,videotracks.GetHeight());

    // Draw divider between video and audio tracks
    dc.SetBrush(Constants::sAudioVideoDividerBrush);
    dc.SetPen(Constants::sAudioVideoDividerPen);
    dc.DrawRectangle(0,mDividerPosition,w,Constants::sAudioVideoDividerHeight);

    const wxBitmap& audiotracks = getAudio().getBitmap();
    dc.DrawBitmap(audiotracks,wxPoint(0,mDividerPosition + Constants::sAudioVideoDividerHeight));
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TimelineView::serialize(Archive & ar, const unsigned int version)
{
    ar & mDividerPosition;
}
template void TimelineView::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void TimelineView::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
