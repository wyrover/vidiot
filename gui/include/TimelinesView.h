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

#ifndef TIMELINES_VIEW_H
#define TIMELINES_VIEW_H

#include "UtilSingleInstance.h"

namespace model {
    class EventCloseProject;
    class EventRemoveNode;
    class EventRenameNode;
}

namespace test {
    class FixtureGui;
}

namespace gui {

class Window;

namespace timeline {
    class Timeline;
}

class TimelinesView
:   public wxPanel
,   public SingleInstance<TimelinesView>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    TimelinesView(Window *parent);
    virtual ~TimelinesView();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onCloseProject(model::EventCloseProject &event);
    void onProjectAssetRemoved(model::EventRemoveNode &event);
    void onProjectAssetRenamed(model::EventRenameNode &event);

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onPageChanged(wxNotebookEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // OPEN/CLOSE SEQUENCE
    //////////////////////////////////////////////////////////////////////////

    /// Open a timeline for the given sequence. If there already is a timeline
    /// for this sequence, it is selected.
    void Open(const model::SequencePtr& sequence);

    /// Close the timeline for the given sequence.
    /// Close current open sequence by default.
    void Close(const model::SequencePtr& sequence = model::SequencePtr());

    /// \return timeline for the given sequence
    /// \param sequence if 0, then the first timeline (the first notebook page) is returned.
    /// \pre sequence must be opened (thus, timeline must exist)
    timeline::Timeline& getTimeline(const model::SequencePtr& sequence = model::SequencePtr());

    /// \return true if a timeline is open
    bool hasTimeline() const;

private:

    wxNotebook mNotebook;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    std::pair<size_t,timeline::Timeline*> findPage(const model::SequencePtr& sequence) const; ///< Find the page associated with a sequence. 0 pointer is returned if not found.
    void updateActivation();

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const;
    template<class Archive>
    void load(Archive & ar, const unsigned int version);
    BOOST_SERIALIZATION_SPLIT_MEMBER();
};

} // namespace

BOOST_CLASS_VERSION(gui::TimelinesView, 1)

#endif
