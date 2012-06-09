#include "TestOnce.h"

#include "Render.h"
#include "Sequence.h"
#include "HelperWindow.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestOnce::setUp()
{
    mProjectFixture.init();
}

void TestOnce::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

//RUNONLY("testTrimming");

void TestOnce::testOnce()
{
    // BREAK();
    // getTimeline().getDump().dump();
    // PrepareSnapping(true);
    StartTestSuite();

    model::Render render(getSequence());
    render.generate();
}

} // namespace