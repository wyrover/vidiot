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

#include "ProjectView.h"

#include "AutoFolder.h"
#include "DataObject.h"
#include "Dialog.h"
#include "File.h"
#include "Folder.h"
#include "ids.h"
#include "Layout.h"
#include "Node.h"
#include "ProjectEvent.h"
#include "ProjectModification.h"
#include "ProjectViewAddAsset.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateFile.h"
#include "ProjectViewCreateFolder.h"
#include "ProjectViewCreateSequence.h"
#include "ProjectViewDeleteAsset.h"
#include "ProjectViewDeleteUnusedFiles.h"
#include "ProjectViewDropSource.h"
#include "ProjectViewMoveAsset.h"
#include "ProjectViewRenameAsset.h"
#include "Sequence.h"
#include "TimeLinesView.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilRecycle.h"
#include "Window.h"

namespace gui {

enum
{
    ID_NEW_FOLDER = wxID_HIGHEST + 1,
    ID_NEW_AUTOFOLDER,
    ID_NEW_SEQUENCE,
    ID_NEW_FILE,
    ID_CREATE_SEQUENCE,
    ID_DELETE_UNUSED,
};

// This method only to be used here. Enum too.
enum NodeType
{
    NODETYPE_ANY,
    NODETYPE_FILE,
    NODETYPE_FOLDER,
    NODETYPE_SEQUENCE
};
bool findConflictingName(wxWindow* window, model::FolderPtr parent, wxString name, NodeType type );

    //////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectView::ProjectView(wxWindow* parent)
    :   wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_STATIC)
    ,   mProject(0)
    ,   mCtrl(this)
    ,   mModel(new ProjectViewModel(mCtrl))
    ,   mDropSource(mCtrl, *mModel)
    ,   mOpenFolders()
    ,   mDragCount(0)
    ,   mDragStart(0,0)
    ,   mHeaderHeight(0)
{
    LOG_INFO;

    mCtrl.AssociateModel( mModel );
    mModel->DecRef();

    mCtrl.EnableDropTarget( DataObject::sFormat );
    wxDataViewColumn* nameColumn = mCtrl.AppendIconTextColumn("Name",       0, wxDATAVIEW_CELL_EDITABLE,    200, wxALIGN_LEFT,   wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );
    wxDataViewColumn* dateColumn = mCtrl.AppendTextColumn("Modified",   1, wxDATAVIEW_CELL_INERT,       -1, wxALIGN_RIGHT,  wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );

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
    gui::Window::get().Unbind(model::EVENT_OPEN_PROJECT,       &ProjectView::onOpenProject,             this);
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,      &ProjectView::onCloseProject,            this);

    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING,   &ProjectView::onStartEditing,    this);
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

void ProjectView::onOpenProject( model::EventOpenProject &event )
{
    mProject = event.getValue();
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

void ProjectView::onCloseProject( model::EventCloseProject &event )
{
    GetSizer()->Hide(&mCtrl);
    GetSizer()->Layout();
    gui::Window::get().Unbind(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, &ProjectView::onAutoOpenFolder, this);
    mCtrl.UnselectAll(); // To avoid crashes when directly loading a new project.
    mProject = 0;
    mOpenFolders.clear(); // Release shared_ptrs
    event.Skip();
}

void ProjectView::onAutoOpenFolder( EventAutoFolderOpen& event )
{
    if (UtilList<model::FolderPtr>(mOpenFolders).hasElement(event.getValue()))
    {
        mCtrl.Expand(wxDataViewItem(event.getValue()->id()));
    }
}

//////////////////////////////////////////////////////////////////////////
// SELECTION
//////////////////////////////////////////////////////////////////////////

void ProjectView::select( model::NodePtrs nodes)
{
    ASSERT(wxThread::IsMain());
    mCtrl.UnselectAll();
    for ( model::NodePtr node : nodes )
    {
        VAR_DEBUG(node->id());
        mCtrl.Select( wxDataViewItem( node->id() ) );
    }
}

void ProjectView::selectAll()
{
    ASSERT(wxThread::IsMain());
    mCtrl.SelectAll();
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

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxPoint ProjectView::find( model::NodePtr node )
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

//////////////////////////////////////////////////////////////////////////
// POPUP MENU
//////////////////////////////////////////////////////////////////////////

void ProjectView::onCut()
{
    ASSERT_MORE_THAN_ZERO(getSelection().size());
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new DataObject(getSelection()));
        wxTheClipboard->Close();
        model::ProjectModification::submit(new command::ProjectViewDeleteAsset(getSelection()));
    }
}

void ProjectView::onCopy()
{
    ASSERT_MORE_THAN_ZERO(getSelection().size());
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new DataObject(getSelection()));
        wxTheClipboard->Close();
    }
}

void ProjectView::onPaste()
{
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported( DataObject::sFormat ))
        {
            DataObject data;
            wxTheClipboard->GetData( data );
            if (data.getAssets().size() > 0)
            {
                for ( model::NodePtr node : data.getAssets() )
                {
                    if (findConflictingName( this, getSelectedContainer(), node->getName(), NODETYPE_ANY ))
                    {
                        return;
                    }
                }
                model::ProjectModification::submit(new command::ProjectViewAddAsset(getSelectedContainer(),data.getAssets()));
            }
        }
        wxTheClipboard->Close();
    }
}

void ProjectView::onDelete()
{
    model::ProjectModification::submit(new command::ProjectViewDeleteAsset(getSelection()));
}

void ProjectView::onDeleteUnused()
{
    model::AutoFolderPtr folder = boost::dynamic_pointer_cast<model::AutoFolder>(getSelectedContainer());
    ASSERT(folder);

    command::ProjectViewDeleteUnusedFiles(folder).recycleFiles();
}

void ProjectView::onNewFolder()
{
    wxString s = gui::Dialog::get().getText(_("Create new folder"), _("Enter folder name"), _("New Folder default value") );
    if ((s.CompareTo(_T("")) != 0) &&
        (!findConflictingName(this, getSelectedContainer(), s, NODETYPE_FOLDER)))
    {
        model::ProjectModification::submit(new command::ProjectViewCreateFolder(getSelectedContainer(), s));
    }
}

void ProjectView::onNewAutoFolder()
{
    wxString s = gui::Dialog::get().getDir( _("Add folder from disk"),wxStandardPaths::Get().GetDocumentsDir() );
    if ((s.CompareTo(_T("")) != 0) &&
        (!findConflictingName(this, getSelectedContainer(), s, NODETYPE_FOLDER)))
    {
        wxFileName path(s,"");
        path.Normalize();
        model::ProjectModification::submit(new command::ProjectViewCreateAutoFolder(getSelectedContainer(), path));
    }
}

void ProjectView::onNewSequence()
{
    wxString s = gui::Dialog::get().getText(_("Create new sequence"), _("Enter sequence name"), _("New sequence default value") );
    if ((s.CompareTo(_T("")) != 0) &&
        (!findConflictingName(this, getSelectedContainer(), s, NODETYPE_SEQUENCE)))
    {
        model::ProjectModification::submit(new command::ProjectViewCreateSequence(getSelectedContainer(), s));
    }
}

void ProjectView::onNewFile()
{
    wxString filetypes = _("Movie clips (*.avi;*.mov;*.mp4)|*.avi;*.mov;*.mp4|Images (*.gif;*.jpg)|*.gif;*.jpg|Sound files (*.wav;*.mp3)|*.wav;*.mp3|All files (%s)|%s");
    wxStrings files = gui::Dialog::get().getFiles( _("Select file(s) to add"), filetypes );
    std::vector<wxFileName> list;
    for ( wxString path : files )
    {
        if (findConflictingName(this, getSelectedContainer(),path, NODETYPE_FILE))
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
        model::ProjectModification::submit(new command::ProjectViewCreateFile(getSelectedContainer(), list));
    }
}

void ProjectView::onCreateSequence()
{
    command::ProjectViewCreateSequence* cmd = new command::ProjectViewCreateSequence(getSelectedContainer());
    if (!findConflictingName(this, cmd->getParent(), cmd->getName(), NODETYPE_SEQUENCE))
    {
        model::ProjectModification::submit(new command::ProjectViewCreateSequence(getSelectedContainer()));
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
            gui::Window::get().getTimeLines().Open(boost::dynamic_pointer_cast<model::Sequence>(node));
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void ProjectView::onContextMenu( wxDataViewEvent &event )
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
    bool enableDelete = true;
    bool enablePaste = (nSelected == 1);
    bool enableCreateSequence = (nSelected == 1);
    bool enableOpen = (nSelected == 1);
    bool enableDeleteUnused = (nSelected == 1);

    for ( wxDataViewItem item : sel )
    {
        model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(item.GetID()));

        bool isRoot = (!node->hasParent());
        bool isFolder = (boost::dynamic_pointer_cast<model::Folder>(node));
        bool isAutoFolder = (boost::dynamic_pointer_cast<model::AutoFolder>(node));
        bool isSequence = (boost::dynamic_pointer_cast<model::Sequence>(node));

        if (isRoot)
        {
            enableDelete = false;
            showNew = true;
            enableCreateSequence = false;
        }
        else if (isAutoFolder)
        {
            // Must be handled before 'folder' since an autofolder is also a folder
            enablePaste = false;
            enableNew = false;
            showCreateSequence = true;
            showDeleteUnused = true;
        }
        else if (isFolder)
        {
            showNew = true;
            showCreateSequence = true;
        }
        else if (isSequence)
        {
            showOpenSequence = true;
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
    wxMenu* createMenu = new wxMenu();;
    createMenu->Append( ID_NEW_FOLDER,     _("&Folder"), _("Add a new folder in the project") );
    createMenu->Append( ID_NEW_SEQUENCE,   _("&Sequence"), _("Create a new (empty) sequence") );

    wxMenu* addMenu = new wxMenu();
    addMenu->Append( ID_NEW_AUTOFOLDER, _("&Folder from disk"), _("Add disk folder and its contents to the project and then monitor for changes.") );
    addMenu->Append( ID_NEW_FILE,       _("Fi&le(s) from disk"), _("Select a file on disk to be added to the project.") );

    menu.Append( wxID_CUT,   _("Cu&t\tCTRL-x") );
    menu.Enable( wxID_CUT, enableDelete );
    menu.Append( wxID_COPY,  _("Cop&y\tCTRL-c") );
    menu.Enable( wxID_COPY, (nSelected > 0) );
    menu.Append( wxID_PASTE, _("&Paste\tCTRL-v") );
    menu.Enable( wxID_PASTE, enablePaste );
    menu.AppendSeparator();
    menu.Append( wxID_DELETE,_("&Delete\tDEL") );
    menu.Enable( wxID_DELETE, enableDelete );
    if (showDeleteUnused )
    {
        menu.Append( ID_DELETE_UNUSED, _("Delete unused files on disk") );
        menu.Enable( ID_DELETE_UNUSED, enableDeleteUnused );
    }
    menu.AppendSeparator();

    if (showCreateSequence)
    {
        menu.Append(ID_CREATE_SEQUENCE, _("&Make sequence"), ("Create a new sequence containing all the clips in the folder") );
        menu.Enable(ID_CREATE_SEQUENCE, enableCreateSequence);
    }
    if (showOpenSequence)
    {
        menu.Append( wxID_OPEN,_("&Open sequence\to") );
        menu.Enable( wxID_OPEN, enableOpen );
    }

    wxMenuItem* pAddMenu = 0;
    wxMenuItem* pCreateMenu = 0;

    if (showNew && enableNew)
    {
        menu.AppendSeparator();
        pAddMenu = menu.AppendSubMenu(addMenu,_("&Add"));
        menu.AppendSeparator();
        pCreateMenu = menu.AppendSubMenu(createMenu,_("&New"));
    }

    int result = GetPopupMenuSelectionFromUser(menu);
    switch (result)
    {
    case wxID_NONE:                                     break;
    case wxID_CUT:              onCut();                break;
    case wxID_COPY:             onCopy();               break;
    case wxID_PASTE:            onPaste();              break;
    case wxID_DELETE:           onDelete();             break;
    case ID_DELETE_UNUSED:      onDeleteUnused();       break;
    case ID_NEW_FOLDER:         onNewFolder();          break;
    case ID_NEW_AUTOFOLDER:     onNewAutoFolder();      break;
    case ID_NEW_SEQUENCE:       onNewSequence();        break;
    case ID_NEW_FILE:           onNewFile();            break;
    case ID_CREATE_SEQUENCE:    onCreateSequence();     break;
    case wxID_OPEN:             onOpen();               break;
    }
}

void ProjectView::onMotion(wxMouseEvent& event)
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
                mCtrl.HitTest(mDragStart, item, col );
                if (item.GetID())
                {
                    DataObject data(getSelection(), boost::bind(&ProjectView::onDragEnd,this));
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
}

void ProjectView::onDragEnd()
{
}

void ProjectView::onDropPossible( wxDataViewEvent &event )
{
    // Can only drop relevant type of info
    if (event.GetDataFormat().GetId() != DataObject::sFormat)
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
    for ( model::NodePtr node : mDropSource.getData().getAssets())
    {
        if (p == node || mModel->isDescendantOf(p, node))
        {
            event.Veto();
            return;
        }
    }
}

void ProjectView::onDrop( wxDataViewEvent &event )
{
    LOG_INFO;

    if (event.GetDataFormat().GetId() != DataObject::sFormat)
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

    DataObject o;
    o.SetData( event.GetDataSize(), event.GetDataBuffer() );

    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(p);
    ASSERT(folder);

    bool conflictingChildExists = false;
    for ( model::NodePtr node : o.getAssets())
    {
        if (findConflictingName(this, folder, node->getName(), NODETYPE_ANY))
        {
            event.Veto();
            return;
        }
    }
    if (o.getAssets().size() > 0)
    {
        model::ProjectModification::submit(new command::ProjectViewMoveAsset(o.getAssets(),p));
    }
}

void ProjectView::onActivated( wxDataViewEvent &event )
{
    model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
    model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(p);
    if (!sequence)
    {
        event.Veto();
        return;
    }
    Window::get().getTimeLines().Open(sequence);
}

void ProjectView::onExpanded( wxDataViewEvent &event )
{
    model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(p);
    ASSERT(folder);
    if(!UtilList<model::FolderPtr>(mOpenFolders).hasElement(folder))
    {
        mOpenFolders.push_back(folder);
    }
}

void ProjectView::onCollapsed( wxDataViewEvent &event )
{
    model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(p);
    ASSERT(folder);
    ASSERT(UtilList<model::FolderPtr>(mOpenFolders).hasElement(folder));
    UtilList<model::FolderPtr>(mOpenFolders).removeElements(boost::assign::list_of(folder));
}

void ProjectView::onStartEditing( wxDataViewEvent &event )
{
    model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
    if (!mModel->canBeRenamed(node))
    {
        event.Veto();
    }
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

bool findConflictingName(wxWindow* window, model::FolderPtr parent, wxString name, NodeType type )
{
    for ( model::NodePtr child : parent->getChildren() )
    {
        if (child->getName().IsSameAs(name))
        {
            bool isSameType = true;
            wxString prefix = _("Name");
            switch (type)
            {
            case NODETYPE_FILE:
                isSameType = (child->isA<model::File>());
                prefix = _("A file with name");
                break;
            case NODETYPE_FOLDER:
                isSameType = (child->isA<model::Folder>());
                prefix = _("A folder with name");
                break;
            case NODETYPE_SEQUENCE:
                isSameType = (child->isA<model::Sequence>());
                prefix = _("A sequence with name");
                break;
            default:
                break;
            }
            if (isSameType)
            {
                gui::Dialog::get().getConfirmation(_("Duplicate exists"), prefix + _(" '") + name + _("' already exists"));
                return true;
            }
        }
    }
    return false;
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
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void ProjectView::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void ProjectView::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} // namespace