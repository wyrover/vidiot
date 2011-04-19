#include <cxxtest/TestSuite.h>

#include "UtilList.h"
#include <list>
#include <boost/assign/list_of.hpp>
#include "UtilLogStl.h"
#include "UtilLog.h"
#include "VideoFile.h"

class MyTestSuite : public CxxTest::TestSuite 
{
public:
    void testAddition();
};