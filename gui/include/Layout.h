#ifndef LAYOUT_H
#define LAYOUT_H

#include <wx/font.h>
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/colour.h>

namespace gui {

class Layout
{
public:

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
    static const int sMinTrackHeight;
    static const int sMaxTrackHeight;
    static const int sClipBorderSize;
    static const int sVideoPosition;
    static int sClipDescriptionBarHeight;

    //////////////////////////////////////////////////////////////////////////
    // BRUSHES AND PENS
    //////////////////////////////////////////////////////////////////////////

	static const wxColour	sBackgroundColour;
    static const wxPen      sBackgroundPen;
    static const wxBrush    sBackgroundBrush;

    static const wxColour	sPreviewBackgroundColour;
    static const wxPen      sPreviewBackgroundPen;
    static const wxBrush    sPreviewBackgroundBrush;

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

    static const wxColour   sClipDescriptionFGColour;
    static const wxColour   sClipDescriptionBGColour;
    static const wxPen      sClipDescriptionPen;
    static const wxBrush    sClipDescriptionBrush;

    static const wxPen      sSelectedClipPen;
    static const wxBrush    sSelectedClipBrush;

    static const wxPen      sSnapPen;
    static const wxBrush    sSnapBrush;

    static const wxColour   sDebugColour;
    static const wxPen      sDebugPen;
    static const wxBrush    sDebugBrush;

    //////////////////////////////////////////////////////////////////////////
    // SNAP
    //////////////////////////////////////////////////////////////////////////

    static const int       sSnapDistance;

    //////////////////////////////////////////////////////////////////////////
    // FONTS
    //////////////////////////////////////////////////////////////////////////

    static void initializeFonts();

    static wxFont* sNormalFont;
    static wxFont* sDebugFont;
    static wxFont* sTimeScaleFont;
    static wxFont* sClipDescriptionFont;
};

} // namespace

#endif // LAYOUT_H