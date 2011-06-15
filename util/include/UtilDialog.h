#ifndef UTIL_DIALOG_H
#define UTIL_DIALOG_H

#include <wx/string.h>
#include <wx/window.h>
#include <boost/optional.hpp>

// Helper class for testing. In an automated test scenario,
// waiting until dialogs are visible is cumbersome and error
// prone (in fact, I didn't succeed in finding a good event
// for signaling the first opening of the dir dialog).
//
// Therefore, this class provides an abstraction. Normally,
// nothing special is done and handling is done by the 
// regular wxWidgets dialogs. In case of automated tests,
// the behaviour can be overruled by calling 'set*' just
// before triggering a dialog with 'get*'.
class UtilDialog
{
public:

    /// Set a fixed outcome for the next directory selection for testing.
    /// \param dir fixed directory which will be returned for the next getDir() call
    static void setDir(wxString dir);

    /// Open a directory dialog. If before this call setDir() was called, the dir given in setDir() is returned and that directory is reset.
    /// \post !sDir 
    static wxString getDir( const wxString & message, const wxString & default_path, wxWindow* parent );

private:

    static boost::optional<wxString> sDir;

};

#endif // UTIL_DIALOG_H