#ifndef LOG_H__
#define LOG_H__

#if _MSC_VER > 1000
// *this used in base-member initialization; it's ok
#pragma warning ( disable : 4355)
#endif

//////////////////////////////////////////////////////////////////////////
// STARTUP/SHUTDOWN
//
// Must be called before doing any logging:
// Log::Init();
//
// Must be called when shutting down:
// Log::Terminate();
//
// The log level is set as follows
// Log::SetReportingLevel(logDEBUG);
//
//////////////////////////////////////////////////////////////////////////
// LOGGING
//
// Logging is done with the following macros:
//
// LOG_VIDEO   << <whatever> << <etc>; --> For detailed logging of video packets/frames/pts/etc. May cause performance drop.
// LOG_AUDIO   << <whatever> << <etc>; --> For detailed logging of audio packets/chunks/pts/etc. May cause performance drop.
// LOG_DEBUG   << <whatever> << <etc>; --> For debugging program flow   (example: button was already pressed)
// LOG_INFO    << <whatever> << <etc>; --> User actions                 (example: button press, resize window)
// LOG_WARNING << <whatever> << <etc>; --> Non critical exceptions      (example: audio not available temporarily due to performance issue)
// LOG_ERROR   << <whatever> << <etc>; --> Critical errors              (example: could not open audio device)
//
// VAR_VIDEO  (<expr1>)...(<exprn>);
// VAR_AUDIO  (<expr1>)...(<exprn>);
// VAR_DEBUG  (<expr1>)...(<exprn>);
// VAR_INFO   (<expr1>)...(<exprn>);
// VAR_WARNING(<expr1>)...(<exprn>);
// VAR_ERROR  (<expr1>)...(<exprn>);
//
// ASSERT(<expression>)(<expr1>)...(<exprn>); <- exprs 1 through n are shown in the log if the assert fails. This is logged only once ;-)
// FATAL               (<expr1>)...(<exprn>); <- exprs 1 through n are shown in the log
//
// NOTE: Assert is not a log level: these are logged always.
//       Highest loglevel that can be set is detailed.
// NOTE: When log level is set to audio or video all other log
//       statements are suppressed. Use detailed to log everything.
//
// Credits:
//
// Petru Marginean
//      Logging In C++: A typesafe, threadsafe, portable logging mechanism
//      Dr. Dobbs journal, September 05, 2007
//      http://www.ddj.com/cpp/201804215
//
// Andrei Alexandrescu and John Torjo
//      Enhancing Assertions
//      Dr. Dobbs journal, August 01, 2003
//      http://www.ddj.com/cpp/184403745


#include <sstream>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include "UtilEnum.h"

DECLAREENUM(LogLevel, \
            logNONE, \
            logERROR, \
            logWARNING, \
            logINFO, \
            logDEBUG, \
            logVIDEO, \
            logAUDIO, \
            logDETAIL, \
            logASSERT);

////////////////////////////////////////////////////////////////////////////////
// LOGGING MACROS
////////////////////////////////////////////////////////////////////////////////

#define LOG_X(level) if (level <= Log::sReportingLevel) Log().Get(level,  __FILE__, __LINE__, __FUNCTION__)
#define VAR_X(level) if (level <= Log::sReportingLevel) LogVar(level,  __FILE__, __LINE__, __FUNCTION__).LOGVAR_A

#define LOG_DETAIL  LOG_X(logDETAIL)
#define LOG_AUDIO   LOG_X(logAUDIO)
#define LOG_VIDEO   LOG_X(logVIDEO)
#define LOG_DEBUG   LOG_X(logDEBUG)
#define LOG_INFO    LOG_X(logINFO)
#define LOG_WARNING LOG_X(logWARNING)
#define LOG_ERROR   LOG_X(logERROR)

#define VAR_DETAIL  VAR_X(logDETAIL)
#define VAR_AUDIO   VAR_X(logAUDIO)
#define VAR_VIDEO   VAR_X(logVIDEO)
#define VAR_DEBUG   VAR_X(logDEBUG)
#define VAR_INFO    VAR_X(logINFO)
#define VAR_WARNING VAR_X(logWARNING)
#define VAR_ERROR   VAR_X(logERROR)

#define ASSERT(expr)    if ((expr)) ; else  LogVar(#expr,   __FILE__, __LINE__,__FUNCTION__).LOGVAR_A
#define FATAL                               LogVar("FATAL", __FILE__, __LINE__,__FUNCTION__).LOGVAR_A
#define NIY                                 LogVar("NIY",   __FILE__, __LINE__,__FUNCTION__).LOGVAR_A



////////////////////////////////////////////////////////////////////////////////
// LOG CLASS
////////////////////////////////////////////////////////////////////////////////

/// This class is responsible for appending a line to the logging. Each line
/// is prefixed with fixed information (time, log level, thread id, file name
/// and line, and method name). A separate class LogWriter (declared in * .cpp file)
/// is used for guaranteeing thread-safety of logging these messages.
/// Actual logging is done when this object's destructor is called.
class Log
    :   boost::noncopyable
{
public:

    Log();
    virtual ~Log();

    static void Init();
    static void Terminate();

    static LogLevel sReportingLevel;
    static void SetReportingLevel(LogLevel level);
    static void SetLogFile(std::string path);

    /// Log info in a separate line, preceded with meta data.
    std::ostringstream& Get(LogLevel level, const char* p_szFileName, size_t p_lLine, const char* p_szFunction);

private:
    std::ostringstream os;
};

////////////////////////////////////////////////////////////////////////////////
// LOGVARS CLASS
////////////////////////////////////////////////////////////////////////////////

/// This class is responsible for
/// - logging a list of variables
/// - logging asserts (log expression, list of variables, and break into debugger)
/// - logging fatals (log message, list of variables, and break into debugger)
/// It uses the Log() class for appending a line to the logging.
/// Actual logging is done when this object's destructor is called.
struct LogVar
    :   boost::noncopyable
{
    
    LogVar& LOGVAR_A;   ///< Helper, in order to be able to compile the code (LOGVAR_* macros)
    LogVar& LOGVAR_B;   ///< Helper, in order to be able to compile the code (LOGVAR_* macros)

    /// Constructor for VAR_* macros.
    /// Defined in .h because LOGVARS_ macro trick below redefines
    /// LOGVAR_A and LOGVAR_B which are used in the initializer list.
    LogVar(LogLevel level, const char* p_szFileName, size_t p_lLine, const char* p_szFunction)
        :   LOGVAR_A(*this)
        ,   LOGVAR_B(*this)
        ,   mLevel(level)
        ,   mFileName(p_szFileName)
        ,   mLine(p_lLine)
        ,   mFunction(p_szFunction)
        ,   mAssert(boost::none)
    {
    }

    /// Constructor for ASSERT and FATAL macros macros.
    /// Defined in .h because LOGVARS_ macro trick below redefines
    /// LOGVAR_A and LOGVAR_B which are used in the initializer list.
    LogVar(const char * expr, const char* p_szFileName, size_t p_lLine, const char* p_szFunction)
        :   LOGVAR_A(*this)
        ,   LOGVAR_B(*this)
        ,   mLevel(logASSERT)
        ,   mFileName(p_szFileName)
        ,   mLine(p_lLine)
        ,   mFunction(p_szFunction)
        ,   mAssert(boost::optional<std::string>(std::string("[ASSERT:") + expr + ']'))
    {
    }
    
    ~LogVar();  ///< Upon destruction of this object, the actual logging is executed.

    /// Append one variable to the list of logged variables.
    template<class type>
    LogVar& logVar(const type& varValue, const char* varName)
    {
        osVars << "[" << varName << "=" << varValue << "]";
        return *this;
    }

private:
    std::ostringstream osVars;
    LogLevel mLevel;
    const char* mFileName;
    size_t mLine;
    const char* mFunction;
    boost::optional<std::string> mAssert;
};

#define LOGVAR_A(x) LOGVAR_OP(x, B)
#define LOGVAR_B(x) LOGVAR_OP(x, A)
#define LOGVAR_OP(x, next) LOGVAR_A.logVar(x, #x).LOGVAR_ ## next

#endif //LOG_H__

