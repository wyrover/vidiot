#ifndef LAYOUT_H
#define LAYOUT_H

#include <wx/font.h>
#include <wx/pen.h>
#include <wx/brush.h>
#include <wx/colour.h>
#include "UtilInt.h"

namespace gui {

/// Fonts/brushes/pens may not be initialized statically (since wxWidgets is not prepared yet).
/// (leads to uninitialized wxStockGDI, for example)
class Layout
{
public:

    Layout();
    virtual ~Layout();
    static Layout& get();

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    static const pixel TimeScaleMinutesHeight               = 10;
    static const pixel TimeScaleSecondHeight                = 5;
    static const pixel TimeScaleHeight                      = 25;
    static const pixel MinimalGreyAboveVideoTracksHeight    = 10;
    static const pixel MinimalGreyBelowAudioTracksHeight    = 10;
    static const pixel TrackDividerHeight                   = 4;
    static const pixel DefaultAudioVideoDividerPosition     = 100;
    static const pixel AudioVideoDividerHeight              = 5;
    static const pixel MinTrackHeight                       = 10;
    static const pixel MaxTrackHeight                       = 100;
    static const pixel ClipBorderSize                       = 2;
    static const pixel VideoPosition                        = TimeScaleHeight + MinimalGreyAboveVideoTracksHeight;
    static const pixel DragThreshold                        = 2;
    static const pixel SnapDistance                         = 50;
    static const pixel CursorClipEditDistance               = 6;
    static       pixel ClipDescriptionBarHeight;
    static       pixel TransitionHeight;

    //////////////////////////////////////////////////////////////////////////
    // BRUSHES AND PENS
    //////////////////////////////////////////////////////////////////////////

    const wxColour BackgroundColour;
    const wxPen BackgroundPen;
    const wxBrush BackgroundBrush;
    const wxColour DetailsViewHeaderColour;
    const wxColour PreviewBoundingBoxColour;
    const wxPen PreviewBoundingBoxPen;
    const wxColour PreviewBackgroundColour;
    const wxPen PreviewBackgroundPen;
    const wxBrush PreviewBackgroundBrush;
    const wxColour TimelineRenderInProgressColour;
    const wxPen TimeScaleDividerPen;
    const wxPen CursorPen;
    const wxPen DropAreaPen;
    const wxBrush DropAreaBrush;
    const wxPen TrackDividerPen;
    const wxBrush TrackDividerBrush;
    const wxPen AudioVideoDividerPen;
    const wxBrush AudioVideoDividerBrush;
    const wxPen ClipPen;
    const wxBrush ClipBrush;
    const wxColour ClipDescriptionFGColour;
    const wxColour ClipDescriptionBGColour;
    const wxPen ClipDescriptionPen;
    const wxBrush ClipDescriptionBrush;
    const wxPen SelectedClipPen;
    const wxBrush SelectedClipBrush;
    const wxPen SnapPen;
    const wxBrush SnapBrush;
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

#endif // LAYOUT_H