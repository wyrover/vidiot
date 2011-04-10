#ifndef GUI_PROJECT_VIEW_H
#define GUI_PROJECT_VIEW_H

#include <set>
#include <wx/dataview.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include "ProjectViewModel.h"
#include "ProjectViewDropSource.h"

namespace gui {

class GuiTimelinesView;

class ProjectView
:   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ProjectView(wxWindow* parent);
    virtual ~ProjectView();
    static ProjectView* current();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onOpenProject( model::EventOpenProject &event );
    void onCloseProject( model::EventCloseProject &event );
    void onAutoOpenFolder( EventAutoFolderOpen& event );

    /// Called to open any folders that were 'marked' as open
    /// when saving the document.
    void OpenRecursive(model::FolderPtr folder);

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onContextMenu( wxDataViewEvent &event );
    void onCut(wxCommandEvent& WXUNUSED(event));
    void onCopy(wxCommandEvent& WXUNUSED(event));
    void onPaste(wxCommandEvent& WXUNUSED(event));
    void onDelete(wxCommandEvent& WXUNUSED(event));
    void onNewFolder(wxCommandEvent& WXUNUSED(event));
    void onNewAutoFolder(wxCommandEvent& WXUNUSED(event));
    void onNewSequence(wxCommandEvent& WXUNUSED(event));
    void onNewFile(wxCommandEvent& WXUNUSED(event));
    void onCreateSequence(wxCommandEvent& WXUNUSED(event));
    void onUpdateAutoFolder(wxCommandEvent& WXUNUSED(event));
    void onStartEditing( wxDataViewEvent &event );
    void onMotion(wxMouseEvent& event);
    void onDragEnd();
    void onDropPossible( wxDataViewEvent &event );
    void onDrop( wxDataViewEvent &event );
    void onActivated( wxDataViewEvent &event );
    void onExpanded( wxDataViewEvent &event );
    void onCollapsed( wxDataViewEvent &event );

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::Project* mProject;
    wxDataViewCtrl mCtrl;
    ProjectViewModel* mModel;
    ProjectViewDropSource mDropSource;
    std::set<model::FolderPtr> mOpenFolders;
    int mDragCount;                                 ///< Used for determining start of dragging
    wxPoint mDragStart;                             ///< Holds start of dragging point

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    model::FolderPtr getSelectedContainer() const;
    model::ProjectViewPtrs getSelection() const;
    bool FindConflictingName(model::FolderPtr parent, wxString name );

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(gui::ProjectView, 1)

#endif // GUI_PROJECT_VIEW_H
