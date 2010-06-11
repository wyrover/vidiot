#include "GuiProjectView.h"

#include "wxInclude.h"
#include <wx/datetime.h>
#include <boost/foreach.hpp>
#include "GuiMain.h"
#include "UtilLog.h"
#include "Project.h"
#include "AProjectViewNode.h"
#include "ProjectCommandAddAsset.h"
#include "ProjectCommandDeleteAsset.h"
#include "ProjectCommandMoveAsset.h"
#include "ProjectCommandRenameAsset.h"
#include "ProjectEventOpenProject.h"
#include "ProjectEventCloseProject.h"
#include "ProjectEventAddAsset.h"
#include "ProjectEventDeleteAsset.h"
#include "ProjectEventRenameAsset.h"
#include "folder-horizontal.xpm"
#include "folder-horizontal-open.xpm"
#include "folder-horizontal-plus.xpm"
#include "folder-horizontal-plus-open.xpm"
#include "film.xpm" 
#include "Folder.h"
#include "AutoFolder.h"
#include "File.h"
#include "Sequence.h"

GuiProjectViewModel::GuiProjectViewModel(wxDataViewCtrl& view)
:   wxDataViewModel()
,   mView(view)
,   mProject(0)
,   mIconAutoFolder(folder_horizontal_plus_xpm)
,   mIconAutoFolderOpen(folder_horizontal_plus_open_xpm)
,   mIconFolder(folder_horizontal_xpm)
,   mIconFolderOpen(folder_horizontal_open_xpm)
,	mIconVideo(film_xpm)
{
    wxGetApp().Bind(PROJECT_EVENT_OPEN_PROJECT,     &GuiProjectViewModel::OnOpenProject,           this);
    wxGetApp().Bind(PROJECT_EVENT_CLOSE_PROJECT,    &GuiProjectViewModel::OnCloseProject,          this);
}

GuiProjectViewModel::~GuiProjectViewModel()
{
    wxGetApp().Unbind(PROJECT_EVENT_OPEN_PROJECT,   &GuiProjectViewModel::OnOpenProject,            this);
    wxGetApp().Unbind(PROJECT_EVENT_CLOSE_PROJECT,  &GuiProjectViewModel::OnCloseProject,           this);
}

//////////////////////////////////////////////////////////////////////////
// OVERRIDES FROM WXDATAVIEWMODEL
//////////////////////////////////////////////////////////////////////////

bool GuiProjectViewModel::IsContainer (const wxDataViewItem &wxItem) const
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

wxDataViewItem GuiProjectViewModel::GetParent( const wxDataViewItem &wxItem ) const
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
            return wxDataViewItem(0);
        }
        else
        {
            return wxDataViewItem(p->getParent()->id());
        }
    }
}

unsigned int GuiProjectViewModel::GetChildren( const wxDataViewItem &wxItem, wxDataViewItemArray &wxItemArray ) const
{
    if (!wxItem.IsOk()) 
    {
        if (mProject != 0)
        {
            wxItemArray.Add(wxDataViewItem(mProject->getRoot()->id()));
            wxGetApp().QueueEvent(new FolderEvent(mProject->getRoot()));
            return 1;
        }
        else
        {
            // No project opened
            return 0;
        }
    }

    // First, fill the list with the already known children
    model::ProjectViewPtr parent = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(wxItem.GetID()));
    BOOST_FOREACH( model::ProjectViewPtr child, parent->getChildren() )
    {
        wxItemArray.Add(wxDataViewItem(child->id()));

        model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(child);
        if (folder)
        {
            wxGetApp().QueueEvent(new FolderEvent(folder));
        }
    }

    // Third, update the children. These will be added via events resulting from the update.
    model::AutoFolderPtr autofolder = boost::dynamic_pointer_cast<model::AutoFolder>(parent);
    if (autofolder)
    {
        autofolder->update();
    }

    return wxItemArray.GetCount();
}

unsigned int GuiProjectViewModel::GetColumnCount() const
{
    return 3;
}

wxString GuiProjectViewModel::GetColumnType(unsigned int col) const
{
    ASSERT(col <= GetColumnCount());
    switch (col)
    {
    case 0: return wxT("icontext");
    case 1: return wxT("string");
    case 2: return wxT("string");
    }
    return wxT("string");
}

void GuiProjectViewModel::GetValue( wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col ) const
{
    ASSERT(wxItem.IsOk());
    ASSERT(col <= GetColumnCount());

    model::ProjectViewPtr node = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(wxItem.GetID()));
    switch (col)
    {
    case 0: 
        {
            wxDataViewIconText icontext(node->getName());
            if (isFolder(node))
            {
                bool open = (mView.IsExpanded(wxItem));
                if (isAutoFolder(node))
                {
                    icontext.SetIcon(open?mIconAutoFolderOpen:mIconAutoFolder);
                }
                else
                {
                    icontext.SetIcon(open?mIconFolderOpen:mIconFolder);
                }
            }
            else
            {
                icontext.SetIcon(mIconVideo);
            }
            variant << icontext;//= wxString(node->getName()); 
            return;
        }
    case 1: 
        //variant = wxString(node->getPath().GetFullPath()); todo
        return;
    case 2: 
        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(node);
        if (file)
        {
            wxDateTime t;
            t.ParseDateTime(file->getLastModified());
            variant = t;
        }
        else
        {
            variant = wxString(""); return;
        }
    };
}

bool GuiProjectViewModel::SetValue( const wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col )
{
    ASSERT(wxItem.IsOk());
    ASSERT(col <= GetColumnCount());
    ASSERT(col == 0); // Only rename is possible

    model::ProjectViewPtr node = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(wxItem.GetID()));
    if (canBeRenamed(node))
    {
        wxDataViewIconText icontext;
        icontext << variant;
        mProject->Submit(new ProjectCommandRenameAsset(node, icontext.GetText()));
        return true;
    }
    return false;
}

bool GuiProjectViewModel::HasDefaultCompare() const
{
    return true;
}

int GuiProjectViewModel::Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const
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
                wxString str1 = value1.GetString();
                wxString str2 = value2.GetString();
                result = str1.CmpNoCase(str2);
                break;
            }
        case 2:
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

bool GuiProjectViewModel::isAutomaticallyGenerated(model::ProjectViewPtr node) const
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

bool GuiProjectViewModel::isRoot(model::ProjectViewPtr node) const
{
    return !node->hasParent();
}

bool GuiProjectViewModel::isFolder(model::ProjectViewPtr node) const
{
    return boost::dynamic_pointer_cast<model::Folder>(node) != 0;
}

bool GuiProjectViewModel::isAutoFolder(model::ProjectViewPtr node) const
{
    return boost::dynamic_pointer_cast<model::AutoFolder>(node) != 0;
}

bool GuiProjectViewModel::isSequence(model::ProjectViewPtr node) const
{
    return boost::dynamic_pointer_cast<model::Sequence>(node) != 0;
}

bool GuiProjectViewModel::isDescendantOf(model::ProjectViewPtr node, model::ProjectViewPtr ascendant) const
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

bool GuiProjectViewModel::canBeRenamed(model::ProjectViewPtr node) const
{
    return !isRoot(node) && !isAutomaticallyGenerated(node) && !isAutoFolder(node);
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiProjectViewModel::OnOpenProject( ProjectEventOpenProject &event )
{
    mProject = event.getProject();

    Cleared();

    wxGetApp().Bind(PROJECT_EVENT_ADD_ASSET,        &GuiProjectViewModel::OnProjectAssetAdded,      this);
    wxGetApp().Bind(PROJECT_EVENT_DELETE_ASSET,     &GuiProjectViewModel::OnProjectAssetDeleted,    this);
    wxGetApp().Bind(PROJECT_EVENT_RENAME_ASSET,     &GuiProjectViewModel::OnProjectAssetRenamed,    this);

    event.Skip();
}

void GuiProjectViewModel::OnCloseProject( ProjectEventCloseProject &event )
{
    mProject = 0;

    Cleared();

    wxGetApp().Unbind(PROJECT_EVENT_ADD_ASSET,      &GuiProjectViewModel::OnProjectAssetAdded,      this);
    wxGetApp().Unbind(PROJECT_EVENT_DELETE_ASSET,   &GuiProjectViewModel::OnProjectAssetDeleted,    this);
    wxGetApp().Unbind(PROJECT_EVENT_RENAME_ASSET,   &GuiProjectViewModel::OnProjectAssetRenamed,    this);

    event.Skip();
}

void GuiProjectViewModel::AddRecursive( model::ProjectViewPtr node)
{
    BOOST_FOREACH( model::ProjectViewPtr child, node->getChildren() )
    {
        VAR_DEBUG(node)(child);
        ItemAdded(wxDataViewItem(node->id()),wxDataViewItem(child->id()));
        AddRecursive(child);
    }
}

void GuiProjectViewModel::OnProjectAssetAdded( ProjectEventAddAsset &event )
{
    VAR_DEBUG(event.getParent())(event.getNode());
    ItemAdded(wxDataViewItem(event.getParent()->id()),wxDataViewItem(event.getNode()->id()));

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
    AddRecursive(event.getNode());

    mView.Expand(wxDataViewItem(event.getParent()->id()));

    event.Skip();
}

void GuiProjectViewModel::OnProjectAssetDeleted( ProjectEventDeleteAsset &event )
{
    VAR_DEBUG(event.getParent())(event.getNode());
    ItemDeleted(wxDataViewItem(event.getParent()->id()),wxDataViewItem(event.getNode()->id()));
    event.Skip();
}

void GuiProjectViewModel::OnProjectAssetRenamed( ProjectEventRenameAsset &event )
{
    VAR_DEBUG(event.getNode());
    ItemChanged(event.getNode()->id());
    event.Skip();
}

GuiProjectViewModel::FolderEvent::FolderEvent(model::FolderPtr folder)
:   wxEvent(wxID_ANY, GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER)
,   mFolder(folder)
{
}

GuiProjectViewModel::FolderEvent::FolderEvent(const GuiProjectViewModel::FolderEvent& other)
:   wxEvent(other)
,   mFolder(other.mFolder)
{
}

wxEvent* GuiProjectViewModel::FolderEvent::Clone() const 
{ 
    return new FolderEvent(*this); 
}

model::FolderPtr GuiProjectViewModel::FolderEvent::getFolder() const
{
    return mFolder;
}

wxDEFINE_EVENT(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, GuiProjectViewModel::FolderEvent);

