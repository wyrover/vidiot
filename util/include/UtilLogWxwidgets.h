#ifndef UTIL_LOG_WXWIDGETS_H
#define UTIL_LOG_WXWIDGETS_H

std::ostream& operator<< (std::ostream& os, const wxFileName& obj);
std::ostream& operator<< (std::ostream& os, const wxPoint& obj);
std::ostream& operator<< (std::ostream& os, const wxSize& obj);
std::ostream& operator<< (std::ostream& os, const wxKeyCode& obj);
std::ostream& operator<< (std::ostream& os, const wxMouseEvent& obj);
std::ostream& operator<< (std::ostream& os, const wxKeyboardState& obj);
std::ostream& operator<< (std::ostream& os, const wxRegion& obj);

#endif //UTIL_LOG_WXWIDGETS_H