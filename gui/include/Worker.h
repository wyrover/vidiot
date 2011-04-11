#ifndef PROJECT_WORKER_H
#define PROJECT_WORKER_H

#include <boost/scoped_ptr.hpp>
#include "FifoWork.h"

namespace gui {
class GuiWindow;
}

namespace model {

/** todo move similar to watcher class. This one however should be started by window? or by main?
* This class is responsible for running lengthy Project tasks
* like indexing an autofolder, or rendering preview files in the
* background.
*/
class ProjectWorker
    :   public boost::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ProjectWorker();

    ~ProjectWorker();

    //////////////////////////////////////////////////////////////////////////
    // NEW WORK
    //////////////////////////////////////////////////////////////////////////

    void schedule(WorkPtr work);

private:

    bool mEnabled;
    boost::scoped_ptr<boost::thread> mThread;
    FifoWork mFifo;

    void thread();

    gui::GuiWindow* mWindow;
};

} // namespace

#endif // PROJECT_WORKER_H
