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
#include "Dialog.h"
#include "File.h"
#include "Folder.h"
#include "ids.h"
#include "Node.h"
#include "Project.h"
#include "ProjectEvent.h"
#include "ProjectModification.h"
#include "ProjectViewAddAsset.h"
#include "ProjectViewClipboard.h"
#include "ProjectViewCommand.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateFile.h"
#include "ProjectViewCreateFolder.h"
#include "ProjectViewCreateSequence.h"
#include "ProjectViewDataObject.h"
#include "ProjectViewDeleteAsset.h"
#include "ProjectViewDeleteUnusedFiles.h"
#include "ProjectViewDropSource.h"
#include "ProjectViewMoveAsset.h"
#include "ProjectViewRenameAsset.h"
#include "Sequence.h"
#include "StatusBar.h"
#include "TimelinesView.h"
#include "UtilException.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilRecycle.h"
#include "UtilVector.h"
#include "Window.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectView::ProjectView(wxWindow* parent)
	: wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_STATIC)
	, mClipboard(new ProjectViewClipboard(*this))
	, mCtrl(this)
	, mModel(new ProjectViewModel(mCtrl))
	, mDropSource(mCtrl, *mModel)
	, mOpenFolders()
	, mDragCount(0)
	, mDragStart(0, 0)
	, mHeaderHeight(0)
{
    LOG_INFO;

    mCtrl.AssociateModel( mModel );
    mModel->DecRef();

    mCtrl.EnableDropTarget( ProjectViewDataObject::sFormat );
    wxDataViewColumn* nameColumn = mCtrl.AppendIconTextColumn("Name",       0, wxDATAVIEW_CELL_EDITABLE,    400, wxALIGN_LEFT,   wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );

    gui::Window::get().Bind(model::EVENT_OPEN_PROJECT,     &ProjectView::onOpenProject,             this);
    gui::Window::get().Bind(model::EVENT_CLOSE_PROJECT,    &ProjectView::onCloseProject,            this);

    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING,     &ProjectView::onStartEditing,    this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU,      &ProjectView::onContextMenu,     this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_DROP_POSSIBLE,     &ProjectView::onDropPossible,    this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_DROP,              &ProjectView::onDrop,            this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED,         &ProjectView::onActivated,       this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_EXPANDED,          &ProjectView::onExpanded,        this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_COLLAPSED,         &ProjectView::onCollapsed,       this);

    mCtrl.GetMainWindow()->Bind(wxEVT_MOTION,           &ProjectView::onMotion,          this);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add( &mCtrl, 1, wxGROW );
    sizer->Hide(&mCtrl);
    sizer->Layout();
    SetSizerAndFit(sizer);
}

ProjectView::~ProjectView()
{
	delete(mClipboard);
	mClipboard = 0;

    gui::Window::get().Unbind(model::EVENT_OPEN_PROJECT,       &ProjectView::onOpenProject,             this);
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,      &ProjectView::onCloseProject,            this);

    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING, &ProjectView::onStartEditing, this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU,    &ProjectView::onContextMenu,     this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_DROP_POSSIBLE,   &ProjectView::onDropPossible,    this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_DROP,            &ProjectView::onDrop,            this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED,       &ProjectView::onActivated,       this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_EXPANDED,        &ProjectView::onExpanded,        this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_COLLAPSED,       &ProjectView::onCollapsed,       this);

    mCtrl.GetMainWindow()->Unbind(wxEVT_MOTION,         &ProjectView::onMotion,          this);
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void ProjectView::onOpenProject(model::EventOpenProject &event)
{
    GetSizer()->Show(&mCtrl);
    GetSizer()->Layout();
    gui::Window::get().Bind(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, &ProjectView::onAutoOpenFolder, this);

    // Rancid, but working... Determine the height of the (well, 'a') header
    // Code specifically put here: Originally it was included in the 'findNode' method (where it is used).
    // However, that method is often run in a different (non GUI) thread, causing problems.
    //
    // Then, i put the code in the constructor. However, that caused (sometimes) issues when the first
    // Idle event was received by the wxdataviewctrl.
    //
    //wxDialog* win = new wxDialog(this,-1,"Dummy");
    wxHeaderCtrlSimple* s = new wxHeaderCtrlSimple(this);
    wxHeaderColumnSimple col("Title");
    s->AppendColumn(col);
    mHeaderHeight = s->GetSize().GetHeight();
    s->Destroy();

    event.Skip();
}

void ProjectView::onCloseProject(model::EventCloseProject &event)
{
    GetSizer()->Hide(&mCtrl);
    GetSizer()->Layout();
    gui::Window::get().Unbind(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, &ProjectView::onAutoOpenFolder, this);
    mCtrl.UnselectAll(); // To avoid crashes when directly loading a new project.
    mOpenFolders.clear(); // Release shared_ptrs
    event.Skip();
}

void ProjectView::onAutoOpenFolder(EventAutoFolderOpen& event)
{
    if (UtilVector<model::FolderPtr>(mOpenFolders).hasElement(event.getValue()))
    {
        expand(event.getValue());
    }
}

//////////////////////////////////////////////////////////////////////////
// SELECTION
//////////////////////////////////////////////////////////////////////////

void ProjectView::select(const model::NodePtrs& nodes)
{
    ASSERT(wxThread::IsMain());
    mCtrl.UnselectAll();
    for ( model::NodePtr node : nodes )
    {
        VAR_DEBUG(node->id());
        mCtrl.Select( wxDataViewItem( node->id() ) );
    }
    ASSERT_EQUALS(mCtrl.GetSelectedItemsCount(), nodes.size());
}

void ProjectView::selectAll()
{
    ASSERT(wxThread::IsMain());
    mCtrl.SelectAll();
}

model::FolderPtr ProjectView::getSelectedContainerOrRoot() const
{
    switch (mCtrl.GetSelectedItemsCount())
    {
        case 0:
            return model::Project::get().getRoot();
        case 1:
            wxDataViewItemArray selection;
            mCtrl.GetSelections(selection);
            model::NodePtr projectNode = model::INode::Ptr(static_cast<model::NodeId>(selection[0].GetID()));
            model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(projectNode);
            if (folder)
            {
                return folder;
            }
    }
    return model::FolderPtr();
}

model::FolderPtr ProjectView::getSelectedContainer() const
{
    wxDataViewItemArray selection;
    mCtrl.GetSelections(selection);
    ASSERT_EQUALS(selection.size(),1);
    model::NodePtr projectNode = model::INode::Ptr(static_cast<model::NodeId>(selection[0].GetID()));
    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(projectNode);
    ASSERT(folder);
    return folder;
}

bool ProjectView::hasSelection() const
{
    return mCtrl.HasSelection();
}

model::NodePtrs ProjectView::getSelection() const
{
    model::NodePtrs l;
    wxDataViewItemArray selection;
    mCtrl.GetSelections(selection);

    for (wxDataViewItem wxItem : selection)
    {
        model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
        ASSERT_NONZERO(node);
        l.push_back(node);
    }
    return l;
}

bool ProjectView::selectionContainsRootNode() const
{
    for (model::NodePtr node : getSelection())
    {
        if (!node->hasParent())
        {
            return true;
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxPoint ProjectView::find(const model::NodePtr& node )
{
    wxPoint result;
    wxDataViewItem item =  wxDataViewItem( node->id() );

    wxPoint headerAdjust(0,mHeaderHeight);
    wxRect rect = mCtrl.GetItemRect(item, 0);

    return wxPoint(rect.GetX() + rect.GetWidth() / 2, rect.GetY() + rect.GetHeight() / 2) + headerAdjust;
    //for (int i = 0; i < mCtrl.GetSize().GetHeight(); ++i)
    //{
    //    wxDataViewItem wxItem;
    //    wxDataViewColumn* column = 0;
    //    wxPoint point(40,i);
    //    mCtrl.HitTest(point, wxItem, column);

    //    if (wxItem.IsOk())
    //    {
    //        model::NodePtr found = model::Node::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
    //        if (found == node)
    //        {
    //            return point + headerAdjust;
    //        }
    //    }
    //}
    //FATAL;
    //return wxPoint(0,0);
}

void ProjectView::expand(const model::NodePtr& node)
{
    mCtrl.Expand(wxDataViewItem(node->id() ));
}

void ProjectView::setOpenFolders(std::vector<model::FolderPtr> folders)
{
    mOpenFolders = folders;
}

void ProjectView::scrollToRight()
{
#ifdef _MSC_VER
    mCtrl.ScrollWindow(mCtrl.GetSize().GetWidth(), -1);
#else
    ScrollWindow(GetSize().GetWidth(), -1);
#endif
}

bool ProjectView::findConflictingName(const model::FolderPtr& parent, const wxString& name, const NodeType& type)
{
    for ( model::NodePtr child : parent->getChildren() )
    {
        if (child->getName().IsSameAs(name))
        {
            bool isSameType = true;
            wxString prefix;
            wxString nodetype;
            switch (type)
            {
            case NODETYPE_FILE:
                isSameType = (child->isA<model::File>());
                nodetype = _("file");
                break;
            case NODETYPE_FOLDER:
                isSameType = (child->isA<model::Folder>());
                nodetype = _("folder");
                break;
            case NODETYPE_SEQUENCE:
                isSameType = (child->isA<model::Sequence>());
                nodetype = _("sequence");
                break;
            default:
                break;
            }
            if (isSameType)
            {
                gui::Dialog::get().getConfirmation(_("Duplicate exists"), wxString::Format(_("A %1$s with name %2$s already exists"), nodetype, name));
                return true;
            }
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// POPUP MENU
//////////////////////////////////////////////////////////////////////////

void ProjectView::onDelete()
{
    model::ProjectModification::submitIfPossible(new cmd::ProjectViewDeleteAsset(getSelection()));
}

void ProjectView::onDeleteUnused()
{
    model::AutoFolderPtr folder = boost::dynamic_pointer_cast<model::AutoFolder>(getSelectedContainer());
    ASSERT(folder);

    cmd::ProjectViewDeleteUnusedFiles(folder).recycleFiles();
    // To ensure that starting this again doesn't use files that no longer exist:
    folder->check(true);
}

void ProjectView::onNewFolder(const model::FolderPtr& parent)
{
    wxString s = gui::Dialog::get().getText(_("Create folder"), _("Enter name"), _("Folder") );
    if ((s.CompareTo(_T("")) != 0) &&
        (!findConflictingName(parent, s, NODETYPE_FOLDER)))
    {
        model::ProjectModification::submit(new cmd::ProjectViewCreateFolder(parent, s));
    }
}

void ProjectView::onNewAutoFolder(const model::FolderPtr& parent)
{
    wxString s = gui::Dialog::get().getDir(_("Select folder"), wxStandardPaths::Get().GetDocumentsDir());
    if ((s.CompareTo(_T("")) != 0) &&
        (!findConflictingName(parent, s, NODETYPE_FOLDER)))
    {
        wxFileName path(s,"");
        path.Normalize();
        model::ProjectModification::submit(new cmd::ProjectViewCreateAutoFolder(parent, path));
    }
}

void ProjectView::onNewAutoFolderInRoot()
{
    onNewAutoFolder(model::Project::get().getRoot());
}

void ProjectView::onNewSequence(const model::FolderPtr& parent)
{
    wxString s = gui::Dialog::get().getText(_("Create sequence"), _("Enter name"), _("Sequence") );
    if ((s.CompareTo(_T("")) != 0) &&
        (!findConflictingName(parent, s, NODETYPE_SEQUENCE)))
    {
        model::ProjectModification::submit(new cmd::ProjectViewCreateSequence(parent, s));
    }
}

void ProjectView::onNewSequenceInRoot()
{
    onNewSequence(model::Project::get().getRoot());
}

void ProjectView::onNewFile(const model::FolderPtr& parent)
{
    wxStrings files = gui::Dialog::get().getFiles( _("Select file(s)") );
    std::vector<wxFileName> list;
    for ( wxString path : files )
    {
        if (findConflictingName(parent,path, NODETYPE_FILE))
        {
            return;
        }
        wxFileName filename(path);
        filename.Normalize();
        model::FilePtr file = boost::make_shared<model::File>(filename);
        if (file->canBeOpened())
        {
            list.push_back(filename);
        }
    }
    if (list.size() > 0 )
    {
        model::ProjectModification::submit(new cmd::ProjectViewCreateFile(parent, list));
    }
}

void ProjectView::onNewFileInRoot()
{
    onNewFile(model::Project::get().getRoot());
}

void ProjectView::onCreateSequence()
{
    cmd::ProjectViewCreateSequence* cmd = new cmd::ProjectViewCreateSequence(getSelectedContainer());
    if (!findConflictingName(cmd->getParent(), cmd->getName(), NODETYPE_SEQUENCE))
    {
        model::ProjectModification::submit(cmd);
    }
    else
    {
        delete cmd;
    }
}

void ProjectView::onOpen()
{
    for ( model::NodePtr node : getSelection() )
    {
        if (node->isA<model::Sequence>())
        {
            gui::TimelinesView::get().Open(boost::dynamic_pointer_cast<model::Sequence>(node));
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void ProjectView::onContextMenu(wxDataViewEvent &event)
{
    CatchExceptions([this]
    {
        wxDataViewItemArray sel;
        int nSelected = mCtrl.GetSelections(sel);

        // Mechanism:
        // Default menu options are hidden and enabled.
        // If an item is selected for which a menu option makes sense, then the option is shown.
        // If an item is selected for which a menu option does not make sense, then the option is disabled.

        bool showNew = true;
        bool showCreateSequence = false;
        bool showOpenSequence = false;
        bool showDeleteUnused = false;

        bool enableUpdateAutoFolder = true;

        bool enableNew = (nSelected == 1);
        bool enableDelete = (nSelected >= 1);
        bool enablePaste = (nSelected == 1);
        bool enableCreateSequence = (nSelected == 1);
        bool enableOpen = (nSelected == 1);
        bool enableDeleteUnused = (nSelected == 1);

        for (wxDataViewItem item : sel)
        {
            model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(item.GetID()));

            bool isRoot = (!node->hasParent());
            bool isFolder = boost::dynamic_pointer_cast<model::Folder>(node) != nullptr;
            bool isAutoFolder = boost::dynamic_pointer_cast<model::AutoFolder>(node) != nullptr;
            bool isSequence = boost::dynamic_pointer_cast<model::Sequence>(node) != nullptr;

            if (isRoot)
            {
                enableDelete = false;
                showNew = true;
                enableCreateSequence = false;
            }
            else if (isAutoFolder)
            {
                // Must be handled before 'folder' since an autofolder is also a folder
                showCreateSequence = true;
                showDeleteUnused = true;

                enablePaste = false;
                enableNew = false;
            }
            else if (isFolder)
            {
                showNew = true;
                showCreateSequence = true;
            }
            else if (isSequence)
            {
                showOpenSequence = true;
                enablePaste = false;
                enableNew = false;
                enableCreateSequence = false;
            }
            else
            {
                enablePaste = false;
                enableNew = false;
                enableCreateSequence = false;
            }

            if (mModel->isAutomaticallyGenerated(node))
            {
                enableDelete = false;
                enablePaste = false;
            }
        }

        wxMenu menu;

        menu.Append(wxID_CUT, _("Cut") + "\t" + _("CTRL") + "-X");
        menu.Enable(wxID_CUT, enableDelete);
        menu.Append(wxID_COPY, _("Copy") + "\t" + _("CTRL") + "-C");
        menu.Enable(wxID_COPY, (nSelected > 0));
        menu.Append(wxID_PASTE, _("Paste") + "\t" + _("CTRL") + "-V");
        menu.Enable(wxID_PASTE, enablePaste);
        menu.AppendSeparator();
        menu.Append(wxID_DELETE, _("Delete") + "\t" +_("DEL"));
        menu.Enable(wxID_DELETE, enableDelete);
        if (showDeleteUnused)
        {
            menu.Append(ID_DELETE_UNUSED, _("Delete unused files on disk"));
            menu.Enable(ID_DELETE_UNUSED, enableDeleteUnused);
        }
        menu.AppendSeparator();

        if (showCreateSequence)
        {
            menu.Append(ID_CREATE_SEQUENCE, _("Make sequence"));
            menu.Enable(ID_CREATE_SEQUENCE, enableCreateSequence);
        }
        if (showOpenSequence)
        {
            menu.Append(wxID_OPEN, _("Open sequence"));
            menu.Enable(wxID_OPEN, enableOpen);
        }

        if (showNew && enableNew)
        {
            menu.AppendSeparator();

            wxMenu* addMenu = new wxMenu();
            addMenu->Append(ID_NEW_AUTOFOLDER, _("Folder from disk"));
            addMenu->Append(ID_NEW_FILES, _("File(s) from disk"));
            menu.AppendSubMenu(addMenu, _("Add"));

            menu.AppendSeparator();

            wxMenu* createMenu = new wxMenu();
            createMenu->Append(ID_NEW_FOLDER, _("Folder"));
            createMenu->Append(ID_NEW_SEQUENCE, _("Sequence"));
            menu.AppendSubMenu(createMenu, _("New"));
        }

        int result = GetPopupMenuSelectionFromUser(menu);
        switch (result)
        {
            case wxID_NONE:                                                      break;
            case wxID_CUT:              mClipboard->onCut();                     break;
            case wxID_COPY:             mClipboard->onCopy();                    break;
            case wxID_PASTE:            mClipboard->onPaste();                   break;
            case wxID_DELETE:           onDelete();                              break;
            case ID_DELETE_UNUSED:      onDeleteUnused();                        break;
            case ID_NEW_FOLDER:         onNewFolder(getSelectedContainer());     break;
            case ID_NEW_AUTOFOLDER:     onNewAutoFolder(getSelectedContainer()); break;
            case ID_NEW_SEQUENCE:       onNewSequence(getSelectedContainer());   break;
            case ID_NEW_FILES:          onNewFile(getSelectedContainer());       break;
            case ID_CREATE_SEQUENCE:    onCreateSequence();                      break;
            case wxID_OPEN:             onOpen();                                break;
        }
    });
}

void ProjectView::onMotion(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        if (event.Dragging())
        {
            if (mDragCount == 0)
            {
                mDragStart = event.GetPosition();
                mDragCount++;
            }
            else if (mDragCount == 3)
            {
                if (event.LeftIsDown())
                {
                    wxDataViewItem item;
                    wxDataViewColumn* col;
                    mCtrl.HitTest(mDragStart, item, col);
                    if (item.GetID() && 
                        !getSelection().empty())
                    {
                        ProjectViewDataObject data(cmd::ProjectViewCommand::prune(getSelection()));
                        mDropSource.startDrag(data);
                        mDragCount = 0;
                    }
                }
            }
            else
            {
                mDragCount++;
            }
        }
        else
        {
            mDragCount = 0;
        }
        event.Skip();
    });
}

void ProjectView::onDropPossible(wxDataViewEvent &event)
{
    CatchExceptions([this, &event]
    {
        // Can only drop relevant type of info
        if (event.GetDataFormat().GetId() != ProjectViewDataObject::sFormat)
        {
            event.Veto();
            return;
        }

        // Cannot drop if there's no item
        if (event.GetItem() == nullptr)
        {
            event.Veto();
            return;
        }

        // Cannot drop into an autofolder tree
        model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
        if (mModel->isAutomaticallyGenerated(p) || !mModel->isFolder(p) || mModel->isAutoFolder(p))
        {
            event.Veto();
            return;
        }

        // Cannot drop a node into itselves, or one of its children
        for (model::NodePtr node : mDropSource.getData().getNodes())
        {
            if (p == node || mModel->isDescendantOf(p, node))
            {
                event.Veto();
                return;
            }

            if (mModel->isAutomaticallyGenerated(node))
            {
                event.Veto();
                return;
            }
        }
        event.Skip();
    });
}

void ProjectView::onDrop(wxDataViewEvent &event)
{
    LOG_INFO;

    CatchExceptions([this, &event]
    {
        if (event.GetDataFormat().GetId() != ProjectViewDataObject::sFormat)
        {
            event.Veto();
            return;
        }
        model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
        if (mModel->isAutomaticallyGenerated(p) || !mModel->isFolder(p) || mModel->isAutoFolder(p))
        {
            event.Veto();
            return;
        }

        model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(p);
        ASSERT(folder);

        bool conflictingChildExists = false;
        for (model::NodePtr node : mDropSource.getData().getNodes())
        {
            if (findConflictingName(folder, node->getName(), NODETYPE_ANY))
            {
                event.Veto();
                return;
            }
        }
        model::ProjectModification::submitIfPossible(new cmd::ProjectViewMoveAsset(mDropSource.getData().getNodes(), p));
        event.Skip();
    });
}

void ProjectView::onActivated(wxDataViewEvent &event)
{
    CatchExceptions([this, &event]
    {
        model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
        model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(p);
        if (!sequence)
        {
            event.Veto();
            return;
        }
        Window::get().getTimeLines().Open(sequence);
        event.Skip();
    });
}

void ProjectView::onExpanded(wxDataViewEvent &event)
{
    CatchExceptions([this, &event]
    {
        model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
        model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(p);
        ASSERT(folder);
        if (!UtilVector<model::FolderPtr>(mOpenFolders).hasElement(folder))
        {
            mOpenFolders.push_back(folder);
        }
        event.Skip();
    });
}

void ProjectView::onCollapsed(wxDataViewEvent &event)
{
    CatchExceptions([this, &event]
    {
        model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
        model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(p);
        ASSERT(folder);
        ASSERT(UtilVector<model::FolderPtr>(mOpenFolders).hasElement(folder));
        UtilVector<model::FolderPtr>(mOpenFolders).removeElements({ folder });
        event.Skip();
    });
}

void ProjectView::onStartEditing(wxDataViewEvent &event)
{
    CatchExceptions([this, &event]
    {
        model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
        if (!mModel->canBeRenamed(node))
        {
            event.Veto();
        }
        event.Skip();
    });
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void ProjectView::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_NVP(mOpenFolders);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void ProjectView::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void ProjectView::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} // namespace
