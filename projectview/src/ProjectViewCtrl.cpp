#include "ProjectViewCtrl.h"

#include "ProjectViewModel.h"
#include "UtilLog.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewCtrl::ProjectViewCtrl(wxWindow* parent)
    :   wxDataViewCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE | wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES)
{
    LOG_INFO;
}

ProjectViewCtrl::~ProjectViewCtrl()
{
}

} // namespace