#include "Details.h"

#include "DetailsClip.h"
#include "DetailsTrim.h"
#include "IClip.h"
#include "Layout.h"
#include "Sequence.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "Window.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Details::Details(wxWindow* parent, Timeline* timeline)
    :   wxPanel(parent)
    ,   Part(timeline)
    ,   mDetails()
{
    LOG_INFO;

    // The order in this list is the order of priority in case two panels want to be shown
    mDetails = boost::assign::list_of
        (static_cast<IDetails*>(new DetailsTrim(this,*timeline)))
        (static_cast<IDetails*>(new DetailsClip(this,*timeline)));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    mHeader = new wxStaticText(this,wxID_ANY,"", wxDefaultPosition, wxSize(2000,-1), wxBORDER_THEME | wxST_ELLIPSIZE_MIDDLE | wxALIGN_CENTRE);
    mHeader->SetBackgroundColour(Layout::get().DetailsViewHeaderColour);
    sizer->Add(mHeader, wxSizerFlags(0).Center());
    BOOST_FOREACH( IDetails* details, mDetails )
    {
        sizer->Add(details, wxSizerFlags(1).Expand().Center() );
    }
    SetSizer(sizer);
    update();
}

Details::~Details()
{
    mDetails.clear();
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

void Details::update()
{
    bool shown = false;
    BOOST_FOREACH(IDetails* details, mDetails)
    {
        if (details->requestsToBeShown() && !shown)
        {
            mCurrent = details;
            mHeader->SetLabel(details->getTitle());
            GetSizer()->Show(details);
            shown = true;
        }
        else
        {
            GetSizer()->Hide(details);
        }
    }
    if (!shown)
    {
        mCurrent = 0;
        mHeader->SetLabel("");
    }
    gui::Window::get().triggerLayout();
}

}} // namespace