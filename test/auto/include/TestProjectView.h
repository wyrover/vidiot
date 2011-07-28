#ifndef TEST_PROJECT_VIEW_H
#define TEST_PROJECT_VIEW_H

#include "SuiteCreator.h"

namespace test {

class ProjectViewTests : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error 
    ,   public SuiteCreator<ProjectViewTests>
{
public:
    void testAdditionAndRemoval();
    void testCreateSequence();
};

} // namespace

using namespace test;

#endif // TEST_PROJECT_VIEW_H