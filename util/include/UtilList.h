#ifndef UTIL_LIST_H
#define UTIL_LIST_H

#include "UtilLog.h"
#include "UtilLogStl.h"

template<class ELEMENT>
class UtilList
{
public:
    typedef std::list<ELEMENT> ELEMENTS;

    /// Create a new UtilList helper object
    /// \param list list which is operated upon
    UtilList(ELEMENTS& list)
        :   mList(list)
    {
    }

    /// Add elements to the list
    /// \param added elements to be added
    /// \param position elements are added AFTER the given element. If this equals a null ptr, then the elements are added at the end.
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

    /// \return the position at which the element are removed.
    /// \param removed list of elements to be removed. These elements should be in the same order in the list, without other objects in between.
    ELEMENT removeElements(ELEMENTS removed)
    {
        ELEMENTS::iterator itBegin = find(mList.begin(), mList.end(), removed.front());
        ASSERT(itBegin != mList.end())(removed.front()); // Ensure that the begin was found

        ELEMENTS::iterator itLast = find(itBegin, mList.end(), removed.back());
        ASSERT(itLast != mList.end())(removed.back()); // Ensure that the end was found

        ++itLast; // See http://www.cplusplus.com/reference/stl/list/erase: one but last is removed

        ELEMENT position = ELEMENT();
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

    void replace(ELEMENT oldElement, ELEMENT newElement)
    {
        ELEMENTS::iterator itOldElement = find(mList.begin(), mList.end(), oldElement);
        ASSERT(itOldElement != mList.end())(oldElement); // Ensure that the element was found

        ELEMENTS::iterator itNewElement = mList.erase(itOldElement);
        mList.insert(itNewElement,newElement);
        VAR_DEBUG(oldElement)(newElement)(mList);
    }

    /// \return true if the list contains the given element
    /// \param element element which is searched for in the list
    bool hasElement(ELEMENT element)
    {
        return (find(mList.begin(), mList.end(), element) != mList.end());
    }

    /// Add elements from the set at the end of the list
    /// \param set the set of elements to be added
    void addElements(std::set<ELEMENT> set)
    {
        BOOST_FOREACH( ELEMENT element, set )
        {
            mList.push_back(element);
        }
    }

private:
    ELEMENTS& mList;
};

#endif // UTIL_LIST_H