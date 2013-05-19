#ifndef HELPER_THREAD_H
#define HELPER_THREAD_H

#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace test {

typedef boost::function<void()> Function;

/// Schedule a call to be ran in the main gui thread.
/// May only be called from non gui threads.
/// Schedules an event that calls the provided function.
/// \param f function to be called in gui thread.
void RunInMainThread(Function f);

} // namespace

#endif // HELPER_THREAD_H