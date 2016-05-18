// Copyright 2013-2016 Eric Raijmakers.
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

#include "CommandLine.h"
#include "Config.h"
#include "File.h"
#include "UtilMail.h"
#include "UtilPath.h"
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
    : mDir(boost::none)
    , mFile(boost::none)
    , mFiles(boost::none)
    , mText(boost::none)
    , mButton(boost::none)
    , mStringsSelection(boost::none)
    , mIncludeScreenshot(Config::get().read<bool>(Config::sPathDebugIncludeScreenShotInDump))
    , mDebugReportGenerated(false)
{
}

Dialog::~Dialog()
{
    // Asserts to ensure that no dialogs are missing
    ASSERT(!mDir);
    ASSERT(!mSaveFile);
    ASSERT(!mFile);
    ASSERT(!mFiles);
    ASSERT(!mText);
    ASSERT(!mButton);
}

// static
wxString Dialog::getMediaFileTypes()
{
    static wxString sSupported{ model::File::sSupportedVideoExtensions + ";" + model::File::sSupportedAudioExtensions + ";" + model::File::sSupportedImageExtensions };

    static wxString result{
        _("Supported files") + "|" + sSupported + ";" + sSupported.Upper() + "|" +
        _("Video files") + " (" + model::File::sSupportedVideoExtensions + ")|" + model::File::sSupportedVideoExtensions + ";" + model::File::sSupportedVideoExtensions.Upper() + "|" +
        _("Audio files") + " (" + model::File::sSupportedAudioExtensions + ")|" + model::File::sSupportedAudioExtensions + ";" + model::File::sSupportedAudioExtensions.Upper() + "|" +
        _("Images") + " (" + model::File::sSupportedImageExtensions + ")|" + model::File::sSupportedImageExtensions + ";" + model::File::sSupportedImageExtensions.Upper() + "|" +
        _("All files") + " (%s)|%s" };
    return result;
}

//////////////////////////////////////////////////////////////////////////
// DIALOG METHODS
//////////////////////////////////////////////////////////////////////////

void Dialog::setDir(const wxString& dir)
{
    ASSERT(!mDir);
    mDir = boost::optional<wxString>(dir);
}

wxString Dialog::getDir(const wxString& message, const wxString& defaultValue, wxWindow* parent)
{
    if (mDir)
    {
        wxString result = *mDir;
        mDir.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<wxString>(std::bind(&wxDirSelector, message, defaultValue, wxDD_DEFAULT_STYLE, wxDefaultPosition, parent));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setSaveFile(const wxString& file)
{
    ASSERT(!mSaveFile);
    mSaveFile = boost::optional<wxString>(file);
}

wxString Dialog::getSaveFile(const wxString& message, const wxString& filetypes, const wxString& defaultpath, const wxString& defaultfilename, const wxString& defaultextension, wxWindow* parent )
{
    if (mSaveFile)
    {
        wxString result = *mSaveFile;
        mSaveFile.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<wxString>(std::bind(&wxFileSelector, message, defaultpath, defaultfilename, defaultextension, filetypes, wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent, wxDefaultCoord, wxDefaultCoord ));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setFile(const wxString& file)
{
    ASSERT(!mFile);
    mFile = boost::optional<wxString>(file);
}

wxString Dialog::getFile(const wxString& message, const wxString& defaultPath, const wxString& filetypes, wxWindow* parent)
{
    if (mFile)
    {
        wxString result = *mFile;
        mFile.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<wxString>(std::bind(&wxFileSelector, message, defaultPath, wxEmptyString, wxEmptyString, filetypes, wxFD_FILE_MUST_EXIST | wxFD_OPEN, parent, wxDefaultCoord, wxDefaultCoord));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setFiles(const wxStrings& files)
{
    ASSERT(!mFiles);
    mFiles = boost::optional<wxStrings>(files);
}

wxStrings getFilesList(const wxString& message, const wxString& filetypes, wxWindow* parent)
{
    if (!parent) { parent = &Window::get(); }
    wxStrings result;
    wxString wildcards = wxString::Format(filetypes, wxFileSelectorDefaultWildcardStr, wxFileSelectorDefaultWildcardStr);
    wxFileDialog dialog(&gui::Window::get(), message, wxEmptyString, wxEmptyString, wildcards, wxFD_OPEN | wxFD_MULTIPLE);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxArrayString paths;
        dialog.GetPaths(paths);
        for (wxString path : paths)
        {
            result.emplace_back(path);
        }
    }
    return result;
}

wxStrings Dialog::getFiles(const wxString& message, const wxString& filetypes, wxWindow* parent)
{
    if (mFiles)
    {
        wxStrings result = *mFiles;
        mFiles.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<wxStrings>(std::bind(getFilesList, message, filetypes, parent));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setText(const wxString& text)
{
    ASSERT(!mText);
    mText = boost::optional<wxString>(text);
}

wxString Dialog::getText(const wxString& title, const wxString& message, const wxString& defaultValue, wxWindow* parent )
{
    if (mText)
    {
        wxString result = *mText;
        mText.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<wxString>(std::bind(&wxGetTextFromUser, message, title, defaultValue, parent, wxDefaultCoord, wxDefaultCoord, true));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setComboText(const wxString& text)
{
    ASSERT(!mComboText);
    mComboText = boost::optional<wxString>(text);
}

wxString Dialog::getComboText(const wxString& title, const wxString& message, const wxStrings& entries, const wxString& defaultValue, wxWindow* parent )
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
        if (entry.IsSameAs(defaultValue))
        {
            initial = i;
        }
        ++i;
    }
    return util::thread::RunInMainReturning<wxString>([message, title, choices, initial, parent]
    {
        return wxGetSingleChoice(message, title, choices, initial, parent);
    });
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setConfirmation(int button)
{
    ASSERT(!mButton);
    mButton = boost::optional<int>(button);
}

int Dialog::getConfirmation(const wxString& title, const wxString& message, int buttons, wxWindow* parent )
{
    if (mButton)
    {
        int result = *mButton;
        mButton.reset();
        return result;
    }
    if (!parent) { parent = &Window::get(); }
    return util::thread::RunInMainReturning<int>(std::bind(&wxMessageBox, message, title, buttons, parent, wxDefaultCoord, wxDefaultCoord));
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setStringsSelection(const wxStrings& stringsSelection)
{
    ASSERT(!mStringsSelection);
    mStringsSelection.reset(stringsSelection);
}

wxStrings Dialog::getStringsSelection(const wxString& title, const wxString& message, const wxStrings& options, wxWindow* parent )
{
    if (mStringsSelection)
    {
        wxStrings result = * mStringsSelection;
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

    wxStrings result;
    if (ok != -1) // -1 == cancel
    {
        for ( int selection : selected )
        {
            result.emplace_back(choices.Item(selection));
        }
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////

int generateDebugReport(bool doexit, bool addcontext, bool screenShot, const wxRect& screenRect)
{
    VAR_ERROR(doexit);
    bool include = false;
    bool send = true;
    wxString title;
    wxString text;
    boost::optional<wxString> zipfile = boost::none;

    if (doexit)
    {
        if (wxCANCEL == wxMessageBox(_("A fatal error was encountered. Press OK to generate debug report. Press Cancel to terminate."), _("Error"), wxOK | wxCANCEL, &Window::get()))
        {
            exit(-1);
            return 0;
        }
        include = true;
        // TRANSLATORS: %s == Name of application (executable)
        title = wxString::Format(_("%s crash report"), CommandLine::get().ExeName);
        text = wxString::Format(_("I'm sorry, but %1$s crashed.\nBy sending this mail you'll provide me with helpful information for resolving the crash.\nThanks for your help.\n\nEric\n"), CommandLine::get().ExeName);
    }
    else
    {
        include = wxYES == wxMessageBox(_("Do you want to include debugging information (log file, screen shot, etc.) with your request?"), _("Submit"), wxYES | wxNO, &Window::get());
        // TRANSLATORS: %s == Name of application (executable)
        title = wxString::Format(_("%s bug report/feature request"), CommandLine::get().ExeName);
        text = wxString::Format(_("\n\nEnter your request/remark here.\n\nThanks for your help.\n\nEric\n"), CommandLine::get().ExeName);
    }

    if (include)
    {
        wxDebugReportCompress report;

        if (screenShot)
        {
            // Screen shot added first. This to reduce the changes of the report taking a long time and the
            // user then moving focus to another window.

            // Ensure that 'fatal error encountered' dialog is gone.
            boost::this_thread::sleep(boost::posix_time::milliseconds(250));

            wxFileName screenShotFile(wxStandardPaths::Get().GetTempDir(), ""); // Store in TEMP
            wxString nameWithProcessId; nameWithProcessId << "screenshot_" << wxGetProcessId();
            screenShotFile.SetName(nameWithProcessId);
            screenShotFile.SetExt("png");
            wxScreenDC screen;
            wxMemoryDC memory;
            if (!screenRect.IsEmpty())
            {
                wxBitmap screenshot(screenRect.width, screenRect.height);
                memory.SelectObject(screenshot);
                memory.Blit(0, 0, screenRect.width, screenRect.height, &screen, screenRect.x, screenRect.y);
                memory.SelectObject(wxNullBitmap);
                screenshot.SaveFile(screenShotFile.GetLongPath(), wxBITMAP_TYPE_PNG);
                report.AddFile(screenShotFile.GetLongPath(), wxT("Screen shot"));
            }
        }

        if (addcontext)
        {
            report.AddCurrentContext();
#ifdef _MSC_VER
            report.AddCurrentDump();
#endif
        }

        if (util::path::getConfigFilePath().FileExists())
        {
            report.AddFile(util::path::getConfigFilePath().GetLongPath(), wxT("Options file"));
        }

        if (util::path::getLogFilePath().FileExists())
        {
            report.AddFile(util::path::getLogFilePath().GetLongPath(), wxT("Log file"));
        }

        if (wxDebugReportPreviewStd().Show(report))
        {
            report.Process();
            wxString original{ report.GetCompressedFileName() };
            wxFileName copy{ original };
            copy.SetExt("rpt"); // Sourcefourge doesn't allow zip files...
            zipfile.reset(copy.GetLongPath());
            send = wxCopyFile(original, copy.GetLongPath(), false); // If file copy fails, just exit (mail won't be helpful anyway).
        }
        else
        {
            send = false; // If cancel is pressed, abort the generation altogether (even if it's a feature request, the button says Cancel and should behave as Cancel)
        }
    }

    if (send)
    {
        util::mail::sendDebugReport(title, text, zipfile);
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
        if (doexit)
        {
            // Only generate one report upon crashes.
            mDebugReportGenerated = true;
        }
        util::thread::RunInMainAndWait([this, doexit, addcontext] { generateDebugReport(doexit, addcontext, mIncludeScreenshot, mScreenRect); });
    }
}

} // namespace
