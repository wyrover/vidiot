#include "ProjectView.h"

#include <wx/datetime.h>
#include <boost/foreach.hpp>
#include "Node.h"
#include "AutoFolder.h"
#include "File.h"
#include "film.xpm" 
#include "Folder.h"
#include "folder-horizontal.xpm"
#include "folder-horizontal-open.xpm"
#include "folder-horizontal-plus.xpm"
#include "folder-horizontal-plus-open.xpm"
#include "Project.h"
#include "ProjectEvent.h"
#include "ProjectViewAddAsset.h"
#include "ProjectViewDeleteAsset.h"
#include "ProjectViewMoveAsset.h"
#include "NodeEvent.h"
#include "ProjectViewRenameAsset.h"
#include "Sequence.h"
#include "UtilLog.h"
#include "Window.h"

namespace gui {

ProjectViewModel::ProjectViewModel(wxDataViewCtrl& view)
:   wxDataViewModel()
,   mView(view)
,   mProject(0)
,   mIconAutoFolder(folder_horizontal_plus_xpm)
,   mIconAutoFolderOpen(folder_horizontal_plus_open_xpm)
,   mIconFolder(folder_horizontal_xpm)
,   mIconFolderOpen(folder_horizontal_open_xpm)
,	mIconVideo(film_xpm)
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
            ASSERT(p == model::Project::get().getRoot())(p);
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
        if (mProject != 0)
        {
            wxItemArray.Add(wxDataViewItem(mProject->getRoot()->id()));
            gui::Window::get().QueueModelEvent(new EventAutoFolderOpen(mProject->getRoot()));
            return 1;
        }
        else
        {
            // No project opened
            return 0;
        }
    }

    model::NodePtr parent = model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
    BOOST_FOREACH( model::NodePtr child, parent->getChildren() )
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
    return 2;
}

wxString ProjectViewModel::GetColumnType(unsigned int col) const
{
    ASSERT(col <= GetColumnCount());
    switch (col)
    {
    case 0: return wxT("icontext");
    case 1: return wxT("string");
    }
    return wxT("string");
}

void ProjectViewModel::GetValue( wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col ) const
{
    ASSERT(wxItem.IsOk());
    ASSERT(col <= GetColumnCount());

    model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
    switch (col)
    {
    case 0: 
        {
            wxDataViewIconText icontext(node->getName());
            icontext.SetIcon(getIcon(node));
            variant << icontext;
            return;
        }
    case 1: 
        {
            model::FilePtr file = boost::dynamic_pointer_cast<model::File>(node);
            if (file)
            {
                variant = wxDateTime(file->getLastModified());
                return;
            }
            else
            {
                variant = wxString(""); 
                return;
            }
        }
    }
}

bool ProjectViewModel::SetValue( const wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col )
{
    ASSERT(wxItem.IsOk());
    ASSERT(col <= GetColumnCount());
    ASSERT(col == 0); // Only rename is possible

    model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
    if (canBeRenamed(node))
    {
        wxDataViewIconText icontext;
        icontext << variant;
        mProject->Submit(new command::ProjectViewRenameAsset(node, icontext.GetText()));
        return true;
    }
    return false;
}

bool ProjectViewModel::HasDefaultCompare() const
{
    return true;
}

int ProjectViewModel::Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const
{
    int result = 0;
    if (column == UINT_MAX)
    {
        // Default when no column header has been clicked
        column = 0;
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
        case 0:
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
        case 1:
            {
                wxDateTime dt1 = value1.GetDateTime();
                wxDateTime dt2 = value2.GetDateTime();
                if (dt1.IsEarlierThan(dt2))
                {
                    result = -1;
                }
                else if (dt1.IsLaterThan(dt2))
                {
                    result = 1;
                }
                else
                {
                    result = 0;
                }
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

bool ProjectViewModel::isAutomaticallyGenerated(model::NodePtr node) const
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

bool ProjectViewModel::isRoot(model::NodePtr node) const
{
    return !node->hasParent();
}

bool ProjectViewModel::isFolder(model::NodePtr node) const
{
    return boost::dynamic_pointer_cast<model::Folder>(node) != 0;
}

bool ProjectViewModel::isAutoFolder(model::NodePtr node) const
{
    return boost::dynamic_pointer_cast<model::AutoFolder>(node) != 0;
}

bool ProjectViewModel::isSequence(model::NodePtr node) const
{
    return boost::dynamic_pointer_cast<model::Sequence>(node) != 0;
}

bool ProjectViewModel::isDescendantOf(model::NodePtr node, model::NodePtr ascendant) const
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

bool ProjectViewModel::canBeRenamed(model::NodePtr node) const
{
    return !isRoot(node) && !isAutomaticallyGenerated(node) && !isAutoFolder(node);
}

wxIcon ProjectViewModel::getIcon(model::NodePtr node) const
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
    else
    {
        icon = mIconVideo;
    }
    return icon;
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void ProjectViewModel::onOpenProject( model::EventOpenProject &event )
{
    mProject = event.getValue();

    Cleared();

    gui::Window::get().Bind(model::EVENT_ADD_NODE,     &ProjectViewModel::onProjectAssetAdded,     this);
    gui::Window::get().Bind(model::EVENT_REMOVE_NODE,  &ProjectViewModel::onProjectAssetRemoved,   this);
    gui::Window::get().Bind(model::EVENT_RENAME_NODE,  &ProjectViewModel::onProjectAssetRenamed,   this);

    event.Skip();
}

void ProjectViewModel::onCloseProject( model::EventCloseProject &event )
{
    mProject = 0;

    Cleared();

    gui::Window::get().Unbind(model::EVENT_ADD_NODE,       &ProjectViewModel::onProjectAssetAdded,      this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_NODE,    &ProjectViewModel::onProjectAssetRemoved,    this);
    gui::Window::get().Unbind(model::EVENT_RENAME_NODE,    &ProjectViewModel::onProjectAssetRenamed,    this);

    event.Skip();
}

void ProjectViewModel::onProjectAssetAdded( model::EventAddNode &event )
{
    model::NodePtr parent = event.getValue().getParent();
    model::NodePtr child = event.getValue().getChild();
    VAR_DEBUG(parent)(child);
    ItemAdded(wxDataViewItem(parent->id()),wxDataViewItem(child->id()));

    mView.Expand(wxDataViewItem(parent->id()));

    event.Skip();
}

void ProjectViewModel::onProjectAssetRemoved( model::EventRemoveNode &event )
{
    model::NodePtr parent = event.getValue().getParent();
    model::NodePtr child = event.getValue().getChild();
    VAR_DEBUG(parent)(child);

    ItemDeleted(wxDataViewItem(parent->id()),wxDataViewItem(child->id()));
    event.Skip();
}

void ProjectViewModel::onProjectAssetRenamed( model::EventRenameNode &event )
{
    VAR_DEBUG(event.getValue().getNode());
    ItemChanged(event.getValue().getNode()->id());
    event.Skip();
}

DEFINE_EVENT(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, EventAutoFolderOpen, model::FolderPtr);

} // namespace

