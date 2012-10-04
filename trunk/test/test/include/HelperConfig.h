#ifndef HELPER_CONFIG_H
#define HELPER_CONFIG_H

#include "Config.h"

namespace test {

class ConfigOverruleBool
{
public:
    ConfigOverruleBool(wxString path, bool temporaryvalue);
    virtual ~ConfigOverruleBool();
private:
    wxString mPath;
    bool mOriginalValue;
    bool mTemporaryValue;
};

class ConfigOverruleLong
{
public:
    ConfigOverruleLong(wxString path, bool temporaryvalue);
    virtual ~ConfigOverruleLong();
private:
    wxString mPath;
    long mOriginalValue;
    long mTemporaryValue;
};

} // namespace

#endif // HELPER_CONFIG_H