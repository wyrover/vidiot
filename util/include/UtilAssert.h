#ifndef UTIL_ASSERT_H
#define UTIL_ASSERT_H

#include <string>

#define ASSERT(expr)    if ((expr)) ; else  LogVar(#expr,   __FILE__, __LINE__,__FUNCTION__).LOGVAR_A
#define FATAL                               LogVar("FATAL", __FILE__, __LINE__,__FUNCTION__).LOGVAR_A
#define NIY                                 LogVar("NIY",   __FILE__, __LINE__,__FUNCTION__).LOGVAR_A

////////////////////////////////////////////////////////////////////////////////
// IASSERT
////////////////////////////////////////////////////////////////////////////////

/// Class implementing this interface is responsible for handling the occurrence
/// of a assertion failure (present debug report).
struct IAssert
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    IAssert();
    virtual~IAssert();

    //////////////////////////////////////////////////////////////////////////
    // THE INTERFACE
    //////////////////////////////////////////////////////////////////////////

    virtual void onAssert() = 0;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// To indicate unfinished features
    static void niy(const char* p_szFileName, size_t p_lLine, const char* p_szFunction);

    /// Break into debugger for debug builds
    /// Trigger onAssert for other builds, to generate debug report
    static void breakIntoDebugger(const std::string& message);
};

#endif //UTIL_ASSERT_H

