#ifndef UTIL_DIALOG_H
#define UTIL_DIALOG_H

#include <list>
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
    static wxString getDir( const wxString& message, const wxString& default, wxWindow* parent );

    /// Set a fixed outcome for the next files selection for testing.
    /// \param files fixed list of absolute file names which will be returned for the next getFiles() call
    static void setFiles(std::list<wxString> files);

    /// Open a files selection dialog. If before this call setFiles() was called, the files given in
    /// setFiles() are returned and that list is reset.
    /// In case the dialog is aborted or no file is selected for another reason, an empty list is returned.
    /// \post !sFiles
    static std::list<wxString> getFiles( const wxString& message, const wxString& filetypes, wxWindow* parent );

    /// Set a fixed outcome for the next text selection for testing.
    /// \param text fixed text which will be returned for the next getText() call
    /// \post sText
    static void setText(wxString dir);

    /// Open a text dialog. If before this call setText() was called, the text 
    /// given in setText() is returned and that text is reset.
    /// \post !sText 
    static wxString getText( const wxString& title, const wxString& message, const wxString& default, wxWindow* parent );

private:

    static boost::optional<wxString> sDir;
    static boost::optional< std::list<wxString> > sFiles;
    static boost::optional<wxString> sText;

};

#endif // UTIL_DIALOG_H