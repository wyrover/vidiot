// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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
const pixel Layout::MaxTrackHeight                       = 250;
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
, TimeScaleBackgroundBrush      (*wxWHITE,wxBRUSHSTYLE_SOLID) // todo make getter methods for all these gdi objects that fully create the gdi objects. Do not rely on stockgdi objects as that will cause multithreading issues (rare crashes)
, TimeScaleDividerPen           (*wxBLACK, 1)
, TimeScaleFontColour           (*wxBLACK)
, CursorPen                     (*wxRED, 1)
, DropAreaPen                   (*wxYELLOW, 1)
, DropAreaBrush                 (*wxYELLOW_BRUSH)
, DividerBackgroundColour       (wxColour(132,132,132))
, DividerPen                    (wxColour(64,64,64),1)
, DividerBrush                  (DividerBackgroundColour,wxBRUSHSTYLE_SOLID)
, ClipBorderColour              (32,32,32)
, ClipPen                       (ClipBorderColour, ClipBorderSize)
, ClipBrush                     (wxColour(160,160,160),wxBRUSHSTYLE_SOLID)
, ClipDescriptionFGColour       (*wxWHITE)
, ClipDescriptionBGColour       (ClipBorderColour)
, ClipDescriptionPen            (ClipDescriptionBGColour,1)
, ClipDescriptionBrush          (ClipDescriptionBGColour,wxBRUSHSTYLE_SOLID)
, SelectedClipPen               (ClipBorderColour,ClipBorderSize)
, SelectedClipBrush             (wxColour(80,80,80),wxBRUSHSTYLE_SOLID)
, AudioPeaksPen                 (wxColour{ 87, 120, 74 })
, SnapPen                       (*wxMEDIUM_GREY_PEN)
, IntervalPen                   (*wxGREY_PEN)
, IntervalBrush                 (*wxLIGHT_GREY,wxBRUSHSTYLE_CROSSDIAG_HATCH)
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
    wxBitmap tmp(100,100 ); // tmp, so size not that important.
    wxMemoryDC dc(tmp);
    dc.SetFont(ClipDescriptionFont);
    ClipDescriptionBarHeight = dc.GetCharHeight() + 2;
    TransitionHeight = ClipDescriptionBarHeight + 4;
}

Layout::~Layout()
{
}

} // namespace
