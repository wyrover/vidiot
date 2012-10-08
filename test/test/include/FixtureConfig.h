#ifndef FIXTURE_CONFIG_H
#define FIXTURE_CONFIG_H

namespace test {

/// This fixture may only be used when a GUI is also used.
class FixtureConfig
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    FixtureConfig();
    ~FixtureConfig();

    //////////////////////////////////////////////////////////////////////////
    // SET
    //////////////////////////////////////////////////////////////////////////

    FixtureConfig& SetDefaults();
    FixtureConfig& SnapToClips(bool snap);
    FixtureConfig& SnapToCursor(bool snap);
};

} // namespace

#endif // FIXTURE_CONFIG_H