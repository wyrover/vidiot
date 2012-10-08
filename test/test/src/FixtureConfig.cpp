#include "FixtureConfig.h"

#include "HelperApplication.h"
#include "HelperWindow.h"
#include "HelperTimeline.h"
#include "ids.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FixtureConfig::FixtureConfig()
{
}

FixtureConfig::~FixtureConfig()
{
}

//////////////////////////////////////////////////////////////////////////
// SET
//////////////////////////////////////////////////////////////////////////

FixtureConfig& FixtureConfig::SetDefaults()
{
    return SnapToClips(false).SnapToCursor(false);
}

FixtureConfig& FixtureConfig::SnapToClips(bool snap)
{

    checkMenu(ID_SNAP_CLIPS, snap);
    waitForIdle();
    return *this;
}
FixtureConfig& FixtureConfig::SnapToCursor(bool snap)
{
    checkMenu(ID_SNAP_CURSOR, snap);
    waitForIdle();
    return *this;
}

} // namespace