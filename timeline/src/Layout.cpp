#include "Constants.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// TIME
//////////////////////////////////////////////////////////////////////////

const int Constants::sMicrosecondsPerSecond = 1000;
const int Constants::sMilliSecond           = 1;
const int Constants::sSecond                = 1000  * sMilliSecond;
const int Constants::sMinute                = 60    * sSecond;
const int Constants::sHour                  = 60    * sMinute;

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

const int Constants::sTimeScaleMinutesHeight                = 10;
const int Constants::sTimeScaleSecondHeight                 = 5;
const int Constants::sTimeScaleHeight                       = 20;
const int Constants::sMinimalGreyAboveVideoTracksHeight     = 10;
const int Constants::sMinimalGreyBelowAudioTracksHeight     = 10;
const int Constants::sTrackDividerHeight                    = 4;
const int Constants::sDefaultAudioVideoDividerPosition      = 100;
const int Constants::sAudioVideoDividerHeight               = 5;
const int Constants::sClipBorderSize                        = 2;
const int Constants::sVideoPosition                         = Constants::sTimeScaleHeight + Constants::sMinimalGreyAboveVideoTracksHeight;

//////////////////////////////////////////////////////////////////////////
// BRUSHES AND PENS
//////////////////////////////////////////////////////////////////////////

const wxColour	Constants::sBackgroundColour		(212,208,200);
const wxPen     Constants::sBackgroundPen           (Constants::sBackgroundColour);
const wxBrush   Constants::sBackgroundBrush         (Constants::sBackgroundColour,wxBRUSHSTYLE_SOLID);

const wxPen     Constants::sTimeScaleDividerPen     (*wxBLACK, 1);

const wxPen     Constants::sCursorPen               (*wxRED, 1);

const wxPen     Constants::sDropAreaPen             (*wxYELLOW, 1);
const wxBrush   Constants::sDropAreaBrush           (*wxYELLOW_BRUSH);

const wxPen     Constants::sTrackDividerPen         (*wxGREEN, 1);
const wxBrush   Constants::sTrackDividerBrush       (*wxGREEN,wxBRUSHSTYLE_SOLID);

const wxPen     Constants::sAudioVideoDividerPen    (*wxBLUE, 1);
const wxBrush   Constants::sAudioVideoDividerBrush  (wxColour(10,20,30),wxBRUSHSTYLE_CROSSDIAG_HATCH);

const wxPen     Constants::sClipPen                 (*wxBLACK, sClipBorderSize);
const wxBrush   Constants::sClipBrush               (wxColour(123,123,123),wxBRUSHSTYLE_SOLID);

const wxPen     Constants::sSelectedClipPen         (*wxBLACK,sClipBorderSize);
const wxBrush   Constants::sSelectedClipBrush       (wxColour(80,80,80),wxBRUSHSTYLE_SOLID);

const wxPen     Constants::sDebugPen                (*wxGREEN, 1);
const wxBrush   Constants::sDebugBrush              (*wxGREEN,wxBRUSHSTYLE_STIPPLE);

//////////////////////////////////////////////////////////////////////////
// FONTS
//////////////////////////////////////////////////////////////////////////

wxFont*   Constants::sTimeScaleFont = 0;

void Constants::initializeFonts()
{
    sTimeScaleFont = const_cast<wxFont*>(wxSMALL_FONT);
}

}} // namespace
