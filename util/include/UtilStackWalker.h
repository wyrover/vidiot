#ifndef UTIL_STACKWALKER_H
#define UTIL_STACKWALKER_H

#include <wx/stackwalk.h>

struct StackWalker
    : public wxStackWalker
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    StackWalker();
    virtual~StackWalker();

    std::string show();

    //////////////////////////////////////////////////////////////////////////
    // THE INTERFACE
    //////////////////////////////////////////////////////////////////////////

    virtual void OnStackFrame(const wxStackFrame &frame);

private:

    std::ostringstream mOverview;
};

#define LOG_STACKTRACE \
    Log().get("STACK   ") << StackWalker().show();

#endif // UTIL_STACKWALKER_H