#ifndef DIALOG_H
#define DIALOG_H

#include <list>
#include <wx/string.h>
#include <boost/optional.hpp>

typedef std::list<wxString> wxStrings;

namespace gui {

/// This class serves two purposes:
///
/// Purpose 1: Helper class for testing. In an automated test scenario, waiting until dialogs are visible is cumbersome and error
/// prone (in fact, I didn't succeed in finding a good event for signaling the first opening of the dir dialog). Therefore, this 
/// class provides an abstraction. Normally, nothing special is done and handling is done by the regular wxWidgets dialogs. In 
/// case of automated tests, the behaviour can be overruled by calling 'set*' just before triggering a dialog with 'get*'.
///
/// Purpose 2: Ensure that any dialog is created via the main wxWidgets thread. Creating dialogs (in fact, any window) from any 
/// thread other than the main wxWidgets thread does not work (wxWidgets limitation). Creating dialogs via this class ensures 
/// that the dialog is created on the main thread, and the calling thread is blocked until the dialog has been dismissed.
class Dialog
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit Dialog();
    virtual ~Dialog();

    static Dialog& get();

    //////////////////////////////////////////////////////////////////////////
    // DIALOG METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Set a fixed outcome for the next directory selection for testing.
    /// \param dir fixed directory which will be returned for the next getDir() call
    void setDir(wxString dir);

    /// Open a directory dialog. If before this call setDir() was called, the dir given in setDir() is returned and that directory is reset.
    /// \post !sDir 
    wxString getDir( const wxString & message, const wxString & default );

    /// Set a fixed outcome for the next files selection for testing.
    /// \param files fixed list of absolute file names which will be returned for the next getFiles() call
    void setFiles(std::list<wxString> files);

    /// Open a files selection dialog. If before this call setFiles() was called, the files given in
    /// setFiles() are returned and that list is reset.
    /// In case the dialog is aborted or no file is selected for another reason, an empty list is returned.
    /// \post !sFiles
    wxStrings getFiles( const wxString& message, const wxString& filetypes );

    /// Set a fixed outcome for the next text selection for testing.
    /// \param text fixed text which will be returned for the next getText() call
    /// \post sText
    void setText(wxString dir);

    /// Open a text dialog. If before this call setText() was called, the text 
    /// given in setText() is returned and that text is reset.
    /// \post !sText 
    wxString getText( wxString title, wxString message, wxString default );

    /// Set a fixed outcome for the next message dialog.
    /// \param button fixed button value (wxYES, wxNO, wxCANCEL, wxOK or wxHELP) to be returned by next getConfirmation(); 
    /// \post sButton
    void setConfirmation(int button);

    /// Show a message dialog requiring a button press to be dismissed
    /// \post !sButton
    /// \see wxMessageBox()
    /// \return wxYES, wxNO, wxCANCEL, wxOK, or wxHELP (whichever button was pressed)
    /// \param caption title text
    /// \param message message text in dialog
    /// \param buttons wxYES | wxNO | wxCANCEL | wxOK | wxHELP (use whichever should apply)
    int getConfirmation( wxString title, wxString message, int buttons = wxOK);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::optional<wxString> mDir;
    boost::optional<wxStrings> mFiles;
    boost::optional<wxString> mText;
    boost::optional<int> mButton;

};

} // namespace

#endif // DIALOG_H