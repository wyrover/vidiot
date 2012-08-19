#ifndef HELPER_CONFIG_H
#define HELPER_CONFIG_H

#include "Config.h"

namespace test {

template <class TYPE>
class ConfigOverrule
{
public:
    ConfigOverrule(wxString path, TYPE temporaryvalue)
        :   mPath(path)
        ,   mOriginalValue(Config::readWithoutDefault<TYPE>(path))
        ,   mTemporaryValue(temporaryvalue)
    {
        wxConfigBase::Get()->Write(mPath,mTemporaryValue);
    }
    virtual ~ConfigOverrule()
    {
        wxConfigBase::Get()->Write(mPath,mOriginalValue);
    }
private:
    wxString mPath;
    TYPE mOriginalValue;
    TYPE mTemporaryValue;
};

} // namespace

#endif // HELPER_CONFIG_H