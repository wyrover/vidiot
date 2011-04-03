#ifndef GUI_SEQUENCE_VIEW_H
#define GUI_SEQUENCE_VIEW_H

#include <wx/notebook.h>
#include <wx/panel.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>

namespace model {
    class Project;
    class EventRemoveAsset;
    class EventRenameAsset;
    class Sequence;
    typedef boost::shared_ptr<Sequence> SequencePtr;
}

namespace gui {

class GuiWindow;

namespace timeline {
    class Timeline;
}

class GuiTimelinesView
:   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    GuiTimelinesView(GuiWindow *parent);
	virtual ~GuiTimelinesView();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnProjectAssetRemoved( model::EventRemoveAsset &event );
    void OnProjectAssetRenamed( model::EventRenameAsset &event );

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnPageChanged(wxNotebookEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // OPEN/CLOSE SEQUENCE
    //////////////////////////////////////////////////////////////////////////

    /// Open a timeline for the given sequence. If there already is a timeline
    /// for this sequence, it is selected.
    void Open( model::SequencePtr sequence );

    /// Close the timeline for the given sequence.
    /// Close current open sequence by default.
    void Close( model::SequencePtr sequence = model::SequencePtr() );

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
BOOST_CLASS_VERSION(gui::GuiTimelinesView, 1)

#endif // GUI_SEQUENCE_VIEW_H
