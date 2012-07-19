#ifndef OPTIONS_H
#define OPTIONS_H

#include "UtilEnumSelector.h"
#include "Enums.h"

namespace gui {

class Options
    :   public wxPropertySheetDialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Options(wxWindow* parent);
    ~Options();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxCheckBox*             mLoadLast;

    wxRadioBox*             mFrameRate;
    wxSpinCtrl*             mDefaultVideoWidth;
    wxSpinCtrl*             mDefaultVideoHeight;
    EnumSelector<model::VideoScaling>* mDefaultVideoScaling;
    EnumSelector<model::VideoAlignment>* mDefaultVideoAlignment;

    wxSpinCtrlDouble*       mMarkerBeginAddition;
    wxSpinCtrlDouble*       mMarkerEndAddition;
    wxTextCtrl*             mStrip;

    EnumSelector<LogLevel>* mSelectLogLevel;
    EnumSelector<int>*      mSelectLogLevelAvcodec;
    wxChoice*               mLogLevelAvcodec;
    wxCheckBox*             mShowDebugInfoOnWidgets;

    wxPanel*                mPanel;     ///< tab:The topmost widget
    wxBoxSizer*             mTopSizer;  ///< tab:Sizer for panel
    wxBoxSizer*             mBoxSizer;  ///< box:Sizer for current box

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void addtab(const wxString& name);
    void addbox(const wxString& name);
    void addoption(const wxString& name, wxWindow* widget);
};

} // namespace

#endif // OPTIONS_H