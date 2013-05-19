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
// Log::SetReportingLevel(LogDebug);
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

#include "UtilEnum.h"

DECLAREENUM(LogLevel, \
            LogNone, \
            LogError, \
            LogWarning, \
            LogInfo, \
            LogDebug, \
            LogVideo, \
            LogAudio, \
            LogDetail, \
            LogAssert);

////////////////////////////////////////////////////////////////////////////////
// LOGGING MACROS
////////////////////////////////////////////////////////////////////////////////

#define LOG_X(level) if (level <= Log::sReportingLevel) Log().get(level,  __FILE__, __LINE__, __FUNCTION__)
#define VAR_X(level) if (level <= Log::sReportingLevel) LogVar(level,  __FILE__, __LINE__, __FUNCTION__).LOGVAR_A

#define LOG_DETAIL  LOG_X(LogDetail)
#define LOG_AUDIO   LOG_X(LogAudio)
#define LOG_VIDEO   LOG_X(LogVideo)
#define LOG_DEBUG   LOG_X(LogDebug)
#define LOG_INFO    LOG_X(LogInfo)
#define LOG_WARNING LOG_X(LogWarning)
#define LOG_ERROR   LOG_X(LogError)

#define VAR_DETAIL  VAR_X(LogDetail)
#define VAR_AUDIO   VAR_X(LogAudio)
#define VAR_VIDEO   VAR_X(LogVideo)
#define VAR_DEBUG   VAR_X(LogDebug)
#define VAR_INFO    VAR_X(LogInfo)
#define VAR_WARNING VAR_X(LogWarning)
#define VAR_ERROR   VAR_X(LogError)

#define ASSERT_EQUALS(value1,value2)                ASSERT(value1 == value2)(value1)(value2)
#define ASSERT_DIFFERS(value1,value2)               ASSERT(value1 != value2)(value1)(value2)
#define ASSERT_MORE_THAN(value1,value2)             ASSERT(value1 >  value2)(value1)(value2)
#define ASSERT_MORE_THAN_EQUALS(value1,value2)      ASSERT(value1 >= value2)(value1)(value2)
#define ASSERT_LESS_THAN(value1,value2)             ASSERT(value1 < value2)(value1)(value2)
#define ASSERT_LESS_THAN_EQUALS(value1,value2)      ASSERT(value1 <= value2)(value1)(value2)
#define ASSERT_IMPLIES(value1,value2)               if ((!(value1) || value2)) ; else  LogVar(#value1 ## " ==> " #value2,   __FILE__, __LINE__,__FUNCTION__).LOGVAR_A
#define ASSERT_CONTAINS(container,value)            ASSERT(find(container.begin(),container.end(),value) != container.end())(container)(value)
#define ASSERT_CONTAINS_NOT(container,value)        ASSERT(find(container.begin(),container.end(),value) == container.end())(container)(value)
#define ASSERT_MAP_CONTAINS(container,value)        ASSERT(container.find(value) != container.end())(#container)(value)
#define ASSERT_MAP_CONTAINS_NOT(container,value)    ASSERT(container.find(value) == container.end())(#container)(value)

#define ASSERT_ZERO(value)                          ASSERT(value == 0)(value)
#define ASSERT_NONZERO(value)                       ASSERT(value != 0)(value)
#define ASSERT_MORE_THAN_ZERO(value)                ASSERT(value >  0)(value)
#define ASSERT_MORE_THAN_EQUALS_ZERO(value)         ASSERT(value >= 0)(value)
#define ASSERT_LESS_THAN_ZERO(value)                ASSERT(value <  0)(value)
#define ASSERT_LESS_THAN_EQUALS_ZERO(value)         ASSERT(value <= 0)(value)

#define ASSERT(expr)    if ((expr)) ; else  LogVar(#expr,   __FILE__, __LINE__,__FUNCTION__).LOGVAR_A
#define FATAL                               LogVar("FATAL", __FILE__, __LINE__,__FUNCTION__).LOGVAR_A
#define NIY(expr)                           LogVar(expr,   __FILE__, __LINE__,__FUNCTION__).LOGVAR_A

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

    /// Initialize logging.
    /// \param testApplicationName fixed string used for module test application name
    /// \param applicationName current application name
    static void init(const wxString& testApplicationName, const wxString& applicationName);
    static void exit();

    static LogLevel sReportingLevel;
    static void setReportingLevel(LogLevel level);
    static std::string getFileName();
    static bool isEnabled(); ///< \return true if log file has been opened
    static void flush(); ///< flush the logging. NOTE: Only use for testing!

    /// Log info in a separate line, preceded with meta data.
    std::ostringstream& get(LogLevel level, const char* p_szFileName, size_t p_lLine, const char* p_szFunction);

    /// Log info in a separate line, without meta data.
    /// \param category identifier to be used in log file
    std::ostringstream& get(std::string category);

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
        ,   mLevel(LogAssert)
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