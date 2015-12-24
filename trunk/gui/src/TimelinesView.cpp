// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "TimelinesView.h"

#include "Logging.h"
#include "Node.h"
#include "NodeEvent.h"
#include "ProjectEvent.h"
#include "Sequence.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "Window.h"

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
    gui::Window::get().Bind(model::EVENT_REMOVE_NODE,              &TimelinesView::onProjectAssetRemoved,       this);
    gui::Window::get().Bind(model::EVENT_RENAME_NODE,              &TimelinesView::onProjectAssetRenamed,       this);
    mNotebook.Bind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,     &TimelinesView::onPageChanged,               this);
    Bind(wxEVT_SIZE, &TimelinesView::onSize, this);
}

TimelinesView::~TimelinesView()
{
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,           &TimelinesView::onCloseProject,              this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_NODE,            &TimelinesView::onProjectAssetRemoved,       this);
    gui::Window::get().Unbind(model::EVENT_RENAME_NODE,            &TimelinesView::onProjectAssetRenamed,       this);
    mNotebook.Unbind(wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,   &TimelinesView::onPageChanged,               this);
    Unbind(wxEVT_SIZE, &TimelinesView::onSize, this);
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void TimelinesView::onCloseProject(model::EventCloseProject &event)
{
    mNotebook.DeleteAllPages();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TimelinesView::onProjectAssetRemoved(model::EventRemoveNode &event)
{
    model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(event.getValue().getChild());
    if (sequence)
    {
        Close(sequence);
    }
    event.Skip();
}

void TimelinesView::onProjectAssetRenamed(model::EventRenameNode &event)
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

void TimelinesView::onSize(wxSizeEvent& event)
{
    wxSize s = event.GetSize();
    // To avoid crashes in wxNotebook when one of the notebook dimensions becomes 0.
    mNotebook.Show(s.GetWidth() >= 1 && s.GetHeight() >= 1);
    event.Skip();
}

void TimelinesView::onPageChanged(wxNotebookEvent& event)
{
    updateActivation();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// OPEN/CLOSE SEQUENCE
//////////////////////////////////////////////////////////////////////////

void TimelinesView::Open(const model::SequencePtr& sequence)
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
    updateActivation();
}

void TimelinesView::Close(const model::SequencePtr& sequence)
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
    updateActivation();
}

timeline::Timeline& TimelinesView::getTimeline(const model::SequencePtr& sequence)
{
    if (sequence)
    {
        std::pair<size_t,timeline::Timeline*> f = findPage(sequence);
        ASSERT(f.second);
        return *(f.second);
    }
    ASSERT_NONZERO(mNotebook.GetPageCount());
    return static_cast<timeline::Timeline&>(*mNotebook.GetPage(0));
}

bool TimelinesView::hasTimeline() const
{
    return mNotebook.GetPageCount() != 0;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

std::pair<size_t,timeline::Timeline*> TimelinesView::findPage(const model::SequencePtr& sequence) const
{
    size_t page = 0;
    while (page < mNotebook.GetPageCount())
    {
        timeline::Timeline* timeline = static_cast<timeline::Timeline*>(mNotebook.GetPage(page));
        if (timeline->getSequence() == sequence)
        {
            return std::make_pair(page,timeline);
        }
        ++page;
    }

    return std::make_pair<size_t,timeline::Timeline*>(0,0);
}

void TimelinesView::updateActivation()
{
    if (mNotebook.GetPageCount() > 0)
    {
        size_t page = 0;
        while (page < mNotebook.GetPageCount())
        {
            if (page != mNotebook.GetSelection())
            {
                static_cast<timeline::Timeline*>(mNotebook.GetPage(page))->activate(false);
            }
            ++page;
        }
        static_cast<timeline::Timeline*>(mNotebook.GetPage(mNotebook.GetSelection()))->activate(true);
    }
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

const std::string sSequence("sequence");
const std::string sCount("notebookCount");
const std::string sSelected("selectedPage");
const std::string sTimeline("timeline");

template<class Archive>
void TimelinesView::save(Archive & ar, const unsigned int version) const
{
    unsigned int notebookCount = mNotebook.GetPageCount();
    unsigned int selectedPage = mNotebook.GetSelection();

    ar & boost::serialization::make_nvp(sCount.c_str(),notebookCount);
    ar & boost::serialization::make_nvp(sSelected.c_str(), selectedPage);

    for (unsigned int page = 0; page < notebookCount; ++page)
    {
        timeline::Timeline* timeline = static_cast<timeline::Timeline*>(mNotebook.GetPage(page));
        model::SequencePtr sequence = timeline->getSequence();
        ar & boost::serialization::make_nvp(sSequence.c_str(),sequence);
        ar & boost::serialization::make_nvp(sTimeline.c_str(),*timeline);
    }
}
template<class Archive>
void TimelinesView::load(Archive & ar, const unsigned int version)
{
    unsigned int notebookCount;
    unsigned int selectedPage = wxNOT_FOUND;

    ar & boost::serialization::make_nvp(sCount.c_str(),notebookCount);
    ar & boost::serialization::make_nvp(sSelected.c_str(), selectedPage);

    for (unsigned int page = 0; page < notebookCount; ++page)
    {
        model::SequencePtr sequence;
        ar & boost::serialization::make_nvp(sSequence.c_str(),sequence);
        timeline::Timeline* timeline = new timeline::Timeline(&mNotebook, sequence, true); // true ==> beginTransaction()
        ar & boost::serialization::make_nvp(sTimeline.c_str(),*timeline);
        timeline->endTransaction(); // Only AFTER deserialization, screen updates may be done. Otherwise, for instance, the wrong zoom may be used.
        mNotebook.AddPage(timeline,sequence->getName(),false);
        LOG_INFO << dump(boost::dynamic_pointer_cast<model::Sequence>(sequence));
    }
    if (selectedPage != wxNOT_FOUND)
    {
        ASSERT_LESS_THAN(selectedPage,mNotebook.GetPageCount());
        mNotebook.SetSelection(selectedPage);
        updateActivation();
    }

}
template void TimelinesView::save<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion) const;
template void TimelinesView::load<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} // namespace
