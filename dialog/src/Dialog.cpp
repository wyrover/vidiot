#include "Dialog.h"

#include "Config.h"
#include "UtilLog.h"
#include "UtilThread.h"
#include "Window.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Dialog::Dialog()
    :   mDir(boost::none)
    ,   mFiles(boost::none)
    ,   mText(boost::none)
    ,   mButton(boost::none)
    ,   mDebugReportGenerated(false)
{
}

Dialog::~Dialog()
{
    // Asserts to ensure that no dialogs are missing
    ASSERT(!mDir);
    ASSERT(!mSaveFile);
    ASSERT(!mFiles);
    ASSERT(!mText);
    ASSERT(!mButton);
}

//////////////////////////////////////////////////////////////////////////
// DIALOG METHODS
//////////////////////////////////////////////////////////////////////////

void Dialog::setDir(wxString dir)
{
    ASSERT(!mDir);
    mDir = boost::optional<wxString>(dir);
}

wxString Dialog::getDir( const wxString & message, const wxString & default, wxWindow* parent )
{
    if (mDir)
    {
        wxString result = *mDir;
        mDir.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<wxString>(boost::bind(&wxDirSelector, message, default, wxDD_DEFAULT_STYLE, wxDefaultPosition, parent));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setSaveFile(wxString file)
{
    ASSERT(!mSaveFile);
    mSaveFile = boost::optional<wxString>(file);
}

wxString Dialog::getSaveFile( const wxString& message, const wxString& filetypes, const wxString& defaultpath, const wxString& defaultfilename, const wxString& defaultextension, wxWindow* parent )
{
    if (mSaveFile)
    {
        wxString result = *mSaveFile;
        mSaveFile.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<wxString>(boost::bind(&wxFileSelector, message, defaultpath, defaultfilename, defaultextension, filetypes, wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent, wxDefaultCoord, wxDefaultCoord ));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setFiles(std::list<wxString> files)
{
    ASSERT(!mFiles);
    mFiles = boost::optional<std::list< wxString > >(files);
}

wxStrings getFilesList( const wxString& message, const wxString& filetypes, wxWindow* parent )
{
    if (!parent) { parent = &Window::get(); }
    wxStrings result;
    wxString wildcards = wxString::Format(filetypes,wxFileSelectorDefaultWildcardStr,wxFileSelectorDefaultWildcardStr);
    wxFileDialog dialog(&gui::Window::get(), message, wxEmptyString, wxEmptyString, wildcards, wxFD_OPEN|wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString paths;
        dialog.GetPaths(paths);
        BOOST_FOREACH( wxString path, paths )
        {
            result.push_back(path);
        }
    }
    return result;
}

wxStrings Dialog::getFiles( const wxString& message, const wxString& filetypes, wxWindow* parent )
{
    if (mFiles)
    {
        wxStrings result = *mFiles;
        mFiles.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<wxStrings>(boost::bind(getFilesList, message, filetypes, parent));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setText(wxString text)
{
    ASSERT(!mText);
    mText = boost::optional<wxString>(text);
}

wxString Dialog::getText( wxString title, wxString message, wxString default, wxWindow* parent )
{
    if (mText)
    {
        wxString result = *mText;
        mText.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<wxString>(boost::bind(&wxGetTextFromUser, message, title, default, parent, wxDefaultCoord, wxDefaultCoord, true));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setConfirmation(int button)
{
    ASSERT(!mButton);
    mButton = boost::optional<int>(button);
}

int Dialog::getConfirmation( wxString title, wxString message, int buttons, wxWindow* parent )
{
    if (mButton)
    {
        int result = *mButton;
        mButton.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<int>(boost::bind(&wxMessageBox, message, title, buttons, parent, wxDefaultCoord, wxDefaultCoord));
}

//////////////////////////////////////////////////////////////////////////

int generateDebugReport(bool doexit, bool addcontext)
{
    VAR_ERROR(doexit);
    if (doexit && wxCANCEL == wxMessageBox("A fatal error was encountered. Press OK to generate debug report. Press Cancel to terminate.", "Error", wxOK | wxCANCEL, &Window::get()))
    {
        exit(-1);
        return 0;
    }

    wxDebugReportCompress report;

    if (addcontext)
    {
        report.AddCurrentContext();
        report.AddCurrentDump();
    }

    if (wxFileName(Config::getFileName()).FileExists())
    {
        report.AddFile(Config::getFileName(), wxT("options file"));
    }

    if (wxFileName(Log::getFileName()).FileExists())
    {
        report.AddFile(Log::getFileName(), wxT("text log file"));
    }

    if ( wxDebugReportPreviewStd().Show(report) )
    {
        report.Process();
    }
    if (doexit)
    {
        exit(-1);
    }

    return 0;
}

void Dialog::getDebugReport(bool doexit, bool addcontext)
{
    VAR_ERROR(doexit);
    if (!mDebugReportGenerated)
    {
       mDebugReportGenerated = true;
       util::thread::RunInMainAndWait([doexit, addcontext] { generateDebugReport(doexit, addcontext); });
    }
}

} // namespace