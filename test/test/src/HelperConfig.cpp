#include "HelperConfig.h"

#include "Config.h"
#include "HelperApplication.h"

namespace test {

ConfigOverruleBool::ConfigOverruleBool(wxString path, bool temporaryvalue)
    :   mPath(path)
    ,   mOriginalValue(Config::ReadBool(path))
    ,   mTemporaryValue(temporaryvalue)
{
    Config::WriteBool(mPath,mTemporaryValue);
    waitForIdle();
}

ConfigOverruleBool::~ConfigOverruleBool()
{
    Config::WriteBool(mPath,mOriginalValue);
    waitForIdle();
}

ConfigOverruleLong::ConfigOverruleLong(wxString path, bool temporaryvalue)
    :   mPath(path)
    ,   mOriginalValue(Config::ReadLong(path))
    ,   mTemporaryValue(temporaryvalue)
{
    Config::WriteLong(mPath,mTemporaryValue);
    waitForIdle();
}

ConfigOverruleLong::~ConfigOverruleLong()
{
    Config::WriteLong(mPath,mOriginalValue);
    waitForIdle();
}

} // namespace