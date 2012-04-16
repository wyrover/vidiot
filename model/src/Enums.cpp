#include "Enums.h"

#include <map>
#include <boost/assign/list_of.hpp>
#include <wx/intl.h>

namespace model {

IMPLEMENTENUM(VideoScaling);

IMPLEMENTENUM(VideoAlignment);

std::map<VideoScaling, wxString> VideoScalingConverter::mapToHumanReadibleString = boost::assign::map_list_of
    (VideoScalingFitAll,    _("Fit all"))
    (VideoScalingFitToFill, _("Fit to fill"))
    (VideoScalingNone,      _("Original size"))
    (VideoScalingCustom,    _("Custom"));

std::map<VideoAlignment,wxString> VideoAlignmentConverter::mapToHumanReadibleString = boost::assign::map_list_of
    (VideoAlignmentCenter,    _("Centered"))
    (VideoAlignmentCustom,    _("Custom"));
}