#include "ProjectWorker.h"
#include "GuiWindow.h"

namespace model {

static const unsigned int sMaximumBufferedWork = 1000;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectWorker::ProjectWorker()
:   mEnabled(true)
,   mFifo(sMaximumBufferedWork)
,   mWindow(gui::GuiWindow::get())
{
    //mThread.reset(new boost::thread(boost::bind(&ProjectWorker::thread,this)));
}

ProjectWorker::~ProjectWorker()
{
	mEnabled = false;
    mFifo.flush();
    mFifo.push(WorkPtr());
    if (mThread)
    {
        mThread->join(); /** /todo detach() ? to avoid having to wait for a long autofolder indexing action?? */
    }
}


//////////////////////////////////////////////////////////////////////////
// NEW WORK
//////////////////////////////////////////////////////////////////////////

void ProjectWorker::schedule(WorkPtr work)
{
    mFifo.push(work);
}

//////////////////////////////////////////////////////////////////////////
// THE THREAD
//////////////////////////////////////////////////////////////////////////

void ProjectWorker::thread()
{
    WorkPtr w;
    while (mEnabled)
    {
        w = mFifo.pop();
        /** \todo make the if (w) checking for all fifo's or find better way. */

        if (w) // Check needed for the case that the fifo is aborted (and thus returns a 0 shared ptr)
        {
            mWindow->SetProcessingText(w->getDescription());
            w->execute();
            mWindow->SetProcessingText(_(""));
        }
    }
}

} //namespace
