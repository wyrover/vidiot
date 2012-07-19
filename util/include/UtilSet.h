#ifndef UTIL_SET_H
#define UTIL_SET_H

#include "UtilLog.h"
#include "UtilLogStl.h"

template<class ELEMENT>
class UtilSet
{
public:
    typedef std::set<ELEMENT> ELEMENTS;

    /// Create a new UtilSet helper object
    /// \param set set which is operated upon
    UtilSet(ELEMENTS& set)
        :   mSet(set)
    {
    }

    /// Add elements to the set
    /// \param added elements to be added
    void addElements(ELEMENTS added)
    {
        mSet.insert(added.begin(),added.end());
        VAR_DEBUG(added)(mSet);
    }

    /// Add one element to the set
    /// \param add element to be added
    void addElement(ELEMENT add)
    {
        mSet.insert(add);
        VAR_DEBUG(add)(mSet);
    }

    /// \param removed elements to be removed
    void removeElements(ELEMENTS removed)
    {
        mSet.erase(removed.begin(),removed.end());
        VAR_DEBUG(removed)(mSet);
    }

    /// Add elements from the list to the set
    /// \param set the set of elements to be added
    void addElements(std::list<ELEMENT> list)
    {
        mSet.insert(list.begin(),list.end());
    }

private:

    ELEMENTS& mSet;

};

#endif // UTIL_SET_H