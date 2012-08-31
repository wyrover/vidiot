#ifndef FIFO_H
#define FIFO_H

#define BOOST_HAS_STDINT_H

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

    /// Get the front ELEMENT and remove it. Blocks until a element is available.
    /// \return found element
    ELEMENT pop()
    {
        boost::mutex::scoped_lock lock(mMutex);

        while (items.empty())
        {
            conditionNotEmpty.wait(lock);
        }
        ELEMENT e = items.front();
        items.pop_front();
        --mSize;
        ASSERT_MORE_THAN_EQUALS_ZERO(mSize);
        conditionNotFull.notify_all();
        return e;
    }

    /// Inserts an object in the list.
    /// If list is full, blocks until list is no longer full.
    void push(ELEMENT e)
    {
        {
            boost::mutex::scoped_lock lock(mMutex);

            while (items.size() == mMaxSize)
            {
                VAR_WARNING(this);
                conditionNotFull.wait(lock);
            }
            items.push_back(e);
            ++mSize;
            ASSERT_MORE_THAN_ZERO(mSize);
        }
        conditionNotEmpty.notify_all();
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