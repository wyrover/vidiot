#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

class MyTestSuite : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
{
public:
    void testAddition();
    void testStartup();

private:
    boost::scoped_ptr<boost::thread> mThread;
    void thread();
};