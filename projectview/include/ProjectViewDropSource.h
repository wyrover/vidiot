#ifndef PROJECT_VIEW_DROP_SOURCE_H
#define PROJECT_VIEW_DROP_SOURCE_H

#include <wx/dnd.h>
#include <wx/dataview.h>
#include <wx/frame.h>
#include <list>
#include <boost/shared_ptr.hpp>

namespace model {
class AProjectViewNode;
typedef AProjectViewNode* ProjectViewId;
typedef boost::shared_ptr<AProjectViewNode> ProjectViewPtr;
typedef std::list<ProjectViewPtr> ProjectViewPtrs;
}

namespace gui {

class ProjectViewModel;
class GuiDataObject;

class ProjectViewDropSource 
    :   public wxDropSource
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ProjectViewDropSource(wxDataViewCtrl& ctrl, ProjectViewModel& model);
    ~ProjectViewDropSource();

    static ProjectViewDropSource& current();

    //////////////////////////////////////////////////////////////////////////
    // FROM WXDROPSOURCE
    //////////////////////////////////////////////////////////////////////////

    virtual bool GiveFeedback(wxDragResult effect);

    //////////////////////////////////////////////////////////////////////////
    // DRAGGING
    //////////////////////////////////////////////////////////////////////////

    void startDrag(GuiDataObject& data);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    GuiDataObject& getData();       ///< \return the current associated data object
    void setFeedback(bool enabled); ///< Enable/disable showing feedback. Used to avoid flicker

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxDataViewCtrl& mCtrl;
    ProjectViewModel& mModel;
    wxFrame *mHint;
    bool mFeedback;             ///< true if feedback must be shown while drawing

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void drawAsset(wxDC* dc, wxRect rect, model::ProjectViewPtr asset);

};

} // namespace

#endif // PROJECT_VIEW_DROP_SOURCE_H