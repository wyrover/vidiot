#ifndef APPLICATION_H
#define APPLICATION_H

#include <wx/app.h>
#include <boost/thread.hpp>
#include "UtilAssert.h"
#include "UtilEvent.h"

#ifdef _MSC_VER
#define CATCH_ALL_ERRORS
#endif // _MSC_VER

namespace test {
struct IEventLoopListener;
}

namespace gui {

DECLARE_EVENT(EVENT_IDLE_TRIGGER, EventIdleTrigger, bool);

class Application
    :   public wxApp
    ,   public IAssert
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Application(test::IEventLoopListener* eventLoopListener = 0);
    ~Application();

    static const wxString sTestApplicationName; ///< Fixed string used to determine if application is running in module test

    //////////////////////////////////////////////////////////////////////////
    // IDLE HANDLING
    //////////////////////////////////////////////////////////////////////////

    /// Original implementation:
    ///    wxWakeUpIdle();
    ///    mCondition.wait(lock);
    /// However, it is possible that the idle event is received between these
    /// two statements. This results in a (temporary) hangup of the tests. After
    /// a while another idle event is received (guess), which resolves the hangup.
    /// To avoid this, first an event is generated. This causes a method to be 
    /// called on the event loop. When wxWakeUpIdle() is called in that method, 
    /// the aforementioned interleaving problem cannot occur.
    void triggerIdle(EventIdleTrigger& event);

    /// Helper method for testing. Somehow the Idle events are only correctly
    /// received by this class. Therefore, the 'wait for idle' used in testing
    /// uses the implementation here.
    void waitForIdle();

    //////////////////////////////////////////////////////////////////////////
    // GUI CALLBACKS
    //////////////////////////////////////////////////////////////////////////

    bool OnInit();
    int OnRun();
    void OnEventLoopEnter(wxEventLoopBase* loop);
    int OnExit();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onIdle(wxIdleEvent& event);

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

    test::IEventLoopListener* mEventLoopListener;
    boost::condition_variable mConditionIdle;
    boost::mutex mMutexIdle;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    bool inTestMode() const;

};

} // namespace

#endif // APPLICATION_H
