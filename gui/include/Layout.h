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

#ifndef LAYOUT_H
#define LAYOUT_H

#include "UtilInt.h"
#include "UtilSingleInstance.h"

namespace gui {

/// Fonts/brushes/pens may not be initialized statically (since wxWidgets is not prepared yet).
/// (leads to uninitialized wxStockGDI, for example)
class Layout : public SingleInstance<Layout>
{
public:

    Layout();
    virtual ~Layout();

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    static const pixel TimeScaleMinutesHeight;
    static const pixel TimeScaleSecondHeight;
    static const pixel TimeScaleHeight;
    static const pixel MinimalGreyAboveVideoTracksHeight;
    static const pixel MinimalGreyBelowAudioTracksHeight;
    static const pixel TrackDividerHeight;
    static const pixel DefaultAudioVideoDividerPosition;
    static const pixel AudioVideoDividerHeight;
    static const pixel MinTrackHeight;
    static const pixel MaxTrackHeight;
    static const pixel ClipBorderSize;
    static const pixel VideoPosition;
    static const pixel DragThreshold;
    static const pixel SnapDistance;
    static const pixel CursorClipEditDistance;
    static       pixel ClipDescriptionBarHeight;
    static       pixel TransitionHeight;

    //////////////////////////////////////////////////////////////////////////
    // BRUSHES AND PENS
    //////////////////////////////////////////////////////////////////////////

    const wxColour BackgroundColour;
    const wxPen BackgroundPen;
    const wxBrush BackgroundBrush;
    const wxColour DetailsViewHeaderColour;
    const wxColour PreviewBackgroundColour;
    const wxPen PreviewBackgroundPen;
    const wxBrush PreviewBackgroundBrush;
    const wxColour TimelineRenderInProgressColour;
    const wxBrush TimeScaleBackgroundBrush;
    const wxPen TimeScaleDividerPen;
    const wxColour TimeScaleFontColour;
    const wxPen CursorPen;
    const wxPen DropAreaPen;
    const wxBrush DropAreaBrush;
    const wxColour DividerBackgroundColour;
    const wxPen DividerPen;
    const wxBrush DividerBrush;
    const wxColour ClipBorderColour;
    const wxPen ClipPen;
    const wxBrush ClipBrush;
    const wxColour ClipDescriptionFGColour;
    const wxColour ClipDescriptionBGColour;
    const wxPen ClipDescriptionPen;
    const wxBrush ClipDescriptionBrush;
    const wxPen SelectedClipPen;
    const wxBrush SelectedClipBrush;
    const wxPen AudioPeaksPen;
    const wxPen SnapPen;
    const wxPen IntervalPen;
    const wxBrush IntervalBrush;
    const wxColour DebugColour;
    const wxPen DebugPen;
    const wxBrush DebugBrush;
    const wxColour TransitionColour;
    const wxPen TransitionPen;
    const wxBrush TransitionBrush;
    const wxBrush TransitionBgUnselected;
    const wxBrush TransitionBgSelected;

    //////////////////////////////////////////////////////////////////////////
    // FONTS
    //////////////////////////////////////////////////////////////////////////

    const wxFont NormalFont;
    const wxFont DebugFont;
    const wxFont TimeScaleFont;
    const wxFont ClipDescriptionFont;
    const wxFont RenderInProgressFont;
};

} // namespace

#endif
