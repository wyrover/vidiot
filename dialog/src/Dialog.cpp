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

#include "Dialog.h"

#include "Config.h"
#include "UtilLog.h"
#include "UtilThread.h"
#include "Window.h"
#include <wx/choicdlg.h>
#include <wx/dcscreen.h>
#include <wx/debugrpt.h>
#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Dialog::Dialog()
    :   mDir(boost::none)
    ,   mFiles(boost::none)
    ,   mText(boost::none)
    ,   mButton(boost::none)
    ,   mStringsSelection(boost::none)
    ,   mIncludeScreenshot(Config::ReadBool(Config::sPathDebugIncludeScreenShot))
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
        for ( wxString path : paths )
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

void Dialog::setComboText(wxString text)
{
    ASSERT(!mComboText);
    mComboText = boost::optional<wxString>(text);
}

wxString Dialog::getComboText( wxString title, wxString message, std::list<wxString> entries, wxString default, wxWindow* parent )
{
    if (mComboText)
    {
        wxString result = *mText;
        mText.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    wxArrayString choices;
    int initial = 0;
    int i = 0;
    for ( wxString entry : entries )
    {
        ASSERT(!entry.IsEmpty()); // Empty string is returned upon Cancel
        choices.Add(entry);
        if (entry.IsSameAs(default))
        {
            initial = i;
        }
        ++i;
    }
    return util::thread::RunInMainReturning<wxString>(boost::bind(&wxGetSingleChoice, message, title, choices, initial, parent));
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

void Dialog::setStringsSelection(std::list<wxString> stringsSelection)
{
    ASSERT(!mStringsSelection);
    mStringsSelection.reset(stringsSelection);
}

std::list<wxString> Dialog::getStringsSelection( wxString title, wxString message, std::list<wxString> options, wxWindow* parent )
{
    if (mStringsSelection)
    {
        std::list<wxString> result = * mStringsSelection;
        mStringsSelection.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }

    wxArrayInt selected;
    wxArrayString choices;
    int i = 0;
    for ( wxString selection : options )
    {
        choices.Add(selection);
        selected.Add(i);
        ++i;
    }

    int ok = util::thread::RunInMainReturning<int>([&selected,message,title,choices,parent]() -> int
    {
        return wxGetSelectedChoices(selected, message, title, choices, parent);
    });

    std::list<wxString> result;
    if (ok != -1) // -1 == cancel
    {
        for ( int selection : selected )
        {
            result.push_back(choices.Item(selection));
        }
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////

int generateDebugReport(bool doexit, bool addcontext, bool screenShot, wxRect screenRect)
{
    VAR_ERROR(doexit);
    if (doexit && wxCANCEL == wxMessageBox("A fatal error was encountered. Press OK to generate debug report. Press Cancel to terminate.", "Error", wxOK | wxCANCEL, &Window::get()))
    {
        exit(-1);
        return 0;
    }

    wxDebugReportCompress report;

    // todo clean old log files (store in ini?)

    if (screenShot)
    {
        // Screen shot added first. This to reduce the changes of the report taking a long time and the
        // user then moving focus to another window.

        // Ensure that 'fatal error encountered' dialog is gone.
        boost::this_thread::sleep(boost::posix_time::milliseconds(250));

        wxFileName screenShotFile(wxStandardPaths::Get().GetTempDir().GetFullPath(),""); // Store in TEMP // todo is now stored one too high?
        wxString nameWithProcessId; nameWithProcessId << "vidiot_screenshot_" << wxGetProcessId();
        screenShotFile.SetName(nameWithProcessId);
        screenShotFile.SetExt("png");
        wxScreenDC screen;
        wxMemoryDC memory;
        wxBitmap screenshot(screenRect.width, screenRect.height);
        memory.SelectObject (screenshot);
        memory.Blit (0, 0, screenRect.width, screenRect.height, &screen, screenRect.x, screenRect.y);
        memory.SelectObject (wxNullBitmap);
        screenshot.SaveFile (screenShotFile.GetFullPath(), wxBITMAP_TYPE_PNG); 
        report.AddFile(screenShotFile.GetFullPath(), wxT("Screen shot"));
    }

    if (addcontext)
    {
        report.AddCurrentContext();
        report.AddCurrentDump();
    }

    if (wxFileName(Config::getFileName()).FileExists())
    {
        report.AddFile(Config::getFileName(), wxT("Options file"));
    }

    if (wxFileName(Log::getFileName()).FileExists())
    {
        report.AddFile(Log::getFileName(), wxT("Log file"));
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
       util::thread::RunInMainAndWait([this, doexit, addcontext] { generateDebugReport(doexit, addcontext, mIncludeScreenshot, mScreenRect); });
    }
}

} // namespace