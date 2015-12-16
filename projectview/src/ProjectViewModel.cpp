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

#include "ProjectView.h"

#include "AutoFolder.h"
#include "File.h"
#include "Folder.h"
#include "INode.h"
#include "NodeEvent.h"
#include "Project.h"
#include "ProjectEvent.h"
#include "ProjectModification.h"
#include "ProjectViewAddAsset.h"
#include "ProjectViewDeleteAsset.h"
#include "ProjectViewMoveAsset.h"
#include "ProjectViewRenameAsset.h"
#include "Sequence.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilPath.h"
#include "UtilWindow.h"
#include "Window.h"

namespace gui {

const int sNameColumn = 0;
const int sNumberOfColumns = 1;

ProjectViewModel::ProjectViewModel(wxDataViewCtrl& view)
: wxDataViewModel()
, mView(view)
, mIconAudio(util::window::getIcon("music-beam.png"))
, mIconAutoFolder(util::window::getIcon("folder-horizontal-plus.png"))
, mIconAutoFolderOpen(util::window::getIcon("folder-horizontal-plus-open.png"))
, mIconFolder(util::window::getIcon("folder-horizontal.png"))
, mIconFolderOpen(util::window::getIcon("folder-horizontal-open.png"))
, mIconPicture(util::window::getIcon("picture.png"))
, mIconSequence(util::window::getIcon("film.png"))
, mIconTitle(util::window::getIcon("edit.png"))
, mIconVideo(util::window::getIcon("clapperboard.png"))
{
    gui::Window::get().Bind(model::EVENT_OPEN_PROJECT,     &ProjectViewModel::onOpenProject,           this);
    gui::Window::get().Bind(model::EVENT_CLOSE_PROJECT,    &ProjectViewModel::onCloseProject,          this);
}

ProjectViewModel::~ProjectViewModel()
{
    gui::Window::get().Unbind(model::EVENT_OPEN_PROJECT,   &ProjectViewModel::onOpenProject,            this);
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,  &ProjectViewModel::onCloseProject,           this);
}

//////////////////////////////////////////////////////////////////////////
// OVERRIDES FROM WXDATAVIEWMODEL
//////////////////////////////////////////////////////////////////////////

bool ProjectViewModel::IsContainer (const wxDataViewItem &wxItem) const
{
    if (!wxItem.IsOk())
    {
        // invisble root node can have children (root asset)
        return true;
    }
    else
    {
        return isFolder(model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID())));
    }
}

wxDataViewItem ProjectViewModel::GetParent( const wxDataViewItem &wxItem ) const
{
    if (!wxItem.IsOk())
    {
        // invisible root node is the parent of the root asset
        return wxDataViewItem(0);
    }
    else
    {
        model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
        if (!p->hasParent())
        {
            // Root asset has the invisible root as parent
            // NOT: ASSERT_EQUALS(p,model::Project::get().getRoot()): This can also happen by first adding a large autofolder and then deleting it while indexing (thus, by adding nodes to a parent that has been removed already).
            return wxDataViewItem(0);
        }
        else
        {
            return wxDataViewItem(p->getParent()->id());
        }
    }
}

unsigned int ProjectViewModel::GetChildren( const wxDataViewItem &wxItem, wxDataViewItemArray &wxItemArray ) const
{
    if (!wxItem.IsOk())
    {
        if (Window::get().isProjectOpened())
        {
            wxItemArray.Add(wxDataViewItem(model::Project::get().getRoot()->id()));
            gui::Window::get().QueueModelEvent(new EventAutoFolderOpen(model::Project::get().getRoot()));
            return 1;
        }
        else
        {
            // No project opened
            return 0;
        }
    }

    model::NodePtr parent = model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
    for (model::NodePtr child : parent->getChildren() )
    {
        wxItemArray.Add(wxDataViewItem(child->id()));

        model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(child);
        if (folder)
        {
            gui::Window::get().QueueModelEvent(new EventAutoFolderOpen(folder));
        }
    }

    return wxItemArray.GetCount();
}

unsigned int ProjectViewModel::GetColumnCount() const
{
    return sNumberOfColumns;
}

wxString ProjectViewModel::GetColumnType(unsigned int col) const
{
    ASSERT_LESS_THAN_EQUALS(col,GetColumnCount());
    switch (col)
    {
    case sNameColumn: return wxT("icontext");
    }
    return wxT("string");
}

void ProjectViewModel::GetValue( wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col ) const
{
    ASSERT(wxItem.IsOk());
    ASSERT_LESS_THAN_EQUALS(col,GetColumnCount());

    model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
    switch (col)
    {
    case sNameColumn:
        {
            wxDataViewIconText icontext(node->getName());
            icontext.SetIcon(getIcon(node));
            variant << icontext;
            return;
        }
    }
}

bool ProjectViewModel::SetValue(const wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col)
{
    ASSERT(wxItem.IsOk());
    ASSERT_LESS_THAN_EQUALS(col,GetColumnCount());
    ASSERT_ZERO(col); // Only rename is possible

    model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
    if (canBeRenamed(node))
    {
        wxDataViewIconText icontext;
        icontext << variant;
        model::ProjectModification::submit(new cmd::ProjectViewRenameAsset(node, icontext.GetText()));
        return true;
    }
    return false;
}

int ProjectViewModel::Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const
{
    int result = 0;
    if (column == UINT_MAX)
    {
        // Default when no column header has been clicked
        column = sNameColumn;
    }

    model::NodePtr node1 = model::INode::Ptr(static_cast<model::NodeId>(item1.GetID()));
    model::NodePtr node2 = model::INode::Ptr(static_cast<model::NodeId>(item2.GetID()));

    if (node1.get() == node2.get())
    {
        result = 0;
    }
    else if (isDescendantOf(node1,node2))
    {
        result = -1;
    }
    else if (isDescendantOf(node2,node1))
    {
        result = 1;
    }
    else if (!isFolder(node1) && isFolder(node2))
    {
        result = 1;
    }
    else if (isFolder(node1) && !isFolder(node2))
    {
        result = -1;
    }
    else
    {
        wxVariant value1, value2;
        GetValue( value1, item1, column );
        GetValue( value2, item2, column );

        switch (column)
        {
        case sNameColumn:
            {
                wxDataViewIconText icontext1;
                icontext1 << value1;
                wxDataViewIconText icontext2;
                icontext2 << value2;

                wxString str1 = icontext1.GetText();
                wxString str2 = icontext2.GetText();
                result = str1.CmpNoCase(str2);
                break;
            }
        default:
            FATAL("Unknown column");
        }

        if (!ascending)
        {
            result *= -1;
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

bool ProjectViewModel::isAutomaticallyGenerated(const model::NodePtr& node) const
{
    if (!node->hasParent())
    {
        return false;
    }
    else
    {
        model::NodePtr parent = node->getParent();
        if (isAutoFolder(parent))
        {
            return true;
        }
        else
        {
            return isAutomaticallyGenerated(parent);
        }
    }
}

bool ProjectViewModel::isRoot(const model::NodePtr& node) const
{
    return !node->hasParent();
}

bool ProjectViewModel::isFolder(const model::NodePtr& node) const
{
    return boost::dynamic_pointer_cast<model::Folder>(node) != 0;
}

bool ProjectViewModel::isAutoFolder(const model::NodePtr& node) const
{
    return boost::dynamic_pointer_cast<model::AutoFolder>(node) != 0;
}

bool ProjectViewModel::isSequence(const model::NodePtr& node) const
{
    return boost::dynamic_pointer_cast<model::Sequence>(node) != 0;
}

bool ProjectViewModel::isDescendantOf(const model::NodePtr& node, const model::NodePtr& ascendant) const
{
    if (!node->hasParent())
    {
        // this node is an orphan. It might be the root (project) node, or it was deleted/cut.
        return false;
    }
    model::NodePtr parent = node->getParent();
    if (parent == ascendant)
    {
        return true;
    }
    return isDescendantOf(parent, ascendant);
}

bool ProjectViewModel::canBeRenamed(const model::NodePtr& node) const
{
    return !isRoot(node) && !isAutomaticallyGenerated(node) && !isAutoFolder(node);
}

wxIcon ProjectViewModel::getIcon(const model::NodePtr& node) const
{
    wxIcon icon = mIconVideo;
    wxDataViewItem wxItem = wxDataViewItem(node->id());
    if (isFolder(node))
    {
        bool open = (mView.IsExpanded(wxItem));
        if (isAutoFolder(node))
        {
            icon = open ? mIconAutoFolderOpen : mIconAutoFolder;
        }
        else
        {
            icon = open ? mIconFolderOpen : mIconFolder;
        }
    }
    else if (isSequence(node))
    {
        icon = mIconSequence;
    }
    else
    {
        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(node);
        ASSERT(file);
        switch (file->getType())
        {
        case model::FileType_Image:
            icon = mIconPicture;
            break;
        case model::FileType_Title:
            icon = mIconTitle;
            break;
        case model::FileType_Audio:
            icon = mIconAudio;
            break;
        case model::FileType_Video: // FALLTHROUGH
        default:
            icon = mIconVideo;
            break;
        }
    }
    return icon;
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void ProjectViewModel::onOpenProject(model::EventOpenProject &event)
{
    ItemAdded(wxDataViewItem(0), wxDataViewItem(model::Project::get().getRoot()->id()));

    if (!event.getValue()) // ! : only if 'loading' a document, see Project::LoadObject()
    {
        // Without the following statements, upon auto-loading a document, the wxGTK version
        // does not show any initial nodes under the project node (the topmost visible node).
        //
        // Furthermore, any descendents 'deeper down the hierarchy' are also not known.
        // This causes problems, for instance, when opening a document for which a file on disk
        // has been removed before opening the document. In that case, the file check causes the
        // file to be removed from the tree view, but it was not yet known in the data view ctrl,
        // causing a lookup failure (crash) in wx.
        std::function<void(model::NodePtr)> addNodes = [this, &addNodes](model::NodePtr parent) -> void
        {
            if (parent->getChildren().size() != 0)
            {
                model::EventAddNodes nodesEvent(model::ParentAndChildren(parent, parent->getChildren()));
                onProjectAssetsAdded(nodesEvent);
                for (model::NodePtr child : parent->getChildren())
                {
                    addNodes(child);
                }
            }
        };
        addNodes(model::Project::get().getRoot());
    }

    gui::Window::get().Bind(model::EVENT_ADD_NODE,     &ProjectViewModel::onProjectAssetAdded,     this);
    gui::Window::get().Bind(model::EVENT_ADD_NODES,    &ProjectViewModel::onProjectAssetsAdded,    this);
    gui::Window::get().Bind(model::EVENT_REMOVE_NODE,  &ProjectViewModel::onProjectAssetRemoved,   this);
    gui::Window::get().Bind(model::EVENT_REMOVE_NODES, &ProjectViewModel::onProjectAssetsRemoved,  this);
    gui::Window::get().Bind(model::EVENT_RENAME_NODE,  &ProjectViewModel::onProjectAssetRenamed,   this);

    event.Skip();
}

void ProjectViewModel::onCloseProject(model::EventCloseProject &event)
{
    ItemDeleted(wxDataViewItem(0), wxDataViewItem(model::Project::get().getRoot()->id()));

    gui::Window::get().Unbind(model::EVENT_ADD_NODE,       &ProjectViewModel::onProjectAssetAdded,      this);
    gui::Window::get().Unbind(model::EVENT_ADD_NODES,      &ProjectViewModel::onProjectAssetsAdded,     this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_NODE,    &ProjectViewModel::onProjectAssetRemoved,    this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_NODES,   &ProjectViewModel::onProjectAssetsRemoved,   this);
    gui::Window::get().Unbind(model::EVENT_RENAME_NODE,    &ProjectViewModel::onProjectAssetRenamed,    this);

    event.Skip();
}

void ProjectViewModel::onProjectAssetAdded(model::EventAddNode &event)
{
    model::NodePtr parent = event.getValue().getParent();
    model::NodePtr child = event.getValue().getChild();
    VAR_DEBUG(parent)(child);
    ItemAdded(wxDataViewItem(parent->id()),wxDataViewItem(child->id()));

    mView.Expand(wxDataViewItem(parent->id()));

    event.Skip();
}

void ProjectViewModel::onProjectAssetsAdded(model::EventAddNodes &event)
{
    model::NodePtr parent = event.getValue().getParent();
    model::NodePtrs children = event.getValue().getChildren();
    VAR_DEBUG(parent)(children);
    ASSERT_NONZERO(children.size());

    mView.Freeze();

    wxDataViewItemArray items;
    for (model::NodePtr node : children )
    {
        items.Add(wxDataViewItem(node->id()));
    }
    ItemsAdded(wxDataViewItem(parent->id()),items);

    // NOT: mView.Expand(wxDataViewItem(parent->id())); // This would automatically open folders when items are inserted.

    mView.Thaw();

    event.Skip();
}

void ProjectViewModel::onProjectAssetRemoved(model::EventRemoveNode &event)
{
    model::NodePtr parent = event.getValue().getParent();
    model::NodePtr child = event.getValue().getChild();
    VAR_DEBUG(parent)(child);

    ItemDeleted(wxDataViewItem(parent->id()),wxDataViewItem(child->id()));
    event.Skip();
}

void ProjectViewModel::onProjectAssetsRemoved(model::EventRemoveNodes &event)
{
    model::NodePtr parent = event.getValue().getParent();
    model::NodePtrs children = event.getValue().getChildren();
    VAR_DEBUG(parent)(children);
    ASSERT_NONZERO(children.size());

    mView.Freeze();

    wxDataViewItemArray items;
    for (model::NodePtr node : children )
    {
        items.Add(wxDataViewItem(node->id()));
    }
    ItemsDeleted(wxDataViewItem(parent->id()),items);

    // NOT: mView.Expand(wxDataViewItem(parent->id())); // This would automatically open folders when items are removed.

    mView.Thaw();

    event.Skip();
}

void ProjectViewModel::onProjectAssetRenamed(model::EventRenameNode &event)
{
    VAR_DEBUG(event.getValue().getNode());
    ItemChanged(wxDataViewItem(event.getValue().getNode()->id()));
    event.Skip();
}

DEFINE_EVENT(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, EventAutoFolderOpen, model::FolderPtr);

} // namespace
