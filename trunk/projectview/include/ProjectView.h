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

#pragma once

#include "ProjectViewCtrl.h"
#include "ProjectViewModel.h"
#include "ProjectViewDropSource.h"
#include "UtilSingleInstance.h"

namespace test {
    class ProjectViewTests;
}

namespace gui {

class TimelinesView;
class ProjectViewClipboard;

enum NodeType
{
    NODETYPE_ANY,
    NODETYPE_FILE,
    NODETYPE_FOLDER,
    NODETYPE_SEQUENCE
};

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

    void onOpenProject(model::EventOpenProject &event);
    void onCloseProject(model::EventCloseProject &event);
    void onAutoOpenFolder(EventAutoFolderOpen& event);

    //////////////////////////////////////////////////////////////////////////
    // SELECTION
    //////////////////////////////////////////////////////////////////////////

    /// \nodes these nodes are selected after the call, all others are not
    void select(const model::NodePtrs& nodes);
    void selectAll();

    /// Return one node to be used for adding nodes.
    /// If more than one node is selected, or a non-folder node is
    /// selected, returns a 0 pointer.
    /// \return nullptr if no single folder can be determined
    /// \return root node if nothing is selected
    /// \return single folder if only one folder node is selected
    model::FolderPtr getSelectedContainerOrRoot() const;

    /// \pre Number of selected items equals 1
    /// \pre Selected item must be a container (folder)
    /// \return selected folder
    model::FolderPtr getSelectedContainer() const;

    /// \return true if one or more nodes are selected.
    bool hasSelection() const;

    model::NodePtrs getSelection() const;

	bool selectionContainsRootNode() const;
    
    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// \return center position (within the ProjectView) of the given node.
    wxPoint find(const model::NodePtr& node);

    void expand(const model::NodePtr& node);

    /// Set the folders that must be opened (expanded) after opening a new
    /// project.
    void setOpenFolders(std::vector<model::FolderPtr> folders);

    void scrollToRight();

    bool findConflictingName(const model::FolderPtr& parent, const wxString& name, const NodeType& type);

    //////////////////////////////////////////////////////////////////////////
    // POPUP MENU
    //////////////////////////////////////////////////////////////////////////

    void onDelete();
    void onDeleteUnused();
    void onNewFolder(const model::FolderPtr& parent);
    void onNewAutoFolder(const model::FolderPtr& parent);
    void onNewAutoFolderInRoot();
    void onNewSequence(const model::FolderPtr& parent);
    void onNewSequenceInRoot();
    void onNewFile(const model::FolderPtr& parent);
    void onNewFileInRoot();
    void onNewFileFromWindow();
    void onCreateSequence();
    void onAddToEndOfSequence();
    void onOpen();

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onContextMenu(wxDataViewEvent &event);
    void onMotion(wxMouseEvent& event);
    void onDropPossible(wxDataViewEvent &event);
    void onDrop(wxDataViewEvent &event);
    void onActivated(wxDataViewEvent &event);
    void onExpanded(wxDataViewEvent &event);
    void onCollapsed(wxDataViewEvent &event);
    void onStartEditing(wxDataViewEvent &event);

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

	ProjectViewClipboard* mClipboard;
    ProjectViewCtrl mCtrl;
    ProjectViewModel* mModel;
    ProjectViewDropSource mDropSource;
    std::vector<model::FolderPtr> mOpenFolders;
    int mDragCount;                                 ///< Used for determining start of dragging
    wxPoint mDragStart;                             ///< Holds start of dragging point
    int mHeaderHeight;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

} // namespace

BOOST_CLASS_VERSION(gui::ProjectView, 1)
