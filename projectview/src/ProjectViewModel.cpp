#include "ProjectView.h"

#include <wx/datetime.h>
#include <boost/foreach.hpp>
#include "UtilLog.h"
#include "Project.h"
#include "AProjectViewNode.h"
#include "ProjectViewAddAsset.h"
#include "ProjectViewDeleteAsset.h"
#include "ProjectViewMoveAsset.h"
#include "ProjectViewRenameAsset.h"
#include "folder-horizontal.xpm"
#include "folder-horizontal-open.xpm"
#include "folder-horizontal-plus.xpm"
#include "folder-horizontal-plus-open.xpm"
#include "film.xpm" 
#include "Folder.h"
#include "AutoFolder.h"
#include "File.h"
#include "Sequence.h"
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
    gui::Window::get().Bind(model::EVENT_OPEN_PROJECT,     &ProjectViewModel::OnOpenProject,           this);
    gui::Window::get().Bind(model::EVENT_CLOSE_PROJECT,    &ProjectViewModel::OnCloseProject,          this);
}

ProjectViewModel::~ProjectViewModel()
{
    gui::Window::get().Unbind(model::EVENT_OPEN_PROJECT,   &ProjectViewModel::OnOpenProject,            this);
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,  &ProjectViewModel::OnCloseProject,           this);
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
        return isFolder(model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(wxItem.GetID())));
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
        model::ProjectViewPtr p = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(wxItem.GetID()));
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

    model::ProjectViewPtr parent = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(wxItem.GetID()));
    BOOST_FOREACH( model::ProjectViewPtr child, parent->getChildren() )
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

    model::ProjectViewPtr node = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(wxItem.GetID()));
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
                wxDateTime t;
                t.ParseDateTime(file->getLastModified());
                variant = t;
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

    model::ProjectViewPtr node = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(wxItem.GetID()));
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

    model::ProjectViewPtr node1 = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(item1.GetID()));
    model::ProjectViewPtr node2 = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(item2.GetID()));

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

bool ProjectViewModel::isAutomaticallyGenerated(model::ProjectViewPtr node) const
{
    if (!node->hasParent())
    {
        return false;
    }
    else
    {
        model::ProjectViewPtr parent = node->getParent();
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

bool ProjectViewModel::isRoot(model::ProjectViewPtr node) const
{
    return !node->hasParent();
}

bool ProjectViewModel::isFolder(model::ProjectViewPtr node) const
{
    return boost::dynamic_pointer_cast<model::Folder>(node) != 0;
}

bool ProjectViewModel::isAutoFolder(model::ProjectViewPtr node) const
{
    return boost::dynamic_pointer_cast<model::AutoFolder>(node) != 0;
}

bool ProjectViewModel::isSequence(model::ProjectViewPtr node) const
{
    return boost::dynamic_pointer_cast<model::Sequence>(node) != 0;
}

bool ProjectViewModel::isDescendantOf(model::ProjectViewPtr node, model::ProjectViewPtr ascendant) const
{
    if (!node->hasParent())
    {
        // this node is an orphan. It might be the root (project) node, or it was deleted/cut.
        return false;
    }
    model::ProjectViewPtr parent = node->getParent();
    if (parent == ascendant)
    {
        return true;
    }
    return isDescendantOf(parent, ascendant);
}

bool ProjectViewModel::canBeRenamed(model::ProjectViewPtr node) const
{
    return !isRoot(node) && !isAutomaticallyGenerated(node) && !isAutoFolder(node);
}

wxIcon ProjectViewModel::getIcon(model::ProjectViewPtr node) const
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

void ProjectViewModel::OnOpenProject( model::EventOpenProject &event )
{
    mProject = event.getValue();

    Cleared();

    gui::Window::get().Bind(model::EVENT_ADD_ASSET, &ProjectViewModel::OnProjectAssetAdded,         this);
    gui::Window::get().Bind(model::EVENT_REMOVE_ASSET,  &ProjectViewModel::OnProjectAssetRemoved,   this);
    gui::Window::get().Bind(model::EVENT_RENAME_ASSET,  &ProjectViewModel::OnProjectAssetRenamed,   this);

    event.Skip();
}

void ProjectViewModel::OnCloseProject( model::EventCloseProject &event )
{
    mProject = 0;

    Cleared();

    gui::Window::get().Unbind(model::EVENT_ADD_ASSET,       &ProjectViewModel::OnProjectAssetAdded,      this);
    gui::Window::get().Unbind(model::EVENT_REMOVE_ASSET,    &ProjectViewModel::OnProjectAssetRemoved,    this);
    gui::Window::get().Unbind(model::EVENT_RENAME_ASSET,    &ProjectViewModel::OnProjectAssetRenamed,    this);

    event.Skip();
}

void ProjectViewModel::AddRecursive( model::ProjectViewPtr node)
{
    BOOST_FOREACH( model::ProjectViewPtr child, node->getChildren() )
    {
        VAR_DEBUG(node)(child);
        ItemAdded(wxDataViewItem(node->id()),wxDataViewItem(child->id()));
        AddRecursive(child);
    }
}

void ProjectViewModel::OnProjectAssetAdded( model::EventAddAsset &event )
{
    model::ProjectViewPtr parent = event.getValue().parent;
    model::ProjectViewPtr child = event.getValue().child;
    VAR_DEBUG(parent)(child);
    ItemAdded(wxDataViewItem(parent->id()),wxDataViewItem(child->id()));

    // This is needed to avoid errors with Undo-ing:
    // - Add a folder A
    // - In A, add a node X
    // - Now, delete A (only select A, then right mouse, delete)
    // - Undo -> The delete is undone, folder A is restored
    // - At this point, Undoing the add X (being a file, folder, whatever) will cause a crash.
    //   That's because of when deleting a subtree only the parent is removed (all children
    //   remain under that parent, just no longer in the main tree). Subsequently, when restoring
    //   that subtree (undo delete) all children are still under folder A. However, the dataviewctrl
    //   is not informed (since these nodes aren't added under folder A anymore).
    // - Note that opening folder A before doing the undo of 'add X' would also cause X to be known
    //   to the control (via 'GetChildren') and thus avoid the crash.
    // As a resolution, all child nodes are made known to the control here.
    AddRecursive(child);

    mView.Expand(wxDataViewItem(parent->id()));

    event.Skip();
}

void ProjectViewModel::OnProjectAssetRemoved( model::EventRemoveAsset &event )
{
    model::ProjectViewPtr parent = event.getValue().parent;
    model::ProjectViewPtr child = event.getValue().child;
    VAR_DEBUG(parent)(child);

    ItemDeleted(wxDataViewItem(parent->id()),wxDataViewItem(child->id()));
    event.Skip();
}

void ProjectViewModel::OnProjectAssetRenamed( model::EventRenameAsset &event )
{
    VAR_DEBUG(event.getValue().node);
    ItemChanged(event.getValue().node->id());
    event.Skip();
}

DEFINE_EVENT(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, EventAutoFolderOpen, model::FolderPtr);

} // namespace

