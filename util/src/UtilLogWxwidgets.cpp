#include "UtilLogWxwidgets.h"

std::ostream& operator<< (std::ostream& os, const wxFileName& obj)
{
    os << obj.GetFullPath();
    return os;
}

std::ostream& operator<< (std::ostream& os, const wxPoint& obj)
{
    os << '(' << obj.x << ',' << obj.y << ')';
    return os;
}
