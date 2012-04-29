#include "Details.h"

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "UtilLog.h"
#include "IClip.h"
#include "DetailsClip.h"
#include "UtilLogWxwidgets.h"
#include "Layout.h"
#include "Window.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Details::Details(wxWindow* parent, Timeline* timeline)
    :   wxPanel(parent)
    ,   Part(timeline)
{
    LOG_INFO;
}

Details::~Details()
{
}

void Details::onHide()
{
}

//////////////////////////////////////////////////////////////////////////
// SET THE CURRENTLY FOCUSED ITEM
//////////////////////////////////////////////////////////////////////////

void Details::focus(model::IClipPtr clip)
{
    DestroyChildren();
    reset(clip->getDescription(), new DetailsClip(this,getTimeline(),clip));
    VAR_INFO(GetSize());
}

void Details::focus(model::IClips clips)
{
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////
// todo wxauimanager::SavePerspective

void Details::reset(wxString title, wxWindow* details)
{
    VAR_INFO(GetSize());

    // todo use mechanism similar to Preview to be able to monitor multiple timelines

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* header = new wxStaticText(this,wxID_ANY,title, wxDefaultPosition, wxSize(GetSize().GetWidth(),-1), wxBORDER_THEME | wxST_ELLIPSIZE_MIDDLE | wxALIGN_CENTRE);
    header->SetBackgroundColour(Layout::sDetailsViewHeaderColour);
    sizer->Add(header, wxSizerFlags(0).Center());
    sizer->Add(details, wxSizerFlags(1).Expand().Center() );
    SetSizer(sizer);

    gui::Window::get().triggerLayout();
}

}} // namespace