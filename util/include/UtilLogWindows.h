#ifndef UTIL_LOG_WINDOWS_H
#define UTIL_LOG_WINDOWS_H

#include <windows.h>

std::ostream& operator<< (std::ostream& os, const EXCEPTION_POINTERS& obj);

#endif // UTIL_LOG_WINDOWS_H