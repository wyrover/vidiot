#include "Layout.h"

#include "UtilLog.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

static Layout* sCurrent;

Layout::Layout()
//////////////////////////////////////////////////////////////////////////
// BRUSHES AND PENS
//////////////////////////////////////////////////////////////////////////
: BackgroundColour		         (212,208,200)
, BackgroundPen                 (BackgroundColour)
, BackgroundBrush               (BackgroundColour,wxBRUSHSTYLE_SOLID)
, DetailsViewHeaderColour       (wxColour(192,192,192))
, PreviewBoundingBoxColour      (*wxWHITE)
, PreviewBoundingBoxPen         (PreviewBoundingBoxColour, 2)
, PreviewBackgroundColour       (*wxBLACK)
, PreviewBackgroundPen          (PreviewBackgroundColour)
, PreviewBackgroundBrush        (PreviewBackgroundColour,wxBRUSHSTYLE_SOLID)
, TimelineRenderInProgressColour(*wxRED)
, TimeScaleDividerPen           (*wxBLACK, 1)
, CursorPen                     (*wxRED, 1)
, DropAreaPen                   (*wxYELLOW, 1)
, DropAreaBrush                 (*wxYELLOW_BRUSH)
, TrackDividerPen               (*wxGREEN, 1)
, TrackDividerBrush             (*wxGREEN,wxBRUSHSTYLE_SOLID)
, AudioVideoDividerPen          (*wxBLUE, 1)
, AudioVideoDividerBrush        (wxColour(10,20,30),wxBRUSHSTYLE_CROSSDIAG_HATCH)
, ClipPen                       (*wxBLACK, ClipBorderSize)
, ClipBrush                     (wxColour(123,123,123),wxBRUSHSTYLE_SOLID)
, ClipDescriptionFGColour       (*wxWHITE)
, ClipDescriptionBGColour       (wxColour(0,0,0))
, ClipDescriptionPen            (ClipDescriptionBGColour,1)
, ClipDescriptionBrush          (ClipDescriptionBGColour,wxBRUSHSTYLE_SOLID)
, SelectedClipPen               (*wxBLACK,ClipBorderSize)
, SelectedClipBrush             (wxColour(80,80,80),wxBRUSHSTYLE_SOLID)
, SnapPen                       (*wxMEDIUM_GREY_PEN)
, SnapBrush                     (*wxMEDIUM_GREY_BRUSH)
, DebugColour                   (*wxGREEN)
, DebugPen                      (DebugColour, 1)
, DebugBrush                    (DebugColour,wxBRUSHSTYLE_STIPPLE)
, TransitionColour              (wxColour(224,0,224))
, TransitionPen                 (TransitionColour, 1)
, TransitionBrush               (TransitionColour,wxBRUSHSTYLE_FDIAGONAL_HATCH)
, TransitionBgUnselected        (wxColour(123,123,123),wxBRUSHSTYLE_SOLID)
, TransitionBgSelected          (wxColour(80,80,80),wxBRUSHSTYLE_SOLID)
//////////////////////////////////////////////////////////////////////////
// FONTS
//////////////////////////////////////////////////////////////////////////
, NormalFont                    (wxFont(wxSize(0,12),wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL))
, DebugFont                     (wxFont(wxSize(0,11),wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL))
, TimeScaleFont                 (wxFont(wxSize(0,11),wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL))
, ClipDescriptionFont           (wxFont(wxSize(0,11),wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_NORMAL))
, RenderInProgressFont          (wxFont(wxSize(60,60),wxFONTFAMILY_DEFAULT,wxFONTSTYLE_NORMAL,wxFONTWEIGHT_BOLD))
{
    sCurrent = this;

    wxBitmap tmp(100,100 ); // tmp, so size not that important.
    wxMemoryDC dc(tmp);
    dc.SetFont(ClipDescriptionFont);
    ClipDescriptionBarHeight = dc.GetCharHeight() + 2;
    TransitionHeight = ClipDescriptionBarHeight + 4;
}

Layout::~Layout()
{
    sCurrent = 0;
}

// static
Layout& Layout::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
}

pixel Layout::ClipDescriptionBarHeight             = 5; // Default value.
pixel Layout::TransitionHeight                     = 8; // Default value

} // namespace