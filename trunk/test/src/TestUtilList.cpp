#include "TestUtilList.h"

#include <list>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"
#include "UtilList.h"

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

// static 
TestExample *TestExample::createSuite()
{ 
    return new TestExample(); 
}

// static 
void TestExample::destroySuite(TestExample *suite)
{
    delete suite; 
}

TestExample::TestExample()
{
    CxxTest::setAbortTestOnFail(false); 
    Log::setFileName("TestExample.log");
    Log::SetReportingLevel(logDEBUG);
    Log::Init();
}

TestExample::~TestExample()
{
    Log::Terminate();
}


//////////////////////////////////////////////////////////////////////////
// PER TEST INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestExample::setUp() 
{ 
}

void TestExample::tearDown() 
{ 
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestExample::testAddition()
{
    LOG_DEBUG << "BEGIN TEST";
    TS_ASSERT( 1 + 1 > 1 );
    TS_ASSERT_EQUALS( 1 + 1, 2 );

    std::list<int> l;
    UtilList<int>(l).addElements(boost::assign::list_of(1)(2)(3),-1);
    ASSERT(l.size() == 3);
    UtilList<int>(l).removeElements(boost::assign::list_of(1)(2)(3));
    ASSERT(l.size() == 0);
    LOG_DEBUG << "END TEST";
}
