#ifndef OPTIONS_H
#define OPTIONS_H

#include <wx/propdlg.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>

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

    wxCheckBox* mLoadLast;

    wxRadioBox* mFrameRate;

    wxSpinCtrlDouble*   mMarkerBeginAddition;
    wxSpinCtrlDouble*   mMarkerEndAddition;
    wxTextCtrl*         mStrip;

    wxChoice*   mLogLevel;
    wxCheckBox* mShowDebugInfoOnWidgets;

    wxPanel* mPanel;       ///< tab:The topmost widget
    wxBoxSizer* mTopSizer;  ///< tab:Sizer for panel
    wxBoxSizer* mBoxSizer;  ///< box:Sizer for current box

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void addtab(const wxString& name);
    void addbox(const wxString& name);
    void addoption(const wxString& name, wxWindow* widget);
};

} // namespace

#endif // OPTIONS_H
