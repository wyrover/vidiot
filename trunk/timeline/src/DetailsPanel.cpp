#include "DetailsPanel.h"

#include "Details.h"
#include "UtilLog.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DetailsPanel::DetailsPanel(wxWindow* parent, Timeline& timeline)
    :   wxPanel(parent)
    ,   Part(&timeline)
    ,   mShow(false)
    ,   mTitle("")
    ,   mTopSizer(0)
    ,   mBoxSizer(0)
{
    mTopSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(mTopSizer);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool DetailsPanel::requestsToBeShown() const
{
    return mShow;
}

wxString DetailsPanel::getTitle() const
{
    return mTitle;
}

//////////////////////////////////////////////////////////////////////////
// INTERFACE TOWARDS ACTUAL DETAILS VIEWS
//////////////////////////////////////////////////////////////////////////

void DetailsPanel::requestShow(bool show, wxString title)
{
    VAR_DEBUG(this)(show)(title);
    mShow = show;
    mTitle = title;
    static_cast<Details*>(GetParent())->update();
}

void DetailsPanel::addbox(const wxString& name)
{
    ASSERT(mTopSizer);
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, name), wxVERTICAL );
    mTopSizer->Add(mBoxSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, 0 );
}

wxSizer* DetailsPanel::addoption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 2);
    hSizer->Add(new wxStaticText(this, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 0);
    hSizer->Add(5, 5, 1, wxALL, 0);
    hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP);
    return hSizer;
}

}} // namespace