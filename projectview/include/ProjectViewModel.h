#ifndef GUI_PROJECT_VIEW_MODEL_H
#define GUI_PROJECT_VIEW_MODEL_H

#include "UtilEvent.h"

namespace model {
class Project;
class EventOpenProject;
class EventCloseProject;
class EventAddNode;
class EventRemoveNode;
class EventRenameNode;
}

namespace gui {

/// This model links the GUI objects to the actual project items and back.
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

    bool IsContainer (const wxDataViewItem &wxItem) const override;
    wxDataViewItem GetParent( const wxDataViewItem &wxItem ) const override;
    unsigned int GetChildren( const wxDataViewItem &wxItem, wxDataViewItemArray &wxItemArray ) const override;
    unsigned int GetColumnCount() const override;
    wxString GetColumnType(unsigned int col) const override;
    void GetValue( wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col ) const override;
    bool SetValue( const wxVariant &variant, const wxDataViewItem &wxItem, unsigned int col ) override;
    bool HasDefaultCompare() const override;
    int Compare(const wxDataViewItem& item1, const wxDataViewItem& item2, unsigned int column, bool ascending) const override;

    //////////////////////////////////////////////////////////////////////////
    //
    //////////////////////////////////////////////////////////////////////////

    bool isRoot( model::NodePtr node ) const;
    bool isAutomaticallyGenerated( model::NodePtr node ) const;
    bool isFolder( model::NodePtr node ) const;
    bool isAutoFolder( model::NodePtr node) const;
    bool isSequence( model::NodePtr node) const;
    bool isDescendantOf( model::NodePtr node, model::NodePtr ascendant ) const;
    bool canBeRenamed( model::NodePtr node ) const;
    wxIcon getIcon( model::NodePtr node ) const;

    //////////////////////////////////////////////////////////////////////////
    // PROJECT EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onOpenProject( model::EventOpenProject &event );
    void onCloseProject( model::EventCloseProject &event );
    void onProjectAssetAdded( model::EventAddNode &event );
    void onProjectAssetRemoved( model::EventRemoveNode &event );
    void onProjectAssetRenamed( model::EventRenameNode &event );

private:

    model::Project* mProject;

    wxDataViewCtrl& mView;

    wxIcon mIconAutoFolder;
    wxIcon mIconAutoFolderOpen;
    wxIcon mIconFolder;
    wxIcon mIconFolderOpen;
    wxIcon mIconVideo;
};

/// This event is used to signal opening of certain folders after loading
/// a saved project. Is done via an event, due to the trigger moment: it should
/// be done after a certain 'to be opened' folder is known to the control. Thus,
/// it should be done after GetChildren() has returned the node. Hence, the decoupling
/// via the event queue instead of a direct call in the GetChildren() method.
DECLARE_EVENT(GUI_EVENT_PROJECT_VIEW_AUTO_OPEN_FOLDER, EventAutoFolderOpen, model::FolderPtr);

} // namespace

#endif // GUI_PROJECT_VIEW_MODEL_H