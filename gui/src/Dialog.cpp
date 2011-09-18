#include "Dialog.h"

#include <wx/dirdlg.h>
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/thread/barrier.hpp>
#include "UtilLog.h"
#include "Window.h"

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

    explicit RunInMainThread(boost::optional< RETURNTYPE >& preset, Method method)
        :   wxEvtHandler()
        ,   mPreset(preset)
        ,   mMethod(method)
        ,   mBarrier(2)
    {
        if (mPreset)
        {
            mResult = *mPreset;
            mPreset.reset();
        }
        else
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

    boost::optional< RETURNTYPE >& mPreset;
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

wxString Dialog::getDir( const wxString & message, const wxString & default )
{
    return RunInMainThread<wxString>(mDir, boost::bind(&wxDirSelector, message, default, wxDD_DEFAULT_STYLE, wxDefaultPosition, &Window::get())).getResult();
}

void Dialog::setFiles(std::list<wxString> files)
{
    mFiles = boost::optional<std::list< wxString > >(files);
}

wxStrings getFilesList( const wxString& message, const wxString& filetypes )
{
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

wxStrings Dialog::getFiles( const wxString& message, const wxString& filetypes )
{
    return RunInMainThread<wxStrings>(mFiles, boost::bind(getFilesList, message, filetypes)).getResult();
}

void Dialog::setText(wxString text)
{
    mText = boost::optional<wxString>(text);
}

wxString Dialog::getText( wxString title, wxString message, wxString default )
{
    return RunInMainThread<wxString>(mText, boost::bind(&wxGetTextFromUser, message, title, default, &Window::get(), wxDefaultCoord, wxDefaultCoord, true)).getResult();
}

void Dialog::setConfirmation(int button)
{
    mButton = boost::optional<int>(button);
}

int Dialog::getConfirmation( wxString title, wxString message, int buttons )
{
    return RunInMainThread<int>(mButton, boost::bind(&wxMessageBox, message, title, buttons, &Window::get(), wxDefaultCoord, wxDefaultCoord)).getResult();
}

} // namespace