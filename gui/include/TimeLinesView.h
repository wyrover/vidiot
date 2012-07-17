#ifndef GUI_SEQUENCE_VIEW_H
#define GUI_SEQUENCE_VIEW_H

#include <wx/notebook.h>
#include <wx/panel.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

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
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    TimelinesView(Window *parent);
	virtual ~TimelinesView();

    static TimelinesView& get();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onCloseProject( model::EventCloseProject &event );
    void onProjectAssetRemoved( model::EventRemoveNode &event );
    void onProjectAssetRenamed( model::EventRenameNode &event );

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onPageChanged(wxNotebookEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // OPEN/CLOSE SEQUENCE
    //////////////////////////////////////////////////////////////////////////

    /// Open a timeline for the given sequence. If there already is a timeline
    /// for this sequence, it is selected.
    void Open( model::SequencePtr sequence );

    /// Close the timeline for the given sequence.
    /// Close current open sequence by default.
    void Close( model::SequencePtr sequence = model::SequencePtr() );

    /// \return timeline for the given sequence
    /// \param sequence if 0, then the first timeline (the first notebook page) is returned.
    /// \pre sequence must be opened (thus, timeline must exist)
    timeline::Timeline& getTimeline( model::SequencePtr sequence = model::SequencePtr() );

private:

	wxNotebook mNotebook;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    std::pair<size_t,timeline::Timeline*> findPage(model::SequencePtr sequence) const; ///< Find the page associated with a sequence. 0 pointer is returned if not found.

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

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(gui::TimelinesView, 1)

#endif // GUI_SEQUENCE_VIEW_H