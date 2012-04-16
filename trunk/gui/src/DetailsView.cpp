#include "DetailsView.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "UtilLog.h"
#include "IClip.h"
#include "DetailsViewClip.h"
#include "UtilLogWxwidgets.h"
#include "Layout.h"
#include "Window.h"

namespace gui {

static DetailsView* sCurrent = 0;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DetailsView::DetailsView(wxWindow* parent)
    :   wxPanel(parent)
{
    LOG_INFO;

    sCurrent = this;
}

DetailsView::~DetailsView()
{
    sCurrent = 0;
}

DetailsView& DetailsView::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// SET THE CURRENTLY FOCUSED ITEM
//////////////////////////////////////////////////////////////////////////

void DetailsView::focus(model::IClipPtr clip)
{
    DestroyChildren();
    reset(clip->getDescription(), new DetailsViewClip(this,clip));
    VAR_INFO(GetSize());
}

void DetailsView::focus(model::IClips clips)
{
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////
// todo wxauimanager::SavePerspective

void DetailsView::reset(wxString title, wxWindow* details)
{
    VAR_INFO(GetSize());

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* header = new wxStaticText(this,wxID_ANY,title, wxDefaultPosition, wxSize(GetSize().GetWidth(),-1), wxBORDER_THEME | wxST_ELLIPSIZE_MIDDLE | wxALIGN_CENTRE);
    header->SetBackgroundColour(Layout::sDetailsViewHeaderColour);
    sizer->Add(header, wxSizerFlags(0).Center());
    sizer->Add(details, wxSizerFlags(1).Expand().Center() );
    SetSizer(sizer);

    gui::Window::get().triggerLayout();
}

} // namespace