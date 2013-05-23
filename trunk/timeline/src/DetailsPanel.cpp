#include "DetailsPanel.h"

#include "Details.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

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

void DetailsPanel::addBox(const wxString& name)
{
    ASSERT(mTopSizer);
    wxStaticBoxSizer* staticBoxSizer = new wxStaticBoxSizer(wxVERTICAL, this, name);
    mBoxSizer = new wxFlexGridSizer(2,0,0);
    ((wxFlexGridSizer*)mBoxSizer)->AddGrowableCol(1);
    staticBoxSizer->Add(mBoxSizer, wxSizerFlags(0).Expand() );
    mTopSizer->Add(staticBoxSizer, wxSizerFlags(0).Expand() );
    mTopSizer->Layout();
    mBoxes[name] = staticBoxSizer;
}

void DetailsPanel::showBox(const wxString& name, bool show)
{
    ASSERT_MAP_CONTAINS(mBoxes,name);
    mTopSizer->Show(mBoxes[name], show);
}

void DetailsPanel::addOption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxStaticText* title = new wxStaticText(this, wxID_ANY, name, wxDefaultPosition, wxSize(100,-1));
    mBoxSizer->Add(title, wxSizerFlags(0).Top().Left());//, 0, wxALL|wxALIGN_TOP, 0);
    mBoxSizer->Add(widget, wxSizerFlags(1).Expand());

    mMapWindowToSizer[widget] = mBoxSizer;
    mMapWindowToTitle[widget] = title;
}

void DetailsPanel::showOption(wxWindow* widget, bool show)
{
    ASSERT_MAP_CONTAINS(mMapWindowToSizer,widget);
    ASSERT_MAP_CONTAINS(mMapWindowToTitle,widget);
    mMapWindowToSizer[widget]->Show(widget,show);
    mMapWindowToSizer[widget]->Show(mMapWindowToTitle[widget],show);
}

}} // namespace