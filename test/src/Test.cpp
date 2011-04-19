#include "Test.h"

#include "UtilList.h"
#include <list>
#include <boost/assign/list_of.hpp>
#include "UtilLogStl.h"
#include "UtilLog.h"
#include "VideoFile.h"

    void MyTestSuite::testAddition()
    {
        TS_ASSERT( 1 + 1 > 1 );
        TS_ASSERT_EQUALS( 1 + 1, 2 );

        model::VideoFile f;

        std::list<int> l;
        UtilList<int>(l).addElements(boost::assign::list_of(1)(2)(3),-1);
        TS_ASSERT(l.size() == 3);
        UtilList<int>(l).removeElements(boost::assign::list_of(1)(2)(3));
        TS_ASSERT(l.size() == 0);


    }
