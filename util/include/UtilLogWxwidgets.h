#ifndef UTIL_LOG_WXWIDGETS_H
#define UTIL_LOG_WXWIDGETS_H

#include <wx/gdicmn.h>
#include <wx/filename.h>

std::ostream& operator<< (std::ostream& os, const wxFileName& obj);
std::ostream& operator<< (std::ostream& os, const wxPoint& obj);

#endif //UTIL_LOG_WXWIDGETS_H

