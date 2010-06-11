#ifndef WX_INCLUDE_H
#define WX_INCLUDE_H

#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */ 

#ifdef _CRT_SECURE_NO_WARNINGS 
#undef _CRT_SECURE_NO_WARNINGS
#endif
#define _CRT_SECURE_NO_WARNINGS

#pragma warning(disable : 4996)// needed for strcpy use of wxwidgets

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#if wxUSE_STD_IOSTREAM
    #include "wx/ioswrap.h"
#else
    #include "wx/txtstrm.h"
#endif

#endif // WX_INCLUDE_H