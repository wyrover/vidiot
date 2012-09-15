#include "DetailsTrim.h"

#include "IClip.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DetailsTrim::DetailsTrim(Details* parent, Timeline& timeline)
    :   IDetails(parent,timeline)
    ,   mTopSizer(0)
    ,   mBoxSizer(0)
{
    LOG_INFO;

    VAR_INFO(GetSize());

    mTopSizer = new wxBoxSizer(wxVERTICAL);

    addbox(_("Resizing clip"));

    wxPanel* lengthpanel = new wxPanel(this);
    wxBoxSizer* opacitysizer = new wxBoxSizer(wxHORIZONTAL);
    mOldLength = new wxSpinCtrl(lengthpanel);
    mOldLength->SetRange(0,std::numeric_limits<int>::max());
    mOldLength->SetValue(0);
    addoption(_("Original length"), mOldLength);
    mNewLength = new wxSpinCtrl(lengthpanel);
    mNewLength->SetRange(0,std::numeric_limits<int>::max());
    mNewLength->SetValue(0);
    addoption(_("Original length"), mNewLength);

    // todo add 'adjust' info

    addbox(_("Notes"));
    wxPanel* notespanel = new wxPanel(this);
    wxStaticText* statictext = new wxStaticText(notespanel, wxID_ANY, _("Press SHIFT to remove blank areas during the trimming"));

    mTopSizer->AddStretchSpacer();
    SetSizerAndFit(mTopSizer);

    getTrim().Bind(EVENT_TRIM_UPDATE, &DetailsTrim::onTrimChanged, this);
    VAR_INFO(GetSize());
}

DetailsTrim::~DetailsTrim()
{
    getTrim().Unbind(EVENT_TRIM_UPDATE, &DetailsTrim::onTrimChanged, this);
}

//////////////////////////////////////////////////////////////////////////
// TRIM EVENTS
//////////////////////////////////////////////////////////////////////////

void DetailsTrim::onTrimChanged( timeline::EventTrimUpdate& event )
{
    mOldLength->SetValue(event.getValue().oldLength);
    mNewLength->SetValue(event.getValue().newLength);
    if (event.getValue().active)
    {
        requestShow(true, event.getValue().description);
    }
    else
    {
        requestShow(false);
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DetailsTrim::addbox(const wxString& name)
{
    ASSERT(mTopSizer);
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, name), wxVERTICAL );
    mTopSizer->Add(mBoxSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, 0 );
}

void DetailsTrim::addoption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 2);
    hSizer->Add(new wxStaticText(this, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 0);
    hSizer->Add(5, 5, 1, wxALL, 0);
    hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP);
}

}} // namespace