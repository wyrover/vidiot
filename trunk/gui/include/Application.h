#ifndef APPLICATION_H
#define APPLICATION_H

#include "UtilAssert.h"
#include "UtilEvent.h"

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

    /// Helper method for testing. Somehow the Idle events are only correctly
    /// received by this class. Therefore, the 'wait for idle' used in testing
    /// uses the implementation here.
    /// This method is steps 1 and 4 of the waitForIdle mechanism:
    /// Trigger an event initiating the idle mechanism, and start the wait
    /// until the idle sequence has finished. Thus, this method blocks until
    /// idle seen.
    void waitForIdle();

    /// Original implementation:
    ///    wxWakeUpIdle();
    ///    mCondition.wait(lock);
    /// However, it is possible that the idle event is received between these
    /// two statements. This results in a (temporary) hangup of the tests. After
    /// a while another idle event is received (guess), which resolves the hangup.
    /// To avoid this, first an event is generated. This causes a method to be
    /// called on the event loop. When wxWakeUpIdle() is called in that method,
    /// the aforementioned interleaving problem cannot occur.
    /// This method is step 2 of the waitForIdle mechanism: handle the
    /// IdleTrigger event and consequently initiate step 3: wxWaitForIdle.
    void triggerIdle(EventIdleTrigger& event);

    /// This method is step 3 of the waitForIdle mechanism: after wxWidgets
    /// signals the Idle event, we know that it was not an already pending
    /// idle event (bec
    void onIdle(wxIdleEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // GUI CALLBACKS
    //////////////////////////////////////////////////////////////////////////

    bool OnInit() override;
    int OnRun() override;
    void OnEventLoopEnter(wxEventLoopBase* loop) override;
    int OnExit() override;
    void OnAssertFailure(const wxChar *file, int Line, const wxChar *function, const wxChar *condition, const wxChar *message) override;
    bool OnExceptionInMainLoop() override;
    void OnUnhandledException() override;
    void OnFatalException() override;

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
};

} // namespace

#endif // APPLICATION_H