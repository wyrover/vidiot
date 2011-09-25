#ifndef FIFO_H
#define FIFO_H

#define BOOST_HAS_STDINT_H

#include <list>
#include <string>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include "UtilLog.h"

template<class ELEMENT>
class Fifo : boost::noncopyable
{
public:
    explicit Fifo(unsigned int maxSize)
        :   mMaxSize(maxSize)
        ,	mSize(0)
    {
    }

    ~Fifo()
    {
    }

    long getSize()
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mSize;
    }

    void resize(long newMaxSize)
    {
        {
            boost::mutex::scoped_lock lock(mMutex);
            ASSERT_MORE_THAN(newMaxSize,mSize);
            mMaxSize = newMaxSize;
        }
        conditionNotFull.notify_all();
    }

    void flush()
    {
        {
            boost::mutex::scoped_lock lock(mMutex);
            items.clear();
            mSize = 0;
        }
        conditionNotFull.notify_all();
    }

    /**
    * Get the front ELEMENT and remove it.
    * Waits until such a packet is available.
    * When the returned ptr goes out of scope,
    * the packet is deleted also (thus, returns
    * the last reference to the packet).
    */
    ELEMENT pop()
    {
        boost::mutex::scoped_lock lock(mMutex);

        while (items.empty())
        {
            conditionNotEmpty.wait(lock);
        }
        ELEMENT frame = items.front();
        items.pop_front();
        --mSize;
        ASSERT_MORE_THAN_EQUALS_ZERO(mSize);
        conditionNotFull.notify_all();
        return frame;
    }

    /**
    * Inserts a copy of this packet in the list.
    * @return new size of queue
    */
    long push(ELEMENT frame)
    {
        {
            boost::mutex::scoped_lock lock(mMutex);

            while (items.size() == mMaxSize)
            {
                conditionNotFull.wait(lock);
            }
            items.push_back(frame);
            ++mSize;
            ASSERT_MORE_THAN_ZERO(mSize);
        }
        conditionNotEmpty.notify_all();
        return mSize;
    }

private:
    long mMaxSize;
    std::list<ELEMENT> items;

    boost::mutex mMutex;
    boost::condition_variable conditionNotEmpty;
    boost::condition_variable conditionNotFull;

    long mSize;
};

#endif // FIFO_H
