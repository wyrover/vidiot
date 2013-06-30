#ifndef UTIL_THREAD_H
#define UTIL_THREAD_H

namespace util { namespace thread {

template <typename RETURNTYPE>
RETURNTYPE RunInMainReturning(boost::function<RETURNTYPE()> method)
{
    return RunInMainThreadWithResult<RETURNTYPE>(method).getResult();
}

void RunInMainAndWait(boost::function<void()> method);
void RunInMainAndDontWait(boost::function<void()> method);

template <class RETURNTYPE>
class RunInMainThreadWithResult
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
public:

    RunInMainThreadWithResult(boost::function<RETURNTYPE()> method)
        :   mMethod(method)
        ,   mDone(false)
    {
        if (wxThread::IsMain())
        {
            mResult = mMethod();
        }
        else
        {
            Bind( wxEVT_THREAD, &RunInMainThreadWithResult::onThreadEvent, this );
            QueueEvent(new wxThreadEvent());
            boost::mutex::scoped_lock lock(mMutex);
            while (!mDone)
            {
                mCondition.wait(lock);
            }
        }
    }

    ~RunInMainThreadWithResult()
    {
        if (mDone)
        {
            Unbind( wxEVT_THREAD, &RunInMainThreadWithResult::onThreadEvent, this );
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
    boost::function<RETURNTYPE()> mMethod;
    bool mWait;

    boost::condition_variable mCondition;
    boost::mutex mMutex;
    bool mDone;
};

void setCurrentThreadName(const char* name);

}} // namespace

#endif // UTIL_THREAD_H