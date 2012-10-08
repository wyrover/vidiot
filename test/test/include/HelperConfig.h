#ifndef HELPER_CONFIG_H
#define HELPER_CONFIG_H

#include "Config.h"

namespace test {

/// Creating an overrule object temporarily overrules the (bool) value of a Config setting.
/// When the object is created, the temporary value is set.
/// When the object is destroyed, the original value is reset.
/// \note Changing config settings is done by accessing the application's Config object directly, not via UI actions.
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

/// Creating an overrule object temporarily overrules the (Long) value of a Config setting.
/// When the object is created, the temporary value is set.
/// When the object is destroyed, the original value is reset.
/// \note Changing config settings is done by accessing the application's Config object directly, not via UI actions.
class ConfigOverruleLong
{
public:
    ConfigOverruleLong(wxString path, long temporaryvalue);
    virtual ~ConfigOverruleLong();
private:
    wxString mPath;
    long mOriginalValue;
    long mTemporaryValue;
};

} // namespace

#endif // HELPER_CONFIG_H