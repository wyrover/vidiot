#include "TestUtilList.h"

#include <list>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilList.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestExample::testAddRemove()
{
    std::list<int> l;
    UtilList<int>(l).addElements(boost::assign::list_of(1)(2)(3),-1);
    ASSERT(l.size() == 3);
    UtilList<int>(l).removeElements(boost::assign::list_of(1)(2)(3));
    ASSERT(l.size() == 0);
}

} // namespace