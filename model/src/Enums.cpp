#include "Enums.h"

#include "Config.h"

namespace model {

IMPLEMENTENUM(VideoScaling);

IMPLEMENTENUM(VideoAlignment);

IMPLEMENTENUM(TransitionType);

boost::bimap<VideoScaling, wxString> VideoScalingConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<VideoScaling, wxString>::relation >
    (VideoScalingFitAll,    _("Fit all"))
    (VideoScalingFitToFill, _("Fit to fill"))
    (VideoScalingNone,      _("Original size"))
    (VideoScalingCustom,    _("Custom"));

boost::bimap<VideoAlignment,wxString> VideoAlignmentConverter::mapToHumanReadibleString = boost::assign::list_of< boost::bimap<VideoAlignment,wxString>::relation >
    (VideoAlignmentCenter,              _("Centered"))
    (VideoAlignmentCenterHorizontal,    _("Centered horizontally"))
    (VideoAlignmentCenterVertical,      _("Centered vertically"))
    (VideoAlignmentCustom,              _("Custom"));
}