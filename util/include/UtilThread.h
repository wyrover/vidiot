#ifndef UTIL_THREAD_H
#define UTIL_THREAD_H

namespace util { namespace thread {

/// Schedule a method to be ran in the main wxWidgets thread.
/// This is implemented by submitting an event. In the handler
/// of the event (which is called in the main wxWidgets thread)
/// the given method is called.
template <class RETURNTYPE>
class RunInMainThread
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
public:

    typedef boost::function<RETURNTYPE()> Method;

    RunInMainThread(Method method)
        :   mMethod(method)
        ,   mDone(false)
    {
        if (wxThread::IsMain())
        {
            mResult = mMethod();
        }
        else
        {
            Bind( wxEVT_THREAD, &RunInMainThread::onThreadEvent, this );
            QueueEvent(new wxThreadEvent());
            boost::mutex::scoped_lock lock(mMutex);
            while (!mDone)
            {
                mCondition.wait(lock);
            }
        }
    }

    ~RunInMainThread()
    {
        if (mDone)
        {
            Unbind( wxEVT_THREAD, &RunInMainThread::onThreadEvent, this );
        }
    }

    void onThreadEvent(wxThreadEvent& event)
    {
        mResult = mMethod();
        boost::mutex::scoped_lock lock(mMutex);
        mDone = true;
        mCondition.notify_all();
    }

    RETURNTYPE getResult() const
    {
        return mResult;
    }

private:

    RETURNTYPE mResult;
    Method mMethod;

    boost::condition_variable mCondition;
    boost::mutex mMutex;
    bool mDone;
};

}} // namespace

#endif // UTIL_THREAD_H