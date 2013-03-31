#include "Layout.h"

#include "UtilLog.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

const pixel Layout::TimeScaleMinutesHeight               = 10;
const pixel Layout::TimeScaleSecondHeight                = 5;
const pixel Layout::TimeScaleHeight                      = 25;
const pixel Layout::MinimalGreyAboveVideoTracksHeight    = 10;
const pixel Layout::MinimalGreyBelowAudioTracksHeight    = 10;
const pixel Layout::TrackDividerHeight                   = 4;
const pixel Layout::DefaultAudioVideoDividerPosition     = 100;
const pixel Layout::AudioVideoDividerHeight              = 6;
const pixel Layout::MinTrackHeight                       = 10;
const pixel Layout::MaxTrackHeight                       = 100;
const pixel Layout::ClipBorderSize                       = 2;
const pixel Layout::VideoPosition                        = TimeScaleHeight + MinimalGreyAboveVideoTracksHeight;
const pixel Layout::DragThreshold                        = 2;
const pixel Layout::SnapDistance                         = 50;
const pixel Layout::CursorClipEditDistance               = 6;
      pixel Layout::ClipDescriptionBarHeight             = 5; // Default value.
      pixel Layout::TransitionHeight                     = 8; // Default value

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

static Layout* sCurrent;

Layout::Layout()
//////////////////////////////////////////////////////////////////////////
// BRUSHES AND PENS
//////////////////////////////////////////////////////////////////////////
: BackgroundColour		        (212,208,200)
, BackgroundPen                 (BackgroundColour)
, BackgroundBrush               (BackgroundColour,wxBRUSHSTYLE_SOLID)
, DetailsViewHeaderColour       (wxColour(192,192,192))
, PreviewBackgroundColour       (*wxBLACK)
, PreviewBackgroundPen          (PreviewBackgroundColour)
, PreviewBackgroundBrush        (PreviewBackgroundColour,wxBRUSHSTYLE_SOLID)
, TimelineRenderInProgressColour(*wxRED)
, TimeScaleDividerPen           (*wxBLACK, 1)
, CursorPen                     (*wxRED, 1)
, DropAreaPen                   (*wxYELLOW, 1)
, DropAreaBrush                 (*wxYELLOW_BRUSH)
, DividerPen                    (wxColour(64,64,64),1)
, DividerBrush                  (wxColour(132,132,132),wxBRUSHSTYLE_SOLID)
, ClipBorderColour              (32,32,32)
, ClipPen                       (ClipBorderColour, ClipBorderSize)
, ClipBrush                     (wxColour(160,160,160),wxBRUSHSTYLE_SOLID)
, ClipDescriptionFGColour       (*wxWHITE)
, ClipDescriptionBGColour       (ClipBorderColour)
, ClipDescriptionPen            (ClipDescriptionBGColour,1)
, ClipDescriptionBrush          (ClipDescriptionBGColour,wxBRUSHSTYLE_SOLID)
, SelectedClipPen               (ClipBorderColour,ClipBorderSize)
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
    ASSERT(wxThread::IsMain()); // To avoid threading issues with GDI objects
    return *sCurrent;
}

} // namespace