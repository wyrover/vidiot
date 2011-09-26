#ifndef UTIL_THREAD_H
#define UTIL_THREAD_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <wx/thread.h>

namespace util {

typedef boost::function<void()> Method;

/// Class responsible for starting a (worker) thread. Using this class ensures
/// that exceptions are caught, resulting in a debug report. In case a non
/// wxWidgets thread model is used, showing the debug report will not work.
class SpawnThread
    :   boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Start a new thread running 'threadmethod'
    /// \param threadmethod method to be ran in the thread
    /// \param joinable if true, then the thread is joinable; getThread().Wait() must be called to avoid memory leaks. 
    explicit SpawnThread(Method threadmethod, bool joinable = false);

    /// For joinable threads, this destructor blocks until the thread is joined.
    virtual ~SpawnThread();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    //wxThread& getThread();

private:

    bool mJoinable;
    //wxThread* mThread;
    boost::scoped_ptr<boost::thread> mThread;
};

} // namespace

#endif // UTIL_THREAD_H

