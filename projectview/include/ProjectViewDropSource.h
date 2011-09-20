#ifndef PROJECT_VIEW_DROP_SOURCE_H
#define PROJECT_VIEW_DROP_SOURCE_H

#include <wx/dnd.h>
#include <wx/dataview.h>
#include <wx/frame.h>
#include <list>
#include <boost/shared_ptr.hpp>

namespace model {
class INode;
typedef INode* NodeId;
typedef boost::shared_ptr<INode> NodePtr;
typedef std::list<NodePtr> ProjectViewPtrs;
}

namespace gui {

class ProjectViewModel;
class DataObject;

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

    virtual bool GiveFeedback(wxDragResult effect) override;

    //////////////////////////////////////////////////////////////////////////
    // DRAGGING
    //////////////////////////////////////////////////////////////////////////

    void startDrag(DataObject& data);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    DataObject& getData();       ///< \return the current associated data object
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

    void drawAsset(wxDC* dc, wxRect rect, model::NodePtr asset);

};

} // namespace

#endif // PROJECT_VIEW_DROP_SOURCE_H