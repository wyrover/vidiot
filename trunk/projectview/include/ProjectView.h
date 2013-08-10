// Copyright 2013 Eric Raijmakers.
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

#ifndef GUI_PROJECT_VIEW_H
#define GUI_PROJECT_VIEW_H

#include "ProjectViewCtrl.h"
#include "ProjectViewModel.h"
#include "ProjectViewDropSource.h"
#include "UtilSingleInstance.h"

namespace test {
    class ProjectViewTests;
}

namespace gui {

class TimelinesView;

class ProjectView
:   public wxPanel
,   public SingleInstance<ProjectView>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ProjectView(wxWindow* parent);
    virtual ~ProjectView();

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onOpenProject( model::EventOpenProject &event );
    void onCloseProject( model::EventCloseProject &event );
    void onAutoOpenFolder( EventAutoFolderOpen& event );

    //////////////////////////////////////////////////////////////////////////
    // SELECTION
    //////////////////////////////////////////////////////////////////////////

    /// \nodes these nodes are selected after the call, all others are not
    void select( model::NodePtrs nodes );
    void selectAll();
    model::FolderPtr getSelectedContainer() const;
    model::NodePtrs getSelection() const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// \return center position (within the ProjectView) of the given node.
    wxPoint find( model::NodePtr node );

    void selectColumnHeader();

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onContextMenu( wxDataViewEvent &event );
    void onCut(wxCommandEvent& event);
    void onCopy(wxCommandEvent& event);
    void onPaste(wxCommandEvent& event);
    void onDelete(wxCommandEvent& event);
    void onNewFolder(wxCommandEvent& event);
    void onNewAutoFolder(wxCommandEvent& event);
    void onNewSequence(wxCommandEvent& event);
    void onNewFile(wxCommandEvent& event);
    void onCreateSequence(wxCommandEvent& event);
    void onAddToEndOfSequence(wxCommandEvent& event);
    void onOpen(wxCommandEvent& event);
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
    ProjectViewCtrl mCtrl;
    ProjectViewModel* mModel;
    ProjectViewDropSource mDropSource;
    std::set<model::FolderPtr> mOpenFolders;
    int mDragCount;                                 ///< Used for determining start of dragging
    wxPoint mDragStart;                             ///< Holds start of dragging point
    int mHeaderHeight;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

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