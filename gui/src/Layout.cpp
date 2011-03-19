#include "Layout.h"

#include <wx/bitmap.h>
#include <wx/dcmemory.h>

namespace gui {

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

const int Layout::sTimeScaleMinutesHeight               = 10;
const int Layout::sTimeScaleSecondHeight                = 5;
const int Layout::sTimeScaleHeight                      = 20;
const int Layout::sMinimalGreyAboveVideoTracksHeight    = 10;
const int Layout::sMinimalGreyBelowAudioTracksHeight    = 10;
const int Layout::sTrackDividerHeight                   = 4;
const int Layout::sDefaultAudioVideoDividerPosition     = 100;
const int Layout::sAudioVideoDividerHeight              = 5;
const int Layout::sMinTrackHeight                       = 10;
const int Layout::sMaxTrackHeight                       = 100;
const int Layout::sClipBorderSize                       = 2;
const int Layout::sVideoPosition                        = Layout::sTimeScaleHeight + Layout::sMinimalGreyAboveVideoTracksHeight;
int Layout::sClipDescriptionBarHeight                           = 5; // Default value.

//////////////////////////////////////////////////////////////////////////
// BRUSHES AND PENS
//////////////////////////////////////////////////////////////////////////

const wxColour	Layout::sBackgroundColour		(212,208,200);
const wxPen     Layout::sBackgroundPen           (Layout::sBackgroundColour);
const wxBrush   Layout::sBackgroundBrush         (Layout::sBackgroundColour,wxBRUSHSTYLE_SOLID);

const wxColour	Layout::sPreviewBackgroundColour (*wxBLACK);
const wxPen     Layout::sPreviewBackgroundPen    (Layout::sPreviewBackgroundColour);
const wxBrush   Layout::sPreviewBackgroundBrush  (Layout::sPreviewBackgroundColour,wxBRUSHSTYLE_SOLID);

const wxPen     Layout::sTimeScaleDividerPen     (*wxBLACK, 1);

const wxPen     Layout::sCursorPen               (*wxRED, 1);

const wxPen     Layout::sDropAreaPen             (*wxYELLOW, 1);
const wxBrush   Layout::sDropAreaBrush           (*wxYELLOW_BRUSH);

const wxPen     Layout::sTrackDividerPen         (*wxGREEN, 1);
const wxBrush   Layout::sTrackDividerBrush       (*wxGREEN,wxBRUSHSTYLE_SOLID);

const wxPen     Layout::sAudioVideoDividerPen    (*wxBLUE, 1);
const wxBrush   Layout::sAudioVideoDividerBrush  (wxColour(10,20,30),wxBRUSHSTYLE_CROSSDIAG_HATCH);

const wxPen     Layout::sClipPen                 (*wxBLACK, sClipBorderSize);
const wxBrush   Layout::sClipBrush               (wxColour(123,123,123),wxBRUSHSTYLE_SOLID);

const wxColour  Layout::sClipDescriptionFGColour (*wxWHITE);
const wxColour  Layout::sClipDescriptionBGColour (wxColour(0,0,0));
const wxPen     Layout::sClipDescriptionPen      (Layout::sClipDescriptionBGColour,1);
const wxBrush   Layout::sClipDescriptionBrush    (Layout::sClipDescriptionBGColour,wxBRUSHSTYLE_SOLID);

const wxPen     Layout::sSelectedClipPen         (*wxBLACK,sClipBorderSize);
const wxBrush   Layout::sSelectedClipBrush       (wxColour(80,80,80),wxBRUSHSTYLE_SOLID);

const wxPen     Layout::sSnapPen                 (*wxMEDIUM_GREY_PEN);
const wxBrush   Layout::sSnapBrush               (*wxMEDIUM_GREY_BRUSH);

const wxColour  Layout::sDebugColour             (*wxGREEN);
const wxPen     Layout::sDebugPen                (Layout::sDebugColour, 1);
const wxBrush   Layout::sDebugBrush              (Layout::sDebugColour,wxBRUSHSTYLE_STIPPLE);

//////////////////////////////////////////////////////////////////////////
// SNAP
//////////////////////////////////////////////////////////////////////////

const int       Layout::sSnapDistance           = 50;

//////////////////////////////////////////////////////////////////////////
// FONTS
//////////////////////////////////////////////////////////////////////////

wxFont*   Layout::sNormalFont           = 0;
wxFont*   Layout::sDebugFont            = 0;
wxFont*   Layout::sTimeScaleFont        = 0;
wxFont*   Layout::sClipDescriptionFont  = 0;

void Layout::initializeFonts()
{
    sNormalFont             = const_cast<wxFont*>(wxNORMAL_FONT);
    sDebugFont              = const_cast<wxFont*>(wxSMALL_FONT);
    sTimeScaleFont          = const_cast<wxFont*>(wxSMALL_FONT);
    sClipDescriptionFont    = const_cast<wxFont*>(wxSMALL_FONT);

    wxBitmap tmp(100,100 ); // tmp, so size not that important.
    wxMemoryDC dc(tmp);
    dc.SetFont(*sClipDescriptionFont);
    sClipDescriptionBarHeight = dc.GetCharHeight() + 2;

}

} // namespace
