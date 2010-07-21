#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <wx/font.h>
#include <wx/pen.h>
#include <wx/brush.h>

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
    static const int sDefaultAudioVideoDividerPosition;
    static const int sAudioVideoDividerHeight;

    static const int sClipBorderSize;


    //////////////////////////////////////////////////////////////////////////
    // BRUSHES AND PENS
    //////////////////////////////////////////////////////////////////////////

    static const wxPen      sBackgroundPen;
    static const wxBrush    sBackgroundBrush;

    static const wxPen      sTimeScaleDividerPen;

    static const wxPen      sCursorPen;

    static const wxPen      sDropAreaPen;
    static const wxBrush    sDropAreaBrush;

    static const wxPen      sAudioVideoDividerPen;
    static const wxBrush    sAudioVideoDividerBrush;

    static const wxPen      sClipPen;
    static const wxBrush    sClipBrush;

    static const wxPen      sSelectedClipPen;
    static const wxBrush    sSelectedClipBrush;

    //////////////////////////////////////////////////////////////////////////
    // FONTS
    //////////////////////////////////////////////////////////////////////////

    static void initializeFonts();

    static wxFont* sTimeScaleFont;

};


#endif // CONSTANTS_H