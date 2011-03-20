#ifndef GUI_PROJECT_VIEW_H
#define GUI_PROJECT_VIEW_H

#include <set>
#include <wx/dataview.h>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include "ProjectViewModel.h"
#include "ModelPtr.h"
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

    void OnOpenProject( model::EventOpenProject &event );
    void OnCloseProject( model::EventCloseProject &event );
    void OnAutoOpenFolder( EventAutoFolderOpen& event );

    /**
    * Called to open any folders that were 'marked' as open
    * when saving the document.
    */
    void OpenRecursive(model::FolderPtr folder);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    std::list<model::IControlPtr> getDraggedAssets() { return mDraggedAssets; }


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
    void OnMotion(wxMouseEvent& event);
    void onDragEnd();
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
    ProjectViewModel* mModel;
    ProjectViewDropSource mDropSource;
    std::set<model::FolderPtr> mOpenFolders;
    std::list<model::IControlPtr> mDraggedAssets;
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
