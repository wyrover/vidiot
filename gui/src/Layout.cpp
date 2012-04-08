#include "Layout.h"

#include <wx/bitmap.h>
#include <wx/dcmemory.h>

namespace gui {

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

const pixel Layout::sTimeScaleMinutesHeight               = 10;
const pixel Layout::sTimeScaleSecondHeight                = 5;
const pixel Layout::sTimeScaleHeight                      = 25;
const pixel Layout::sMinimalGreyAboveVideoTracksHeight    = 10;
const pixel Layout::sMinimalGreyBelowAudioTracksHeight    = 10;
const pixel Layout::sTrackDividerHeight                   = 4;
const pixel Layout::sDefaultAudioVideoDividerPosition     = 100;
const pixel Layout::sAudioVideoDividerHeight              = 5;
const pixel Layout::sMinTrackHeight                       = 10;
const pixel Layout::sMaxTrackHeight                       = 100;
const pixel Layout::sClipBorderSize                       = 2;
const pixel Layout::sVideoPosition                        = Layout::sTimeScaleHeight + Layout::sMinimalGreyAboveVideoTracksHeight;
      pixel Layout::sClipDescriptionBarHeight             = 5; // Default value.
      pixel Layout::sTransitionHeight                     = 8; // Default value
const pixel Layout::sDragThreshold                        = 2;

//////////////////////////////////////////////////////////////////////////
// BRUSHES AND PENS
//////////////////////////////////////////////////////////////////////////

const wxColour	Layout::sBackgroundColour		 (212,208,200);
const wxPen     Layout::sBackgroundPen           (Layout::sBackgroundColour);
const wxBrush   Layout::sBackgroundBrush         (Layout::sBackgroundColour,wxBRUSHSTYLE_SOLID);

const wxColour	Layout::sPreviewBoundingBoxColour(*wxWHITE);
const wxPen     Layout::sPreviewBoundingBoxPen   (Layout::sPreviewBoundingBoxColour, 2);

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

const wxColour  Layout::sTransitionColour        (wxColour(224,0,224));
const wxPen     Layout::sTransitionPen           (Layout::sTransitionColour, 1);
const wxBrush   Layout::sTransitionBrush         (Layout::sTransitionColour,wxBRUSHSTYLE_FDIAGONAL_HATCH);
const wxBrush   Layout::sTransitionBgUnselected  (wxColour(123,123,123),wxBRUSHSTYLE_SOLID);
const wxBrush   Layout::sTransitionBgSelected    (wxColour(80,80,80),wxBRUSHSTYLE_SOLID);

//////////////////////////////////////////////////////////////////////////
// SNAP
//////////////////////////////////////////////////////////////////////////

const int       Layout::sSnapDistance           = 50;
const int       Layout::sCursorClipEditDistance = 6;

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
    sTransitionHeight = sClipDescriptionBarHeight + 4;
}

} // namespace