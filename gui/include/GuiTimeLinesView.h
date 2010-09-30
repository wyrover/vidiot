#ifndef GUI_SEQUENCE_VIEW_H
#define GUI_SEQUENCE_VIEW_H

#include <wx/notebook.h>
#include <wx/panel.h>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include "GuiPtr.h"
#include "ModelPtr.h"

namespace model {
    class Project;
    class EventRemoveAsset;
    class EventRenameAsset;
}

namespace gui {

class GuiWindow;

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

    /**
    * Open a timeline for the given sequence. If there already is a timeline
    * for this sequence, it is selected.
    */
    void Open( model::SequencePtr sequence );

    /**
    * Close the timeline for the given sequence.
    * Close current open sequence by default.
    */
    void Close( model::SequencePtr sequence = model::SequencePtr() );

private:

	wxNotebook mNotebook;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /** Find the page associated with a sequence. 0 pointer is returned if not found. */
    std::pair<size_t,timeline::GuiTimeLinePtr> findPage(model::SequencePtr sequence) const;

    /** To be called whenever the notebook selection (current open page) changes. */
    void update() const;

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
