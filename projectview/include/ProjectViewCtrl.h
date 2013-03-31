#ifndef GUI_PROJECT_VIEW_CTRL_H
#define GUI_PROJECT_VIEW_CTRL_H

namespace gui {

class ProjectViewCtrl
:   public wxDataViewCtrl
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ProjectViewCtrl(wxWindow* parent);
    virtual ~ProjectViewCtrl();

    //////////////////////////////////////////////////////////////////////////
    // MODIFY
    //////////////////////////////////////////////////////////////////////////

    void selectColumnHeader(int pos);
};

} // namespace

#endif // GUI_PROJECT_VIEW_CTRL_H