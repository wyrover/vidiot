#ifndef UTIL_THREAD_H
#define UTIL_THREAD_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

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

    explicit SpawnThread(Method threadmethod);
};

} // namespace

#endif // UTIL_THREAD_H

