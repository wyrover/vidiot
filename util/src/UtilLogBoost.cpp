#include "UtilLogBoost.h"

std::ostream& operator<< (std::ostream& os, const boost::filesystem::path& obj)
{
    os << obj.string();
    return os;
}
