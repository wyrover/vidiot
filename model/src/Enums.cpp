#include "Enums.h"

#include <map>
#include <boost/assign/list_of.hpp>
#include <wx/intl.h>

namespace model {

IMPLEMENTENUM(VideoScaling);

IMPLEMENTENUM(VideoAlignment);

boost::bimap<VideoScaling, wxString> VideoScalingConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<VideoScaling, wxString>::relation >
    (VideoScalingFitAll,    _("Fit all"))
    (VideoScalingFitToFill, _("Fit to fill"))
    (VideoScalingNone,      _("Original size"))
    (VideoScalingCustom,    _("Custom"));

boost::bimap<VideoAlignment,wxString> VideoAlignmentConverter::mapToHumanReadibleString = boost::assign::list_of< boost::bimap<VideoAlignment,wxString>::relation >
    (VideoAlignmentCenter,    _("Centered"))
    (VideoAlignmentCustom,    _("Custom"));
}