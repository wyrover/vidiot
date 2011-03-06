#ifndef UTIL_LIST_H
#define UTIL_LIST_H

#include <list>
#include "UtilLog.h"

template<class ELEMENT>
class UtilList
{
public:
    typedef std::list<ELEMENT> ELEMENTS;

    UtilList(ELEMENTS& list)
        :   mList(list)
    {
    }
    void addElements(ELEMENTS added, ELEMENT position)
    {
        ELEMENTS::iterator itPosition = find(mList.begin(), mList.end(), position);
        // NOT: ASSERT(itPosition != mList.end()) Giving a null pointer results in mList.end() which results in adding at the end

        // See http://www.cplusplus.com/reference/stl/list/splice:
        // elements will be removed from this list. Hence, a copy is made (call by value
        // in the arguments list) before doing the splice call. Furthermore, we can use
        // the original lists for doing the logging.
        ELEMENTS tobeadded = added; // Needed for logging statement below. Without this copy, nothing is logged as 'added' (which is emptied by 'splice').
        mList.splice(itPosition,tobeadded); // See http://www.cplusplus.com/reference/stl/list/splice: elements added BEFORE position
        VAR_DEBUG(added)(position)(mList);
    }
    /** @return the position at which the element are removed. */
    ELEMENT removeElements(ELEMENTS removed)
    {
        ELEMENTS::iterator itBegin = find(mList.begin(), mList.end(), removed.front());
        ASSERT(itBegin != mList.end())(removed.front()); // Ensure that the begin was found

        ELEMENTS::iterator itLast = find(itBegin, mList.end(), removed.back());
        ASSERT(itLast != mList.end())(removed.back()); // Ensure that the end was found

        ++itLast; // See http://www.cplusplus.com/reference/stl/list/erase: one but last is removed

        ELEMENT position;
        if (itLast != mList.end())
        {
            // Only if the last element is not removed then position must be set to that last element.
            // Otherwise position is a null pointer indicating 'at end'.
            position = *itLast;
        }

        mList.erase(itBegin,itLast);
        VAR_DEBUG(removed)(position)(mList);

        return position;
    }
    bool hasElement(ELEMENT element)
    {
        return (find(mList.begin(), mList.end(), element) != mList.end());
    }
private:
    std::list<ELEMENT>& mList;
};

#endif // UTIL_LIST_H
