#include "Details.h"

#include <wx/button.h>
#include <wx/sizer.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "DetailsClip.h"
#include "IClip.h"
#include "Layout.h"
#include "Sequence.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "Window.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Details::Details(wxWindow* parent, Timeline* timeline)
    :   wxPanel(parent)
    ,   Part(timeline)
    ,   mCurrent(0)
    ,   mDetailsClip(new DetailsClip(this,*timeline))
{
    LOG_INFO;

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    mHeader = new wxStaticText(this,wxID_ANY,"", wxDefaultPosition, wxSize(2000,-1), wxBORDER_THEME | wxST_ELLIPSIZE_MIDDLE | wxALIGN_CENTRE);
    mHeader->SetBackgroundColour(Layout::sDetailsViewHeaderColour);
    sizer->Add(mHeader, wxSizerFlags(0).Center());
    sizer->Add(mDetailsClip, wxSizerFlags(1).Expand().Center() );
    sizer->Hide(mDetailsClip);
    SetSizer(sizer);
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

void Details::onSelectionChanged()
{
    std::set<model::IClipPtr> selection = getSequence()->getSelectedClips();
    if (selection.size() == 1)
    {
        focus(*selection.begin());
    }
    else if (selection.size() == 2)
    {
        model::IClipPtr a = *selection.begin();
        model::IClipPtr b = *(++(selection.begin()));
        if (a->getLink() == b)
        {
            ASSERT_EQUALS(b->getLink(),a);
            focus(a); // Any of the two will do, since DetailsClip handles linked clips.
        }
    }
    else
    {
        // Reset details view
        reset();
    }
}

//////////////////////////////////////////////////////////////////////////
// TEST
//////////////////////////////////////////////////////////////////////////

wxWindow* Details::getCurrent() const
{
    return mCurrent;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Details::focus(model::IClipPtr clip)
{
    VAR_INFO(GetSize());
    if (mDetailsClip->getClip() != clip)
    {
        mDetailsClip->setClip(clip);
        reset(clip->getDescription(), mDetailsClip);
    }
    VAR_INFO(GetSize());
}

void Details::focus(model::IClips clips)
{
}

void Details::reset(wxString title, wxWindow* details)
{
    VAR_INFO(GetSize());

    mHeader->SetLabel(title);

    if (mCurrent != details)
    {
        if (mCurrent)
        {
            GetSizer()->Hide(mCurrent);
        }
        mCurrent = details;
        if (details)
        {
            GetSizer()->Show(details);
        }
        gui::Window::get().triggerLayout();
    }

    //Fit();

}

}} // namespace