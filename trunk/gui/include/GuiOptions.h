#ifndef GUI_OPTIONS_WINDOW_H
#define GUI_OPTIONS_WINDOW_H

#include <vector>
#include <wx/propdlg.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <boost/optional.hpp>
#include <boost/tuple/tuple.hpp>
#include "FrameRate.h"

namespace gui {

class GuiOptions
    :   public wxPropertySheetDialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // APPLICATION INITIALIZATION & CONFIGURATION
    //////////////////////////////////////////////////////////////////////////

    /// To be called upon startup. Distributes options to the various components.
    /// @see distributeOptions()
    static void init();

    /// This distributes the current set of options to the various other components.
    /// Done initially, and after Tools->Options->Ok.
    /// May not log as it is also called in the beginning of GuiMain::OnInit(), 
    /// before the logging is initialized.
    static void distributeOptions();

    //////////////////////////////////////////////////////////////////////////
    // SETTERS & GETTERS
    //////////////////////////////////////////////////////////////////////////

    static wxString getOptionsFileName();
    static wxString getLogFileName();
    static bool     getShowDebugInfoOnWidgets();

    static boost::optional<wxString> GetAutoLoad();
    static void SetAutoLoadFilename(wxString filename);

    static model::FrameRate getDefaultFrameRate();

    static double getMarkerBeginAddition();
    static double getMarkerEndAddition();

    //////////////////////////////////////////////////////////////////////////
    // CONFIGURATION DIALOG
    //////////////////////////////////////////////////////////////////////////

    GuiOptions(wxWindow* parent);
    ~GuiOptions();

private:

    //////////////////////////////////////////////////////////////////////////
    // GENERAL
    //////////////////////////////////////////////////////////////////////////

    wxCheckBox* mLoadLast;

    //////////////////////////////////////////////////////////////////////////
    // VIDEO
    //////////////////////////////////////////////////////////////////////////

    wxRadioBox* mFrameRate;

    //////////////////////////////////////////////////////////////////////////
    // TIMELINE
    //////////////////////////////////////////////////////////////////////////

    wxSpinCtrlDouble*   mMarkerBeginAddition;
    wxSpinCtrlDouble*   mMarkerEndAddition;

    //////////////////////////////////////////////////////////////////////////
    // DEBUG
    //////////////////////////////////////////////////////////////////////////

    wxChoice*   mLogLevel;
    wxCheckBox* mShowDebugInfoOnWidgets;

    static bool sShowDebugInfoOnWidgets; ///< Cached for performance

    //////////////////////////////////////////////////////////////////////////
    // CONFIG FILE PATH
    //////////////////////////////////////////////////////////////////////////

    static wxString sConfigFile;
};

} // namespace

#endif // GUI_OPTIONS_WINDOW_H
