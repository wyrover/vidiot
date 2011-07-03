#include "ProjectView.h"

#include <wx/clipbrd.h>
#include <wx/dirdlg.h>
#include <wx/dnd.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wx/tokenzr.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/foreach.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "Node.h"
#include "AutoFolder.h"
#include "DataObject.h"
#include "Folder.h"
#include "ids.h"
#include "Layout.h"
#include "Project.h"
#include "ProjectEvent.h"
#include "ProjectViewAddAsset.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateFile.h"
#include "ProjectViewCreateFolder.h"
#include "ProjectViewCreateSequence.h"
#include "ProjectViewDeleteAsset.h"
#include "ProjectViewDropSource.h"
#include "ProjectViewMoveAsset.h"
#include "ProjectViewRenameAsset.h"
#include "Sequence.h"
#include "TimeLinesView.h"
#include "UtilDialog.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "Window.h"

namespace gui {

static ProjectView* sCurrent = 0;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectView::ProjectView(wxWindow* parent)
    :   wxPanel(parent)
    ,   mProject(0)
    ,   mCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_MULTIPLE | wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_VERT_RULES)
    ,   mModel(new ProjectViewModel(mCtrl))
    ,   mDropSource(mCtrl, *mModel)
    ,   mOpenFolders()
    ,   mDragCount(0)
    ,   mDragStart(0,0)
{
    LOG_INFO;

    sCurrent = this;

    mCtrl.EnableDropTarget( DataObject::sFormat );
    wxDataViewColumn* nameColumn = mCtrl.AppendIconTextColumn("Name",       0, wxDATAVIEW_CELL_EDITABLE,    200, wxALIGN_LEFT,   wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );
    wxDataViewColumn* dateColumn = mCtrl.AppendTextColumn("Modified",   1, wxDATAVIEW_CELL_INERT,       -1, wxALIGN_RIGHT,  wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );

    mCtrl.AssociateModel( mModel );
    mModel->DecRef();

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add( &mCtrl, 1, wxGROW );
    sizer->Hide(&mCtrl);
    sizer->Layout();
    SetSizerAndFit(sizer);

    gui::Window::get().Bind(model::EVENT_OPEN_PROJECT,     &ProjectView::onOpenProject,             this);
    gui::Window::get().Bind(model::EVENT_CLOSE_PROJECT,    &ProjectView::onCloseProject,            this);

    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onCut,                 this, wxID_CUT);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onCopy,                this, wxID_COPY);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onPaste,               this, wxID_PASTE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onDelete,              this, wxID_DELETE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onNewFolder,           this, meID_NEW_FOLDER);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onNewAutoFolder,       this, meID_NEW_AUTOFOLDER);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onNewSequence,         this, meID_NEW_SEQUENCE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onNewFile,             this, meID_NEW_FILE);
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onCreateSequence,      this, meID_CREATE_SEQUENCE);

    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING,     &ProjectView::onStartEditing,    this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU,      &ProjectView::onContextMenu,     this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_DROP_POSSIBLE,     &ProjectView::onDropPossible,    this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_DROP,              &ProjectView::onDrop,            this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED,         &ProjectView::onActivated,       this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_EXPANDED,          &ProjectView::onExpanded,        this);
    Bind(wxEVT_COMMAND_DATAVIEW_ITEM_COLLAPSED,         &ProjectView::onCollapsed,       this);

    mCtrl.GetMainWindow()->Bind(wxEVT_MOTION,           &ProjectView::onMotion,          this);
}

ProjectView::~ProjectView()
{
    gui::Window::get().Unbind(model::EVENT_OPEN_PROJECT,       &ProjectView::onOpenProject,             this);
    gui::Window::get().Unbind(model::EVENT_CLOSE_PROJECT,      &ProjectView::onCloseProject,            this);

    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onCut,               this, wxID_CUT);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onCopy,              this, wxID_COPY);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onPaste,             this, wxID_PASTE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onDelete,            this, wxID_DELETE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onNewFolder,         this, meID_NEW_FOLDER);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onNewAutoFolder,     this, meID_NEW_AUTOFOLDER);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onNewSequence,       this, meID_NEW_SEQUENCE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onNewFile,           this, meID_NEW_FILE);
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onCreateSequence,    this, meID_CREATE_SEQUENCE);

    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_START_EDITING,   &ProjectView::onStartEditing,    this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU,    &ProjectView::onContextMenu,     this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_DROP_POSSIBLE,   &ProjectView::onDropPossible,    this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_DROP,            &ProjectView::onDrop,            this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_ACTIVATED,       &ProjectView::onActivated,       this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_EXPANDED,        &ProjectView::onExpanded,        this);
    Unbind(wxEVT_COMMAND_DATAVIEW_ITEM_COLLAPSED,       &ProjectView::onCollapsed,       this);

    mCtrl.GetMainWindow()->Unbind(wxEVT_MOTION,         &ProjectView::onMotion,          this);

    sCurrent = 0;
}

ProjectView& ProjectView::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
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
    if (mOpenFolders.count(event.getValue()) == 1)
    {
        mCtrl.Expand(wxDataViewItem(event.getValue()->id()));
    }
}

//////////////////////////////////////////////////////////////////////////
// SELECTION
//////////////////////////////////////////////////////////////////////////

void ProjectView::select( model::NodePtrs nodes)
{
    mCtrl.UnselectAll();
    BOOST_FOREACH( model::NodePtr node, nodes )
    {
        VAR_DEBUG(node->id());
        mCtrl.Select( wxDataViewItem( node->id() ) );
    }
}

void ProjectView::selectAll()
{
    mCtrl.SelectAll();
}

model::FolderPtr ProjectView::getSelectedContainer() const
{
    wxDataViewItemArray selection;
    mCtrl.GetSelections(selection);
    ASSERT(selection.size() == 1);
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

    BOOST_FOREACH(wxDataViewItem wxItem, selection)
    {
        model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(wxItem.GetID()));
        ASSERT(node != 0);
        l.push_back(node);
    }
    return l;
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

    bool enableUpdateAutoFolder = true;

    bool enableNew = (nSelected == 1);
    bool enableDelete = true;
    bool enablePaste = (nSelected == 1);
    bool enableCreateSequence = (nSelected == 1);

    BOOST_FOREACH( wxDataViewItem item, sel )
    {
        model::NodePtr node = model::INode::Ptr(static_cast<model::NodeId>(item.GetID()));

        bool isRoot = (!node->hasParent());
        bool isFolder = (boost::dynamic_pointer_cast<model::Folder>(node));
        bool isAutoFolder = (boost::dynamic_pointer_cast<model::AutoFolder>(node));

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
        }
        else if (isFolder)
        {
            showNew = true;
            showCreateSequence = true;
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

    wxMenu createMenu;
    createMenu.Append( meID_NEW_FOLDER,     _("&Folder") );
    createMenu.Append( meID_NEW_SEQUENCE,   _("&Sequence") );

    wxMenu addMenu;
    addMenu.Append( meID_NEW_AUTOFOLDER, _("&Auto Folder") );
    addMenu.Append( meID_NEW_FILE,       _("Fi&le") );

    wxMenu menu;
    menu.Append( wxID_CUT,   _("Cu&t\tCTRL-x") );
    menu.Enable( wxID_CUT, enableDelete );
    menu.Append( wxID_COPY,  _("Cop&y\tCTRL-c") );
    menu.Enable( wxID_COPY, (nSelected > 0) );
    menu.Append( wxID_PASTE, _("&Paste\tCTRL-v") );
    menu.Enable( wxID_PASTE, enablePaste );
    menu.AppendSeparator();
    menu.Append( wxID_DELETE,_("&Delete\tDEL") );
    menu.Enable( wxID_DELETE, enableDelete );

    if (showCreateSequence)
    {
        menu.AppendSeparator();
        menu.Append(meID_CREATE_SEQUENCE, _("&Make sequence"));
        menu.Enable(meID_CREATE_SEQUENCE, enableCreateSequence);
    }

    wxMenuItem* pAddMenu = 0;
    wxMenuItem* pCreateMenu = 0;

    if (showNew && enableNew)
    {
        menu.AppendSeparator();
        pAddMenu = menu.AppendSubMenu(&addMenu,_("&Add"));
        menu.AppendSeparator();
        pCreateMenu = menu.AppendSubMenu(&createMenu,_("&New"));
    }

    PopupMenu(&menu);
    if (showNew && enableNew)
    {
        menu.Remove(pCreateMenu);   // To avoid deletion via menu structure AND via going out of scope.
        menu.Remove(pAddMenu);      // To avoid deletion via menu structure AND via going out of scope.
    }
}

void ProjectView::onCut(wxCommandEvent& event)
{
    ASSERT(getSelection().size() > 0);
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new DataObject(getSelection()));
        wxTheClipboard->Close();
        mProject->Submit(new command::ProjectViewDeleteAsset(getSelection()));
    }
}

void ProjectView::onCopy(wxCommandEvent& event)
{
    ASSERT(getSelection().size() > 0);
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new DataObject(getSelection()));
        wxTheClipboard->Close();
    }
}

void ProjectView::onPaste(wxCommandEvent& event)
{
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported( DataObject::sFormat ))
        {
            DataObject data;
            wxTheClipboard->GetData( data );
            if (data.getAssets().size() > 0)
            {
                BOOST_FOREACH( model::NodePtr node, data.getAssets() )
                {
                    if (FindConflictingName( getSelectedContainer(), node->getName() ))
                    {
                        return;
                    }
                }
                mProject->Submit(new command::ProjectViewAddAsset(getSelectedContainer(),data.getAssets()));
            }
        }
        wxTheClipboard->Close();
    }
}

void ProjectView::onDelete(wxCommandEvent& event)
{
    mProject->Submit(new command::ProjectViewDeleteAsset(getSelection()));
}

void ProjectView::onNewFolder(wxCommandEvent& event)
{
    wxString s = UtilDialog::getText(_("Create new folder"), _("Enter folder name"), _("New Folder default value"), this );
    if ((s.CompareTo(_T("")) != 0) &&
        (!FindConflictingName(getSelectedContainer(), s)))
    {
        mProject->Submit(new command::ProjectViewCreateFolder(getSelectedContainer(), s));
    }
}

void ProjectView::onNewAutoFolder(wxCommandEvent& event)
{
    wxString s = UtilDialog::getDir( _("Select folder to automatically index"),wxStandardPaths::Get().GetDocumentsDir(), this );
    if ((s.CompareTo(_T("")) != 0) &&
        (!FindConflictingName(getSelectedContainer(), s)))
    {
        mProject->Submit(new command::ProjectViewCreateAutoFolder(getSelectedContainer(), wxFileName(s)));
    }
}

void ProjectView::onNewSequence(wxCommandEvent& event)
{
    wxString s = UtilDialog::getText(_("Create new sequence"), _("Enter sequence name"), _("New sequence default value"), this );
    if ((s.CompareTo(_T("")) != 0) &&
        (!FindConflictingName(getSelectedContainer(), s)))
    {
        mProject->Submit(new command::ProjectViewCreateSequence(getSelectedContainer(), s));
    }
}

void ProjectView::onNewFile(wxCommandEvent& event)
{
    wxString filetypes = _("Movie clips (*.avi)|*.avi|Images (*.gif;*.jpg)|*.gif;*.jpg|Sound files (*.wav;*.mp3)|*.wav;*.mp3|All files (%s)|%s");
    std::list<wxString> files = UtilDialog::getFiles( _("Select file(s) to add"), filetypes, this );
    std::vector<wxFileName> list;
    BOOST_FOREACH( wxString path, files )
    {
        if (FindConflictingName(getSelectedContainer(),path))
        {
            return;
        }
        list.push_back(wxFileName(path));
    }
    if (list.size() > 0 )
    {
        mProject->Submit(new command::ProjectViewCreateFile(getSelectedContainer(), list));
    }
}

void ProjectView::onCreateSequence(wxCommandEvent& event)
{
    command::ProjectViewCreateSequence* cmd = new command::ProjectViewCreateSequence(getSelectedContainer());
    mProject->Submit(cmd);
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
    BOOST_FOREACH( model::NodePtr node, mDropSource.getData().getAssets())
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
    BOOST_FOREACH( model::NodePtr node, o.getAssets())
    {
        if (FindConflictingName(folder, node->getName()))
        {
            event.Veto();
            return;
        }
    }
    if (o.getAssets().size() > 0)
    {
        mProject->Submit(new command::ProjectViewMoveAsset(o.getAssets(),p));
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
    mOpenFolders.insert(folder);
}

void ProjectView::onCollapsed( wxDataViewEvent &event )
{
    model::NodePtr p = model::INode::Ptr(static_cast<model::NodeId>(event.GetItem().GetID()));
    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(p);
    ASSERT(folder);
    mOpenFolders.erase(folder);
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

bool ProjectView::FindConflictingName(model::FolderPtr parent, wxString name )
{
    BOOST_FOREACH( model::NodePtr child, parent->getChildren() )
    {
        if (child->getName().IsSameAs(name))
        {
            wxMessageDialog dialog(this, _("Name '" + name + "' already exists"), _("Duplicate exists"), wxCENTER | wxOK | wxICON_ERROR);
            dialog.ShowModal();
            return true;

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
    ar & mOpenFolders;
}
template void ProjectView::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void ProjectView::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace
