#include "TimelinesView.h"

#include "Node.h"
#include "Menu.h"
#include "Preview.h"
#include "ProjectEvent.h"
#include "NodeEvent.h"
#include "Sequence.h"
#include "Timeline.h"

#include "UtilLog.h"
#include "Window.h"

namespace gui {

static TimelinesView* sCurrent = 0;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

TimelinesView::TimelinesView(Window *parent)
:	wxPanel(parent)
,   mNotebook(this,wxID_ANY)
{
    sCurrent = this;

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add( &mNotebook, 1, wxGROW );
    SetSizerAndFit(sizer);

    gui::Window::get().Bind(model::EVENT_CLOSE_PROJECT,             &TimelinesView::onCloseProject,            this);
    gui::Window::get().Bind(model::EVENT_REMOVE_NODE,              &TimelinesView::onProjectAssetRemoved,       this);
    gui::Window::get().Bind(model::EVENT_RENAME_NODE,              &TimelinesView::onProjectAssetRenamed,       this);
    mNotebook.Bind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,     &TimelinesView::onPageChanged,               this);
}

TimelinesView::~TimelinesView()
{
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,           &TimelinesView::onCloseProject,              this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_NODE,            &TimelinesView::onProjectAssetRemoved,       this);
    gui::Window::get().Unbind(model::EVENT_RENAME_NODE,            &TimelinesView::onProjectAssetRenamed,       this);
    mNotebook.Unbind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,   &TimelinesView::onPageChanged,               this);
    sCurrent = 0;
}

// static
TimelinesView& TimelinesView::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
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

void TimelinesView::onProjectAssetRemoved( model::EventRemoveNode &event )
{
    model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(event.getValue().getChild());
    if (sequence)
    {
        Close(sequence);
    }
    event.Skip();
}

void TimelinesView::onProjectAssetRenamed( model::EventRenameNode &event )
{
    model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(event.getValue().getNode());

    if (sequence)
    {
        std::pair<size_t,timeline::Timeline*> f = findPage(sequence);
        if (f.second != 0)
        {
            mNotebook.SetPageText(f.first, event.getValue().getName());
        }
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void TimelinesView::onPageChanged(wxNotebookEvent& event)
{
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
    if (mNotebook.GetPageCount() > 0)
    {
        static_cast<timeline::Timeline*>(mNotebook.GetPage(mNotebook.GetSelection()))->activate();
    }
}

timeline::Timeline& TimelinesView::getTimeline( model::SequencePtr sequence )
{
    if (sequence)
    {
        std::pair<size_t,timeline::Timeline*> f = findPage(sequence);
        ASSERT(f.second);
        return *(f.second);
    }
    return static_cast<timeline::Timeline&>(*mNotebook.GetPage(0));
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
        ASSERT_LESS_THAN(selectedPage,mNotebook.GetPageCount());
        mNotebook.SetSelection(selectedPage);
    }
}
template void TimelinesView::save<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion) const;
template void TimelinesView::load<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace