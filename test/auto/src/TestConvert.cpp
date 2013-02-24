#include "TestConvert.h"

#include "Convert.h"
#include "HelperTestSuite.h"
#include "Properties.h"
#include "UtilLog.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

RUNWITHOUTGUI(testTimeConversions);
//RUNONLY(testTimeConversions);
void TestConvert::testTimeConversions()
{
    StartTestSuite();

    {
        model::Properties properties(FrameRate::s25p);
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(0),"00.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(25),"01.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(50),"02.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(500),"20.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(5000),"03:20.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(500000),"05:33:20.000");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(3),"00.120");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(27),"01.080");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(60),"02.400");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(522),"20.880");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(5007),"03:20.280");
        ASSERT_EQUALS(model::Convert::ptsToHumanReadibleString(500024),"05:33:20.960");
    }
}

} // namespace