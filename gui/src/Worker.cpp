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
    //mThread.reset(new boost::thread(boost::bind(&Worker::thread,this)));
}

Worker::~Worker()
{
	mEnabled = false;
    mFifo.flush();
    mFifo.push(WorkPtr());
    if (mThread)
    {
        mThread->join(); /** /todo detach() ? to avoid having to wait for a long autofolder indexing action?? */
    }
}

// static
Worker& Worker::get()
{
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
        /** \todo make the if (w) checking for all fifo's or find better way. */

        if (w) // Check needed for the case that the fifo is aborted (and thus returns a 0 shared ptr)
        {
            Window::get().SetProcessingText(w->getDescription());
            w->execute();
            Window::get().SetProcessingText(_(""));
        }
    }
}

} //namespace
