#include "TimelinesView.h"

#include <wx/sizer.h>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "Preview.h"
#include "Timeline.h"
#include "Menu.h"
#include "Project.h"
#include "Window.h"
#include "AProjectViewNode.h"
#include "UtilLog.h"
#include "Sequence.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

TimelinesView::TimelinesView(Window *parent)
:	wxPanel(parent)
,   mNotebook(this,wxID_ANY)
{
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL); 
    sizer->Add( &mNotebook, 1, wxGROW );
    SetSizerAndFit(sizer);

    gui::Window::get().Bind(model::EVENT_CLOSE_PROJECT,             &TimelinesView::onCloseProject,            this);
    gui::Window::get().Bind(model::EVENT_REMOVE_ASSET,              &TimelinesView::onProjectAssetRemoved,       this);
    gui::Window::get().Bind(model::EVENT_RENAME_ASSET,              &TimelinesView::onProjectAssetRenamed,       this);
    mNotebook.Bind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,     &TimelinesView::onPageChanged,               this);
}

TimelinesView::~TimelinesView()
{
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,           &TimelinesView::onCloseProject,              this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_ASSET,            &TimelinesView::onProjectAssetRemoved,       this);
    gui::Window::get().Unbind(model::EVENT_RENAME_ASSET,            &TimelinesView::onProjectAssetRenamed,       this);
    mNotebook.Unbind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,   &TimelinesView::onPageChanged,               this);
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void TimelinesView::onCloseProject( model::EventCloseProject &event )
{
    mNotebook.DeleteAllPages();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TimelinesView::onProjectAssetRemoved( model::EventRemoveAsset &event )
{
    model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(event.getValue().child);
    if (sequence)
    {
        Close(sequence);
    }
    event.Skip();
}

void TimelinesView::onProjectAssetRenamed( model::EventRenameAsset &event )
{
    model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(event.getValue().node);

    if (sequence)
    {
        std::pair<size_t,timeline::Timeline*> f = findPage(sequence);
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

void TimelinesView::onPageChanged(wxNotebookEvent& event)
{
    //wxNOT_FOUND
    //int selection = mNotebook.GetSelection();

    // todo handling the unselection of the 'old' one


    //Window::get().getPreview().selectTimeline(static_cast<timeline::Timeline*>(mNotebook.GetPage(event.GetSelection())
    static_cast<timeline::Timeline*>(mNotebook.GetPage(event.GetSelection()))->activate();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// OPEN/CLOSE SEQUENCE
//////////////////////////////////////////////////////////////////////////

void TimelinesView::Open( model::SequencePtr sequence )
{
    ASSERT(sequence);

    std::pair<size_t,timeline::Timeline*> f = findPage(sequence);
    if (f.second == 0)
    {
        timeline::Timeline* timeline = new timeline::Timeline(&mNotebook, sequence);
        timeline->SetFocus();
        mNotebook.AddPage(timeline,sequence->getName(),false);
    }
    mNotebook.SetSelection(findPage(sequence).first); // Don't reuse f, since the current active timeline might just have been added above.
}

void TimelinesView::Close( model::SequencePtr sequence )
{
    if (sequence)
    {
        std::pair<size_t,timeline::Timeline*> f = findPage(sequence);
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
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

std::pair<size_t,timeline::Timeline*> TimelinesView::findPage(model::SequencePtr sequence) const
{
    size_t page = 0;
    while (page < mNotebook.GetPageCount())
    {
        timeline::Timeline* timeline = static_cast<timeline::Timeline*>(mNotebook.GetPage(page));
        if (timeline->getSequence() == sequence)
        {
            return std::make_pair<size_t,timeline::Timeline*>(page,timeline);
        }
        ++page;
    }

    return std::make_pair<size_t,timeline::Timeline*>(0,0);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void TimelinesView::save(Archive & ar, const unsigned int version) const
{
    unsigned int n = mNotebook.GetPageCount();
    ar & n;
    unsigned int selectedPage = mNotebook.GetSelection();
    ar & selectedPage;
    for (unsigned int page = 0; page < n; ++page)
    {
        timeline::Timeline* timeline = static_cast<timeline::Timeline*>(mNotebook.GetPage(page));
        ar & timeline->getSequence();
        ar & *timeline;
    }
}
template<class Archive>
void TimelinesView::load(Archive & ar, const unsigned int version)
{
    unsigned int n;
    ar & n;
    unsigned int selectedPage = wxNOT_FOUND;
    ar & selectedPage;
    for (unsigned int page = 0; page < n; ++page)
    {
        model::SequencePtr sequence;
        ar & sequence;
        timeline::Timeline* timeline = new timeline::Timeline(&mNotebook, sequence);
        ar & *timeline;
        mNotebook.AddPage(timeline,sequence->getName(),false);
    }
    if (selectedPage != wxNOT_FOUND)
    {
        ASSERT(selectedPage < mNotebook.GetPageCount());
        mNotebook.SetSelection(selectedPage);
    }
}
template void TimelinesView::save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion) const;
template void TimelinesView::load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace
