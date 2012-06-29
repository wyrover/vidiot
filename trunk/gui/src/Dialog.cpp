#include "Dialog.h"

#include <wx/dirdlg.h>
#include <wx/debugrpt.h>
#include <wx/filename.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/thread/barrier.hpp>
#include "Config.h"
#include "UtilLog.h"
#include "Window.h"

#include <wx/thread.h>

namespace gui {

/// Schedule a method to be run in the main wxWidgets thread.
/// This is implemented by submitting an event. In the handler
/// of the event (which is called in the main wxWidgets thread)
/// the given method is called.
/// Usage:
/// RETURNTYPE x = RunInMainThread(this, boost::bind(&Dialog::theMethod, this, x1, x2, ...)).getResult()
/// \pre !wxThread::IsMain()
template <class RETURNTYPE>
class RunInMainThread
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
public:

    typedef boost::function<RETURNTYPE()> Method;

    /// For methods with a preset
    RunInMainThread(boost::optional< RETURNTYPE >& preset, Method method)
        :   mMethod(method)
        ,   mBarrier(2)
    {
        if (preset)
        {
            mResult = *preset;
            preset.reset();
        }
        else
        {
            showDialog();
        }
    }

    /// For methods without a preset (show dialog always)
    RunInMainThread(Method method)
        :   mMethod(method)
        ,   mBarrier(2)
    {
        showDialog();
    }

    void showDialog()
    {
        if (wxThread::IsMain())
        {
            mResult = mMethod();
        }
        else
        {
            Bind( wxEVT_THREAD, &RunInMainThread::onThreadEvent, this );
            QueueEvent(new wxThreadEvent());
            mBarrier.wait();
        }
    }

    ~RunInMainThread()
    {
        Unbind( wxEVT_THREAD, &RunInMainThread::onThreadEvent, this );
    }

    void onThreadEvent(wxThreadEvent& event)
    {
        mResult = mMethod();
        mBarrier.wait();
    }

    RETURNTYPE getResult() const
    {
        return mResult;
    }

private:

    RETURNTYPE mResult;
    Method mMethod;
    boost::barrier mBarrier; ///< Used to join the thread instigating the dialog and the dialog closing (in the main thread)
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

static Dialog* sCurrent = 0;

Dialog::Dialog()
    :   mDir(boost::none)
    ,   mFiles(boost::none)
    ,   mText(boost::none)
    ,   mButton(boost::none)
    ,   mDebugReportGenerated(false)
{
    sCurrent = this;
}

Dialog::~Dialog()
{
    sCurrent = 0;
}

// static
Dialog& Dialog::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
}

//////////////////////////////////////////////////////////////////////////
// DIALOG METHODS
//////////////////////////////////////////////////////////////////////////

void Dialog::setDir(wxString dir)
{
    mDir = boost::optional<wxString>(dir);
}

wxString Dialog::getDir( const wxString & message, const wxString & default, wxWindow* parent )
{
    if (!parent) { parent = &Window::get(); }
    return RunInMainThread<wxString>(mDir, boost::bind(&wxDirSelector, message, default, wxDD_DEFAULT_STYLE, wxDefaultPosition, parent)).getResult();
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setSaveFile(wxString file)
{
    mSaveFile = boost::optional<wxString>(file);
}

wxString Dialog::getSaveFile( const wxString& message, const wxString& filetypes, const wxString& defaultpath, const wxString& defaultfilename, const wxString& defaultextension, wxWindow* parent )
{
    if (!parent) { parent = &Window::get(); }
    return RunInMainThread<wxString>(mSaveFile, boost::bind(&wxFileSelector, message, defaultpath, defaultfilename, defaultextension, filetypes, wxFD_SAVE | wxFD_OVERWRITE_PROMPT, parent, wxDefaultCoord, wxDefaultCoord )).getResult();
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setFiles(std::list<wxString> files)
{
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
    if (!parent) { parent = &Window::get(); }
    return RunInMainThread<wxStrings>(mFiles, boost::bind(getFilesList, message, filetypes, parent)).getResult();
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setText(wxString text)
{
    mText = boost::optional<wxString>(text);
}

wxString Dialog::getText( wxString title, wxString message, wxString default, wxWindow* parent )
{
    if (!parent) { parent = &Window::get(); }
    return RunInMainThread<wxString>(mText, boost::bind(&wxGetTextFromUser, message, title, default, parent, wxDefaultCoord, wxDefaultCoord, true)).getResult();
}

//////////////////////////////////////////////////////////////////////////

void Dialog::setConfirmation(int button)
{
    mButton = boost::optional<int>(button);
}

int Dialog::getConfirmation( wxString title, wxString message, int buttons, wxWindow* parent )
{
    if (!parent) { parent = &Window::get(); }
    return RunInMainThread<int>(mButton, boost::bind(&wxMessageBox, message, title, buttons, parent, wxDefaultCoord, wxDefaultCoord)).getResult();
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
       RunInMainThread<int>(boost::bind(&generateDebugReport, doexit, addcontext));
    }
}

} // namespace