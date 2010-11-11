#include "GuiTimelinesView.h"

#include <wx/sizer.h>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "GuiMain.h"
#include "GuiPreview.h"
#include "GuiTimeLine.h"
#include "Menu.h"
#include "GuiWindow.h"
#include "AProjectViewNode.h"
#include "UtilLog.h"
#include "Sequence.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimelinesView::GuiTimelinesView(GuiWindow *parent)
:	wxPanel(parent)
,   mNotebook(this,wxID_ANY)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL); 
    sizer->Add( &mNotebook, 1, wxGROW );
    SetSizerAndFit(sizer);

    wxGetApp().Bind(model::EVENT_REMOVE_ASSET,      &GuiTimelinesView::OnProjectAssetRemoved,       this);
    wxGetApp().Bind(model::EVENT_RENAME_ASSET,      &GuiTimelinesView::OnProjectAssetRenamed,       this);
    Bind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,       &GuiTimelinesView::OnPageChanged,               this);
}

GuiTimelinesView::~GuiTimelinesView()
{
    wxGetApp().Unbind(model::EVENT_REMOVE_ASSET,      &GuiTimelinesView::OnProjectAssetRemoved,       this);
    wxGetApp().Unbind(model::EVENT_RENAME_ASSET,      &GuiTimelinesView::OnProjectAssetRenamed,       this);
    Unbind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,         &GuiTimelinesView::OnPageChanged,               this);
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimelinesView::OnProjectAssetRemoved( model::EventRemoveAsset &event )
{
    model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(event.getValue().child);
    if (sequence)
    {
        Close(sequence);
    }
    event.Skip();
}

void GuiTimelinesView::OnProjectAssetRenamed( model::EventRenameAsset &event )
{
    model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(event.getValue().node);

    if (sequence)
    {
        std::pair<size_t,timeline::GuiTimeLine*> f = findPage(sequence);
        if (f.second != 0)
        {
            mNotebook.SetPageText(f.first, event.getValue().newname);
        }
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiTimelinesView::OnPageChanged(wxNotebookEvent& event)
{
	update();
}

//////////////////////////////////////////////////////////////////////////
// OPEN/CLOSE SEQUENCE
//////////////////////////////////////////////////////////////////////////

void GuiTimelinesView::Open( model::SequencePtr sequence )
{
    ASSERT(sequence);

    std::pair<size_t,timeline::GuiTimeLine*> f = findPage(sequence);
    if (f.second == 0)
    {
        timeline::GuiTimeLine* timeline = new timeline::GuiTimeLine(sequence);
        timeline->init(&mNotebook);
        timeline->SetFocus();
        mNotebook.AddPage(timeline,sequence->getName(),false);
    }
    mNotebook.SetSelection(findPage(sequence).first); // Don't reuse f, since the current active timeline might just have been added above.
    update();
}

void GuiTimelinesView::Close( model::SequencePtr sequence )
{
    if (sequence)
    {
        std::pair<size_t,timeline::GuiTimeLine*> f = findPage(sequence);
        if (f.second != 0)
        {
            mNotebook.DeletePage(f.first);
        }
    }
    else
    {
        // Close open sequence
        mNotebook.DeletePage(mNotebook.GetSelection());
    }
    update();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

std::pair<size_t,timeline::GuiTimeLine*> GuiTimelinesView::findPage(model::SequencePtr sequence) const
{
    size_t page = 0;
    while (page < mNotebook.GetPageCount())
    {
        timeline::GuiTimeLine* timeline = static_cast<timeline::GuiTimeLine*>(mNotebook.GetPage(page));
        if (timeline->getSequence() == sequence)
        {
            return std::make_pair<size_t,timeline::GuiTimeLine*>(page,timeline);
        }
        ++page;
    }

    return std::make_pair<size_t,timeline::GuiTimeLine*>(0,0);
}

void GuiTimelinesView::update() const
{
    GuiWindow& window = *(dynamic_cast<GuiWindow*>(GetParent()));
    timeline::GuiTimeLine* timeline = static_cast<timeline::GuiTimeLine*>(mNotebook.GetCurrentPage());
    if (timeline)
    {
        window.setSequenceMenu(timeline->getMenuHandler().getMenu());
    }
    else
    {
        window.setSequenceMenu(0);
    }
    window.getPreview().selectTimeline(timeline);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiTimelinesView::save(Archive & ar, const unsigned int version) const
{
    unsigned int n = mNotebook.GetPageCount();
    ar & n;
    unsigned int selectedPage = mNotebook.GetSelection();
    ar & selectedPage;
    for (unsigned int page = 0; page < n; ++page)
    {
        ar & *(static_cast<timeline::GuiTimeLine*>(mNotebook.GetPage(page)));
    }
}
template<class Archive>
void GuiTimelinesView::load(Archive & ar, const unsigned int version)
{
    unsigned int n;
    ar & n;
    unsigned int selectedPage = wxNOT_FOUND;
    ar & selectedPage;
    for (unsigned int page = 0; page < n; ++page)
    {
        timeline::GuiTimeLine* timeline = new timeline::GuiTimeLine();
        ar & *timeline;
        timeline->init(&mNotebook);
        mNotebook.AddPage(timeline,timeline->getSequence()->getName(),false);
    }
    if (selectedPage != wxNOT_FOUND)
    {
        ASSERT(selectedPage < mNotebook.GetPageCount());
        mNotebook.SetSelection(selectedPage);
    }
    update();
}
template void GuiTimelinesView::save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion) const;
template void GuiTimelinesView::load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace
