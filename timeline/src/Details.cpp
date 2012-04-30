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
    ,   mCurrent(0)
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

void Details::focus()
{
    DestroyChildren();
    reset(_(""),0);
    VAR_INFO(GetSize());
}

void Details::focus(model::IClipPtr clip)
{
    DestroyChildren(); // Must be called before creating a new child
    reset(clip->getDescription(), new DetailsClip(this,getTimeline(),clip));
    VAR_INFO(GetSize());
}

void Details::focus(model::IClips clips)
{
}

wxWindow* Details::getCurrent() const
{
    return mCurrent;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////
// todo wxauimanager::SavePerspective

void Details::reset(wxString title, wxWindow* details)
{
    VAR_INFO(GetSize());

    mCurrent = details;

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticText* header = new wxStaticText(this,wxID_ANY,title, wxDefaultPosition, wxSize(GetSize().GetWidth(),-1), wxBORDER_THEME | wxST_ELLIPSIZE_MIDDLE | wxALIGN_CENTRE);
    header->SetBackgroundColour(Layout::sDetailsViewHeaderColour);
    sizer->Add(header, wxSizerFlags(0).Center());
    if (details)
    {
        sizer->Add(details, wxSizerFlags(1).Expand().Center() );
    }
    SetSizer(sizer);

    gui::Window::get().triggerLayout();
}

}} // namespace