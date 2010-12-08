#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <wx/font.h>
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/colour.h>

namespace gui { namespace timeline {

class Constants
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TIME
    //////////////////////////////////////////////////////////////////////////

    static const int sMicrosecondsPerSecond;
    static const int sMilliSecond;
    static const int sSecond;
    static const int sMinute;
    static const int sHour;

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    static const int sTimeScaleMinutesHeight;
    static const int sTimeScaleSecondHeight;
    static const int sTimeScaleHeight;
    static const int sMinimalGreyAboveVideoTracksHeight;
    static const int sMinimalGreyBelowAudioTracksHeight;
    static const int sTrackDividerHeight;
    static const int sDefaultAudioVideoDividerPosition;
    static const int sAudioVideoDividerHeight;
    static const int sClipBorderSize;
    static const int sVideoPosition;

    //////////////////////////////////////////////////////////////////////////
    // BRUSHES AND PENS
    //////////////////////////////////////////////////////////////////////////

	static const wxColour	sBackgroundColour;

    static const wxPen      sBackgroundPen;
    static const wxBrush    sBackgroundBrush;

    static const wxPen      sTimeScaleDividerPen;

    static const wxPen      sCursorPen;

    static const wxPen      sDropAreaPen;
    static const wxBrush    sDropAreaBrush;

    static const wxPen      sTrackDividerPen;
    static const wxBrush    sTrackDividerBrush;

    static const wxPen      sAudioVideoDividerPen;
    static const wxBrush    sAudioVideoDividerBrush;

    static const wxPen      sClipPen;
    static const wxBrush    sClipBrush;

    static const wxPen      sSelectedClipPen;
    static const wxBrush    sSelectedClipBrush;

    static const wxPen      sDebugPen;
    static const wxBrush    sDebugBrush;

    //////////////////////////////////////////////////////////////////////////
    // FONTS
    //////////////////////////////////////////////////////////////////////////

    static void initializeFonts();

    static wxFont* sTimeScaleFont;

};

}} // namespace

#endif // CONSTANTS_H