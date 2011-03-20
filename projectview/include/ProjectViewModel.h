#ifndef GUI_PROJECT_VIEW_MODEL_H
#define GUI_PROJECT_VIEW_MODEL_H

#include <wx/event.h>
#include <wx/dataview.h>
#include "AProjectViewNode.h"
#include "Project.h"
#include "ModelPtr.h"

namespace gui {

/**
* This model links the GUI objects to the actual project items and back.
*/
class ProjectViewModel
    :   public wxDataViewModel
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ProjectViewModel(wxDataViewCtrl& view);
    ~ProjectViewModel();

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
    const wxIcon& getIcon(model::ProjectViewPtr node) const;

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnOpenProject( model::EventOpenProject &event );
    void OnCloseProject( model::EventCloseProject &event );
    void AddRecursive( model::ProjectViewPtr node );
    void OnProjectAssetAdded( model::EventAddAsset &event );
    void OnProjectAssetRemoved( model::EventRemoveAsset &event );
    void OnProjectAssetRenamed( model::EventRenameAsset &event );

private:

    model::Project* mProject;

    wxDataViewCtrl& mView;

    wxIcon mIconAutoFolder;
    wxIcon mIconAutoFolderOpen;
    wxIcon mIconFolder;
    wxIcon mIconFolderOpen;
	wxIcon mIconVideo;
};

/**
* This event is used to signal opening of certain folders after loading
* a saved project. Is done via an event, due to the trigger moment: it should
* be done after a certain 'to be opened' folder is known to the control. Thus,
* it should be done after GetChildren() has returned the node. Hence, the decoupling
* via the event queue instead of a direct call in the GetChildren() method.
*/
DECLARE_EVENT(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, EventAutoFolderOpen, model::FolderPtr);

} // namespace

#endif // GUI_PROJECT_VIEW_MODEL_H