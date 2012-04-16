#include "DetailsViewClip.h"

#include <boost/foreach.hpp>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/choice.h>
//#include <wx/gbsizer.h>
#include "IClip.h"
#include "VideoClip.h"
#include "UtilEnumSelector.h"
#include "AudioClip.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include <wx/statbox.h>

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DetailsViewClip::DetailsViewClip(wxWindow* parent, model::IClipPtr clip)
    :   wxPanel(parent)
    ,   mClip(clip)
    ,   mTopSizer(0)
    ,   mBoxSizer(0)
    ,   mSelectScaling(0)
    ,   mSelectAlignment(0)
{
    LOG_INFO;

    VAR_INFO(GetSize());

    mTopSizer = new wxBoxSizer(wxVERTICAL);

    model::IClipPtr link = clip->getLink();
    model::VideoClipPtr videoclip = boost::dynamic_pointer_cast<model::VideoClip>(clip);
    if (!videoclip)
    {
        videoclip = boost::dynamic_pointer_cast<model::VideoClip>(link);
    }
    model::AudioClipPtr audioclip = boost::dynamic_pointer_cast<model::AudioClip>(clip);
    if (!audioclip)
    {
        audioclip = boost::dynamic_pointer_cast<model::AudioClip>(link);
    }

    if (videoclip)
    {
        addbox(_("Video"));

        mSelectScaling = new EnumSelector<model::VideoScaling>(this, model::VideoScalingConverter::mapToHumanReadibleString, videoclip->getScaling());
        addoption(_("Scaling"), mSelectScaling);

        mSelectAlignment = new EnumSelector<model::VideoAlignment>(this, model::VideoAlignmentConverter::mapToHumanReadibleString, videoclip->getAlignment());
        addoption(_("Alignment"), mSelectAlignment);

// todo remember:        value = mSelectScaling->getValue();
    }

    if (audioclip)
    {
        addbox(_("Audio"));
        addoption(_("test"), new wxButton(this,wxID_ANY,_("label")));
    }

    mTopSizer->AddStretchSpacer();
    SetSizerAndFit(mTopSizer);
    VAR_INFO(GetSize());
}

DetailsViewClip::~DetailsViewClip()
{
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DetailsViewClip::addbox(const wxString& name)
{
    ASSERT(mTopSizer);
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, name), wxVERTICAL );
    mTopSizer->Add(mBoxSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, 0 );
}

void DetailsViewClip::addoption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 2);
    hSizer->Add(new wxStaticText(this, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 0);
    hSizer->Add(5, 5, 1, wxALL, 0);
    hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP);
}

} // namespace