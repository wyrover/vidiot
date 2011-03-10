#ifndef GUI_PROJECT_VIEW_H
#define GUI_PROJECT_VIEW_H

#include <set>
#include <wx/dataview.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include "GuiProjectViewModel.h"
#include "ModelPtr.h"

namespace gui {

class GuiTimelinesView;

class GuiProjectView
:   public wxPanel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    GuiProjectView(wxWindow* parent);
    virtual ~GuiProjectView();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnOpenProject( model::EventOpenProject &event );
    void OnCloseProject( model::EventCloseProject &event );
    void OnAutoOpenFolder( EventAutoFolderOpen& event );

    /**
    * Called to open any folders that were 'marked' as open
    * when saving the document.
    */
    void OpenRecursive(model::FolderPtr folder);

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnContextMenu( wxDataViewEvent &event );
    void OnCut(wxCommandEvent& WXUNUSED(event));
    void OnCopy(wxCommandEvent& WXUNUSED(event));
    void OnPaste(wxCommandEvent& WXUNUSED(event));
    void OnDelete(wxCommandEvent& WXUNUSED(event));
    void OnNewFolder(wxCommandEvent& WXUNUSED(event));
    void OnNewAutoFolder(wxCommandEvent& WXUNUSED(event));
    void OnNewSequence(wxCommandEvent& WXUNUSED(event));
    void OnNewFile(wxCommandEvent& WXUNUSED(event));
    void OnCreateSequence(wxCommandEvent& WXUNUSED(event));
    void OnUpdateAutoFolder(wxCommandEvent& WXUNUSED(event));
    void OnStartEditing( wxDataViewEvent &event );
    void OnBeginDrag( wxDataViewEvent &event );
    void OnDropPossible( wxDataViewEvent &event );
    void OnDrop( wxDataViewEvent &event );
    void OnActivated( wxDataViewEvent &event );
    void OnExpanded( wxDataViewEvent &event );
    void OnCollapsed( wxDataViewEvent &event );

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::Project* mProject;
    wxDataViewCtrl mCtrl;
    GuiProjectViewModel* mModel;
    std::set<model::FolderPtr> mOpenFolders;

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
BOOST_CLASS_VERSION(gui::GuiProjectView, 1)

#endif // GUI_PROJECT_VIEW_H
