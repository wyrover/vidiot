#include "Worker.h"

#include "Window.h"

namespace gui {

static const unsigned int sMaximumBufferedWork = 1000;
static Worker* sCurrent = 0;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Worker::Worker()
:   mEnabled(true)
,   mFifo(sMaximumBufferedWork)
{
    sCurrent = this;
    mThread.reset(new boost::thread(boost::bind(&Worker::thread,this)));
}

Worker::~Worker()
{
	mEnabled = false;
    mFifo.flush();
    mFifo.push(WorkPtr());
    if (mThread)
    {
        mThread->join();
    }
    sCurrent = 0;
}

// static
Worker& Worker::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
}

//////////////////////////////////////////////////////////////////////////
// NEW WORK
//////////////////////////////////////////////////////////////////////////

void Worker::schedule(WorkPtr work)
{
    mFifo.push(work);
}

//////////////////////////////////////////////////////////////////////////
// THE THREAD
//////////////////////////////////////////////////////////////////////////

void Worker::thread()
{
    WorkPtr w;
    while (mEnabled)
    {
        w = mFifo.pop();

        if (w) // Check needed for the case that the fifo is aborted (and thus returns a 0 shared ptr)
        {
            Window::get().setProcessingText(w->getDescription());
            w->execute();
            Window::get().setProcessingText(_(""));
        }
    }
}

} //namespace