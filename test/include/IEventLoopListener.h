#ifndef TEST_I_EVENT_LOOP_LISTENER
#define TEST_I_EVENT_LOOP_LISTENER

namespace test {

/// Helper interface. Used to signal in the tests that the event loop
/// has been activated, thus the application is properly started
/// for running tests.
struct IEventLoopListener
{
    virtual void OnEventLoopEnter() = 0;
};

} // namespace

#endif // TEST_I_EVENT_LOOP_LISTENER
