#ifndef APPLICATION_H
#define APPLICATION_H

#include <wx/app.h>
#include "UtilAssert.h"

#ifdef _MSC_VER
#define CATCH_ALL_ERRORS
#endif // _MSC_VER

namespace gui {

/// Helper interface. Used to signal in the tests that the event loop
/// has been activated, thus the application is properly started
/// for running tests.
struct IEventLoopListener
{
    virtual void OnEventLoopEnter() = 0;
};

class Application
    :   public wxApp
    ,   public IAssert
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Application(IEventLoopListener* eventLoopListener = 0);
    ~Application();

	static const wxString sTestApplicationName; ///< Fixed string used to determine if application is running in module test

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    bool OnInit();
    int OnRun();
    void OnEventLoopEnter(wxEventLoopBase* loop);
    int OnExit();

#ifdef CATCH_ALL_ERRORS
    virtual void OnAssertFailure(const wxChar *file, int Line, const wxChar *func, const wxChar *cond, const wxChar *msg);
    virtual bool OnExceptionInMainLoop();
    virtual void OnUnhandledException();
    virtual void OnFatalException();
#endif //CATCH_ALL_ERRORS

    //////////////////////////////////////////////////////////////////////////
    // IASSERT
    //////////////////////////////////////////////////////////////////////////

    void onAssert();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    IEventLoopListener* mEventLoopListener;
};

} // namespace

#endif // APPLICATION_H
