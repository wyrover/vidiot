// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#define BOOST_HAS_STDINT_H

template<class ELEMENT>
class Fifo
{
public:
    explicit Fifo(const size_t& maxSize)
        :   mMaxSize(maxSize)
    {
    }

    Fifo(const Fifo& other) = delete;
    Fifo& operator=(const Fifo&) = delete;
    ~Fifo() = default;

    size_t getSize()
    {
        boost::mutex::scoped_lock lock(mMutex);
        return mSize;
    }

    void resize(const size_t& newMaxSize)
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
    void push(const ELEMENT& e)
    {
        {
            boost::mutex::scoped_lock lock(mMutex);

            while (items.size() == mMaxSize)
            {
                conditionNotFull.wait(lock);
            }
            items.push_back(e);
            ++mSize;
            ASSERT_MORE_THAN_ZERO(mSize);
        }
        conditionNotEmpty.notify_all();
    }

private:
    size_t mMaxSize = 0;
    std::deque<ELEMENT> items;

    boost::mutex mMutex;
    boost::condition_variable conditionNotEmpty;
    boost::condition_variable conditionNotFull;

    size_t mSize = 0;
};
