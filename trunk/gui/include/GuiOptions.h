#ifndef GUI_OPTIONS_WINDOW_H
#define GUI_OPTIONS_WINDOW_H

#include <boost/optional.hpp>
#include <vector>
#include <wx/propdlg.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <boost/tuple/tuple.hpp>
#include "FrameRate.h"

class GuiOptions : public wxPropertySheetDialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // APPLICATION INITIALIZATION & CONFIGURATION
    //////////////////////////////////////////////////////////////////////////

    /** To be called upon startup. Distributes options to the various components. */
    static void init();

    //////////////////////////////////////////////////////////////////////////
    // SETTERS & GETTERS
    //////////////////////////////////////////////////////////////////////////

    static wxString getOptionsFileName();
    static wxString getLogFileName();

    static boost::optional<wxString> GuiOptions::GetAutoLoad();
    static void SetAutoLoadFilename(wxString filename);

    static model::FrameRate getDefaultFrameRate();

    //////////////////////////////////////////////////////////////////////////
    // CONFIGURATION DIALOG
    //////////////////////////////////////////////////////////////////////////

    GuiOptions(wxWindow* parent);
    ~GuiOptions();

private:
    wxCheckBox* mLoadLast; 
    wxChoice*   mLogLevel;
    wxRadioBox* mFrameRate;

    static wxString sConfigFile;
};

#endif // GUI_OPTIONS_WINDOW_H