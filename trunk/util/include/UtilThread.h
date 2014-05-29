// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef UTIL_THREAD_H
#define UTIL_THREAD_H

namespace util { namespace thread {

void RunInMainAndWait(const boost::function<void()> &method);
void RunInMainAndDontWait(const boost::function<void()>& method);

template <class RETURNTYPE>
class RunInMainThreadWithResult
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
public:

    RunInMainThreadWithResult(const boost::function<RETURNTYPE()>& method)
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

    virtual ~RunInMainThreadWithResult()
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

template <typename RETURNTYPE>
RETURNTYPE RunInMainReturning(const boost::function<RETURNTYPE()>& method)
{
    return RunInMainThreadWithResult<RETURNTYPE>(method).getResult();
}

void setCurrentThreadName(const char* name);

}} // namespace

#endif
