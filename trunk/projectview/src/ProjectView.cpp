#include "ProjectView.h"

#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/set.hpp>
#include <wx/stdpaths.h>
#include <wx/dnd.h>
#include <wx/dirdlg.h>
#include <wx/tokenzr.h>
#include <wx/clipbrd.h>
#include <wx/textdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include "Folder.h"
#include "DataObject.h"
#include "Window.h"
#include "TimeLinesView.h"
#include "Project.h"
#include "AProjectViewNode.h"
#include "Layout.h"
#include "ProjectViewAddAsset.h"
#include "ProjectViewDropSource.h"
#include "ProjectViewDeleteAsset.h"
#include "ProjectViewMoveAsset.h"
#include "ProjectViewRenameAsset.h"
#include "ProjectViewCreateSequence.h"
#include "ProjectViewCreateFolder.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateFile.h"
#include "UtilLogWxwidgets.h"
#include "UtilLog.h"
#include "Sequence.h"

namespace gui {

enum {
    meID_NEW_FOLDER= wxID_HIGHEST+1,
    meID_NEW_AUTOFOLDER,
    meID_NEW_SEQUENCE,
    meID_NEW_FILE,
    meID_CREATE_SEQUENCE,
    meID_UPDATE_AUTOFOLDER,
};

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
    wxDataViewColumn* dateColumn = mCtrl.AppendTextColumn("Modified",   2, wxDATAVIEW_CELL_INERT,       -1, wxALIGN_RIGHT,  wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE | wxDATAVIEW_COL_REORDERABLE );

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
    Bind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onUpdateAutoFolder,    this, meID_UPDATE_AUTOFOLDER);

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
    Unbind(wxEVT_COMMAND_MENU_SELECTED,   &ProjectView::onUpdateAutoFolder,  this, meID_CREATE_SEQUENCE);

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

ProjectView* ProjectView::current()
{
    return sCurrent;
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

    bool showUpdateAutoFolder = false;
    bool enableUpdateAutoFolder = true;

    bool enableNew = (nSelected == 1);
    bool enableDelete = true;
    bool enablePaste = (nSelected == 1);
    bool enableCreateSequence = (nSelected == 1);

    BOOST_FOREACH( wxDataViewItem item, sel )
    {
        model::ProjectViewPtr node = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(item.GetID()));

        bool isRoot = (!node->getParent());
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
            showUpdateAutoFolder = true;
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

    if (showUpdateAutoFolder)
    {
        menu.AppendSeparator();
        menu.Append(meID_UPDATE_AUTOFOLDER, _("&Update autofolder"));
        menu.Enable(meID_UPDATE_AUTOFOLDER, enableUpdateAutoFolder);
    }

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

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void ProjectView::onCut(wxCommandEvent& WXUNUSED(event))
{
    ASSERT(getSelection().size() > 0);
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new DataObject(getSelection()));
        wxTheClipboard->Close();
        mProject->Submit(new command::ProjectViewDeleteAsset(getSelection()));
    }
}

void ProjectView::onCopy(wxCommandEvent& WXUNUSED(event))
{
    ASSERT(getSelection().size() > 0);
    if (wxTheClipboard->Open())
    {
        wxTheClipboard->SetData(new DataObject(getSelection()));
        wxTheClipboard->Close();
    }
}

void ProjectView::onPaste(wxCommandEvent& WXUNUSED(event))
{
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported( DataObject::sFormat ))
        {
            DataObject data;
            wxTheClipboard->GetData( data );
            if (data.getAssets().size() > 0)
            {
                BOOST_FOREACH( model::ProjectViewPtr node, data.getAssets() )
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

void ProjectView::onDelete(wxCommandEvent& WXUNUSED(event))
{
    mProject->Submit(new command::ProjectViewDeleteAsset(getSelection()));
}

void ProjectView::onNewFolder(wxCommandEvent& WXUNUSED(event))
{
    wxString s = wxGetTextFromUser (_("Enter folder name"),_("Input text"), "New Folder default value", 0, wxDefaultCoord, wxDefaultCoord, true);
    if ((s.CompareTo(_T("")) != 0) &&
        (!FindConflictingName(getSelectedContainer(), s)))
    {
        mProject->Submit(new command::ProjectViewCreateFolder(getSelectedContainer(), s));
    }
}

void ProjectView::onNewAutoFolder(wxCommandEvent& WXUNUSED(event))
{
    wxString s = wxDirSelector(_("Select folder to automatically index"),wxStandardPaths::Get().GetDocumentsDir(), wxDD_DEFAULT_STYLE);
    if ((s.CompareTo(_T("")) != 0) &&
        (!FindConflictingName(getSelectedContainer(), s)))
    {
        mProject->Submit(new command::ProjectViewCreateAutoFolder(getSelectedContainer(), boost::filesystem::path(s.ToStdString())));
    }
}

void ProjectView::onNewSequence(wxCommandEvent& WXUNUSED(event))
{
    wxString s = wxGetTextFromUser(_("Enter sequence name"),_("Input text"), "New sequence default value", 0, wxDefaultCoord, wxDefaultCoord, true);
    if ((s.CompareTo(_T("")) != 0) &&
        (!FindConflictingName(getSelectedContainer(), s)))
    {
        mProject->Submit(new command::ProjectViewCreateSequence(getSelectedContainer(), s));
    }
}

void ProjectView::onNewFile(wxCommandEvent& WXUNUSED(event))
{
    wxString wildcards =
        wxString::Format
        (
          _("Movie clips (*.avi)|*.avi|Images (*.gif;*.jpg)|*.gif;*.jpg|Sound files (*.wav;*.mp3)|*.wav;*.mp3|All files (%s)|%s"),
        wxFileSelectorDefaultWildcardStr,
        wxFileSelectorDefaultWildcardStr
        );
    wxFileDialog dialog(this, _T("Select file(s) to add"), wxEmptyString, wxEmptyString, wildcards, wxFD_OPEN|wxFD_MULTIPLE);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString paths;
        dialog.GetPaths(paths);


        std::vector<boost::filesystem::path> list;
        BOOST_FOREACH( wxString path, paths)
        {
            boost::filesystem::path p(path.ToStdString());
            if (FindConflictingName(getSelectedContainer(),p.filename().string()))
            {
                return;
            }
            list.push_back(p);
        }
        mProject->Submit(new command::ProjectViewCreateFile(getSelectedContainer(), list));
    }
}

void ProjectView::onCreateSequence(wxCommandEvent& WXUNUSED(event))
{
    command::ProjectViewCreateSequence* cmd = new command::ProjectViewCreateSequence(getSelectedContainer());
    mProject->Submit(cmd);
}

void ProjectView::onUpdateAutoFolder(wxCommandEvent& WXUNUSED(event))
{
    BOOST_FOREACH(model::ProjectViewPtr node, getSelection())
    {
        if (node->isA<model::AutoFolder>())
        {
            model::AutoFolderPtr autofolder = boost::dynamic_pointer_cast<model::AutoFolder>(node);
            autofolder->update();
        }
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
    model::ProjectViewPtr p = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(event.GetItem().GetID()));
    if (mModel->isAutomaticallyGenerated(p) || !mModel->isFolder(p) || mModel->isAutoFolder(p))
    {
        event.Veto();
        return;
    }

    // Cannot drop a node into itselves, or one of its children
    BOOST_FOREACH( model::ProjectViewPtr node, mDropSource.getData().getAssets())
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
    model::ProjectViewPtr p = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(event.GetItem().GetID()));
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
    BOOST_FOREACH( model::ProjectViewPtr node, o.getAssets())
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
	model::ProjectViewPtr p = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(event.GetItem().GetID()));
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
    model::ProjectViewPtr p = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(event.GetItem().GetID()));
    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(p);
    ASSERT(folder);
    mOpenFolders.insert(folder);
}

void ProjectView::onCollapsed( wxDataViewEvent &event )
{
    model::ProjectViewPtr p = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(event.GetItem().GetID()));
    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(p);
    ASSERT(folder);
    mOpenFolders.erase(folder);
}

void ProjectView::onStartEditing( wxDataViewEvent &event )
{
    model::ProjectViewPtr node = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(event.GetItem().GetID()));
    if (!mModel->canBeRenamed(node))
    {
        event.Veto();
    }
    event.Skip();
}


//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

model::FolderPtr ProjectView::getSelectedContainer() const
{
    wxDataViewItemArray selection;
    mCtrl.GetSelections(selection);
    ASSERT(selection.size() == 1);
    model::ProjectViewPtr projectNode = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(selection[0].GetID()));
    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(projectNode);
    ASSERT(folder);
    return folder;
}

model::ProjectViewPtrs ProjectView::getSelection() const
{
    model::ProjectViewPtrs l;
    wxDataViewItemArray selection;
    mCtrl.GetSelections(selection);

    BOOST_FOREACH(wxDataViewItem wxItem, selection)
    {
        model::ProjectViewPtr node = model::AProjectViewNode::Ptr(static_cast<model::ProjectViewId>(wxItem.GetID()));
        ASSERT(node != 0);
        l.push_back(node);
    }
    return l;
}

bool ProjectView::FindConflictingName(model::FolderPtr parent, wxString name )
{
    BOOST_FOREACH( model::ProjectViewPtr child, parent->getChildren() )
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

void ProjectView::OpenRecursive(model::FolderPtr folder)
{
    if (mOpenFolders.count(folder) == 1)
    {
        mCtrl.Expand(wxDataViewItem(folder->id()));
    }
    BOOST_FOREACH( model::ProjectViewPtr child, folder->getChildren())
    {
        model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(child);
        if (folder)
        {
            OpenRecursive(folder);
        }
    }
}

template<class Archive>
void ProjectView::serialize(Archive & ar, const unsigned int version)
{
    ar & mOpenFolders;
}
template void ProjectView::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void ProjectView::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace
