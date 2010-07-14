#ifndef GUI_PROJECT_VIEW_MODEL_H
#define GUI_PROJECT_VIEW_MODEL_H

#include <map>
#include "wx/event.h"
#include "wx/dataview.h"
#include "AProjectViewNode.h"

namespace model { 
    class Project;
    class Folder;
    typedef boost::shared_ptr<Folder> FolderPtr;
}
class ProjectEventAddAsset;
class ProjectEventOpenProject;
class ProjectEventCloseProject;
class ProjectEventAddAsset;
class ProjectEventDeleteAsset;
class ProjectEventRenameAsset;

/**
* This model links the GUI objects to the actual project items and back.
*/
class GuiProjectViewModel : public wxDataViewModel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    GuiProjectViewModel(wxDataViewCtrl& view);
    ~GuiProjectViewModel();

    //////////////////////////////////////////////////////////////////////////
    // OVERRIDES FROM WXDATAVIEWMODEL
    //////////////////////////////////////////////////////////////////////////

    bool IsContainer (const wxDataViewItem &wxItem) const;
    wxDataViewItem GetParent( const wxDataViewItem &wxItem ) const;
    unsigned int GetChildren( const wxDataViewItem &wxItem, wxDataViewItemArray &wxItemArray ) const;
    unsigned int GetColumnCount() const;
    wxString GetColumnType(unsigned int col) const;
    void GetValue( wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col ) const;
    bool SetValue( const wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col );
    bool HasDefaultCompare() const;
    int Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const;

    //////////////////////////////////////////////////////////////////////////
    // 
    //////////////////////////////////////////////////////////////////////////

    bool isRoot(model::ProjectViewPtr node) const;
    bool isAutomaticallyGenerated(model::ProjectViewPtr node) const;
    bool isFolder(model::ProjectViewPtr node) const;
    bool isAutoFolder(model::ProjectViewPtr node) const;
    bool isSequence(model::ProjectViewPtr node) const;
    bool isDescendantOf(model::ProjectViewPtr node, model::ProjectViewPtr ascendant) const;
    bool canBeRenamed(model::ProjectViewPtr node) const;

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnOpenProject( ProjectEventOpenProject &event );
    void OnCloseProject( ProjectEventCloseProject &event );
    void AddRecursive( model::ProjectViewPtr node );
    void OnProjectAssetAdded( ProjectEventAddAsset &event );
    void OnProjectAssetDeleted( ProjectEventDeleteAsset &event );
    void OnProjectAssetRenamed( ProjectEventRenameAsset &event );


    /**
    * This event is used to signal opening of certain folders after loading
    * a saved project. Is done via an event, due to the trigger moment: it should
    * be done after a certain 'to be opened' folder is known to the control. Thus,
    * it should be done after GetChildren() has returned the node. Hence, the decoupling
    * via the event queue instead of a direct call in the GetChildren() method.
    */
    // todo use utilevent macro
    class FolderEvent : public wxEvent
    {
    public:
        FolderEvent(model::FolderPtr folder);
        FolderEvent(const FolderEvent& other);
        virtual wxEvent* Clone() const;
        model::FolderPtr getFolder() const;
    private:
        model::FolderPtr mFolder;
    };

private:

    model::Project* mProject;

    wxDataViewCtrl& mView;

    wxIcon mIconAutoFolder;
    wxIcon mIconAutoFolderOpen;
    wxIcon mIconFolder;
    wxIcon mIconFolderOpen;
	wxIcon mIconVideo;
};

wxDECLARE_EVENT(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, GuiProjectViewModel::FolderEvent);

#endif // GUI_PROJECT_VIEW_MODEL_H