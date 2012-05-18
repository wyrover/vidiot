#include "TestManual.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "Constants.h"
#include "HelperWindow.h"
#include "Convert.h"
#include "Layout.h"
#include "HelperModel.h"
#include "Details.h"
#include "DetailsClip.h"
#include "VideoClip.h"
#include "UtilLogWxwidgets.h"

namespace test {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestManual::setUp()
{
    mProjectFixture.init();
}

void TestManual::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

template <class ITEMTYPE>
void ClickOnEnumSelector(EnumSelector<ITEMTYPE>* widget, ITEMTYPE value)
{
    ClickTopLeft(widget);
    Type(WXK_HOME);
    for (int step = 0; step < widget->getIndex(value); ++step)
    {
        Type(WXK_DOWN);
    }
    Type(WXK_RETURN);
    ASSERT_EQUALS(widget->getValue(),value);
    waitForIdle();
}

void TestManual::testManual()
{
    StartTest("Manual...");
    pause(60000 * 60); // One hour should be enough...
}
} // namespace