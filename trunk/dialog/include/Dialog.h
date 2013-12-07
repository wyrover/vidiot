// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef DIALOG_H
#define DIALOG_H

#include "UtilSingleInstance.h"

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
    :   public SingleInstance<Dialog>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit Dialog();
    virtual ~Dialog();

    //////////////////////////////////////////////////////////////////////////
    // DIALOG METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Set a fixed outcome for the next directory selection for testing.
    /// \param dir fixed directory which will be returned for the next getDir() call
    void setDir(wxString dir);

    /// Open a directory dialog. If before this call setDir() was called, the dir given in setDir() is returned and that directory is reset.
    /// \post !MDir
    wxString getDir( const wxString & message, const wxString & default, wxWindow* parent = 0 );

    //////////////////////////////////////////////////////////////////////////

    /// Set a fixed outcome for the next 'save file' selection for testing.
    /// \param files fixed file to be returned for saving
    void setSaveFile(wxString file);

    /// Open a file selection dialog for saving. If before this call setSaveFile() was called, the file given in
    /// setSaveFile() is returned and the preset file is reset.
    /// In case the dialog is aborted or no file is selected for another reason, an empty string is returned.
    /// \post !mSaveFile
    wxString getSaveFile( const wxString& message, const wxString& filetypes, const wxString& defaultpath, const wxString& defaultfilename, const wxString& defaultextension, wxWindow* parent = 0 );

    //////////////////////////////////////////////////////////////////////////

    /// Set a fixed outcome for the next files selection for testing.
    /// \param files fixed list of absolute file names which will be returned for the next getFiles() call
    void setFiles(std::list<wxString> files);

    /// Open a files selection dialog. If before this call setFiles() was called, the files given in
    /// setFiles() are returned and that list is reset.
    /// In case the dialog is aborted or no file is selected for another reason, an empty list is returned.
    /// \post !sFiles
    wxStrings getFiles( const wxString& message, const wxString& filetypes, wxWindow* parent = 0 );

    //////////////////////////////////////////////////////////////////////////

    /// Set a fixed outcome for the next text selection for testing.
    /// \param text fixed text which will be returned for the next getText() call
    /// \post sText
    void setText(wxString dir);

    /// Open a text dialog. If before this call setText() was called, the text
    /// given in setText() is returned and that text is reset.
    /// \post !mText
    wxString getText( wxString title, wxString message, wxString default, wxWindow* parent = 0 );

    //////////////////////////////////////////////////////////////////////////

    /// Set a fixed outcome for the next combo box selection for testing.
    /// \param text fixed text which will be returned for the next getComboText() call
    /// \post sComboText
    void setComboText(wxString text);

    /// Open a text dialog. If before this call setComboText() was called, the text
    /// given in setComboText() is returned and that text is reset.
    /// \post !mComboText
    wxString getComboText( wxString title, wxString message, std::list<wxString> entries, wxString default = "", wxWindow* parent = 0 );

    //////////////////////////////////////////////////////////////////////////

    /// Set a fixed outcome for the next message dialog.
    /// \param button fixed button value (wxYES, wxNO, wxCANCEL, wxOK or wxHELP) to be returned by next getConfirmation();
    /// \post sButton
    void setConfirmation(int button = wxOK);

    /// Show a message dialog requiring a button press to be dismissed
    /// \post !mButton
    /// \see wxMessageBox()
    /// \return wxYES, wxNO, wxCANCEL, wxOK, or wxHELP (whichever button was pressed)
    /// \param caption title text
    /// \param message message text in dialog
    /// \param buttons wxYES | wxNO | wxCANCEL | wxOK | wxHELP (use whichever should apply)
    int getConfirmation( wxString title, wxString message, int buttons = wxOK, wxWindow* parent = 0);

    //////////////////////////////////////////////////////////////////////////

    /// Set a fixed outcome for the next string selection for testing.
    /// \param selection fixed list of selected strings which will be returned for the next getStringsSelection() call
    void setStringsSelection(std::list<wxString> stringsSelection);

    /// Open a files selection dialog. If before this call setFiles() was called, the files given in
    /// setFiles() are returned and that list is reset.
    /// In case the dialog is aborted or no file is selected for another reason, an empty list is returned.
    /// \post !sFiles
    std::list<wxString> getStringsSelection( wxString title, wxString message, std::list<wxString> options, wxWindow* parent = 0 );

    //////////////////////////////////////////////////////////////////////////

    /// Show a debug report. In some scenarios adding a context/dump
    /// leads to hangups (particularly, wxWidgets assert in a separate
    /// thread), so that behaviour can be disabled via the addcontext flag.
    /// \param exit if true, application is exited via exit(-1)
    /// \param addcontext if true, then add context and dump
    void getDebugReport(bool doexit = true, bool addcontext = true);

    static bool sIncludeScreenshot; ///< Cached, since lookup (of the config setting) might fail when crashing
    static wxRect sScreenRect;      ///< Cached, since accessing the window object might fail when crashing

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    boost::optional<wxString> mDir;
    boost::optional<wxString> mSaveFile;
    boost::optional<wxStrings> mFiles;
    boost::optional<wxString> mText;
    boost::optional<wxString> mComboText;
    boost::optional<int> mButton;
    boost::optional<std::list<wxString>> mStringsSelection;

    /// Only one report is generated. After that application should end.
    /// Handling more errors after the initial fatal one serves no purpose
    /// and leads to more errors.
    bool mDebugReportGenerated;
};

} // namespace

#endif // DIALOG_H