#ifndef WORKER_H
#define WORKER_H

#include <boost/scoped_ptr.hpp>
#include "FifoWork.h"

namespace gui {

/// This class is responsible for running lengthy tasks in the
/// background.
class Worker
    :   public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Worker();

    ~Worker();

    static Worker& get();

    //////////////////////////////////////////////////////////////////////////
    // NEW WORK
    //////////////////////////////////////////////////////////////////////////

    void schedule(WorkPtr work);

private:

    bool mEnabled;
    boost::scoped_ptr<boost::thread> mThread;
    FifoWork mFifo;

    void thread();
};

} // namespace

#endif // WORKER_H
