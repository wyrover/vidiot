#include "Worker.h"

#include "StatusBar.h"

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
    updateQueueText();
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
        updateQueueText();

        if (w) // Check needed for the case that the fifo is aborted (and thus returns a 0 shared ptr)
        {
            w->execute();
            w.reset(); // Clear, so that unfreezing is done if needed
            StatusBar::get().setProcessingText();
            StatusBar::get().hideProgressBar();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Worker::updateQueueText()
{
    if (mFifo.getSize() == 0)
    {
        StatusBar::get().setQueueText("");
    }
    else if (mFifo.getSize() == 1)
    {
        StatusBar::get().setQueueText(_("1 item queued"));
    }
    else
    {
        StatusBar::get().setQueueText(wxString::Format("%d %s", mFifo.getSize(), _("items queued")));
    }
}

} //namespace