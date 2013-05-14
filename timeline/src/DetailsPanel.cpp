#include "DetailsPanel.h"

#include "Details.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h" //todo remove

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

DetailsPanel::~DetailsPanel()
{
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
    wxStaticBoxSizer* staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, name);
    mBoxSizer = new wxFlexGridSizer(2,0,0);
    ((wxFlexGridSizer*)mBoxSizer)->AddGrowableCol(1);
    staticBoxSizer->Add(mBoxSizer, wxSizerFlags(0).Expand() );
    mTopSizer->Add(staticBoxSizer, wxSizerFlags(0).Expand() );
    mTopSizer->Layout();
}

wxSizer* DetailsPanel::addoption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxSizer* hSizer = mBoxSizer;
    mBoxSizer->Add(new wxStaticText(this, wxID_ANY, name, wxDefaultPosition, wxSize(100,-1)), wxSizerFlags(0).Top().Left());//, 0, wxALL|wxALIGN_TOP, 0);
    mBoxSizer->Add(widget, wxSizerFlags(1).Expand());//, wxALIGN_TOP);//, wxEXPAND | wxRIGHT | wxALIGN_TOP);
    VAR_ERROR(mBoxSizer->GetSize());
    return mBoxSizer;// todo remove this current use will not work anyway...
}

}} // namespace