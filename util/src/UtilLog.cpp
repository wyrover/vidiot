#include "UtilLog.h"

#include <boost/assign/list_of.hpp>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <share.h> // _SH_DENYWR
#include <string>
#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>
#include <wx/thread.h>
#include "UtilFifo.h"
#include "UtilAssert.h"
#include "Config.h"

IMPLEMENTENUM(LogLevel);

boost::bimap<LogLevel, wxString> LogLevelConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<LogLevel, wxString>::relation >
    (LogError,     _("Errors"))
    (LogWarning,   _("Errors & Warnings"))
    (LogInfo,      _("Info (UI actions)"))
    (LogDebug,     _("Debugging"))
    (LogVideo,     _("Video details"))
    (LogAudio,     _("Audio details"))
    (LogDetail,    _("All"));

static std::string sFilename("");
static const unsigned int sMaximumBufferedLoglines = 1000;

LogLevel Log::sReportingLevel = LogWarning;

////////////////////////////////////////////////////////////////////////////////
// THREAD SAFETY & WRITING TO DISK
////////////////////////////////////////////////////////////////////////////////

class LogWriter;
static LogWriter* sWriter = 0;

static std::string sEndString("END");

class LogWriter : boost::noncopyable
{
public:
    LogWriter()
        :   mEnabled(true)
        ,   mFifo(sMaximumBufferedLoglines)
        ,   mFile(0)
    {
        mFile = _fsopen(sFilename.c_str(),"w",_SH_DENYWR);
        if (!mFile)
        {
            // If file open fails, mFile == 0. Then, nothing will be logged.
            mEnabled = false;
        }
        else
        {
            mThread.reset(new boost::thread(boost::bind(&LogWriter::thread,this)));
        }
    }
    ~LogWriter()
    {
        mEnabled = false;
        if (mThread)
        {
            mFifo.push(sEndString); // To avoid deadlock if the thread() code has just passed the 'while (enabled)' guard and is waiting for a new log message
            mThread->join();
        }
        mThread.reset();
        if (mFile != 0)
        {
            fclose(mFile);
            mFile = 0;
        }
        mFifo.flush();
    }
    void write(const std::string& logLine)
    {
        if (mEnabled)
        {
            mFifo.push(logLine);
        }
    }
private:
    void thread()
    {
        std::string s;
        while (mEnabled)
        {
            s = mFifo.pop();
            fprintf(mFile, "%s", s.c_str());
            fflush(mFile);
        }
        // Log the remaining lines. This is useful for the
        // case of a failed assertion. breakIntoDebugger()
        // causes the addition of new log lines to stop.
        // (via Log::exit()->~LogWriter()->mEnabled=false)
        // However, the assert message has already been pushed,
        // and must still be logged. This also helps in ensuring
        // that codec issues are logged via the ffmpeg decoding
        // hook.
        while (mFifo.getSize() > 0)
        {
            s = mFifo.pop();
            fprintf(mFile, "%s", s.c_str());
            fflush(mFile);
        }
    }

    bool mEnabled;
    boost::scoped_ptr<boost::thread> mThread;
    Fifo<std::string> mFifo;
    FILE* mFile;
};

////////////////////////////////////////////////////////////////////////////////
// LOG CLASS
////////////////////////////////////////////////////////////////////////////////

 Log::Log()
{
}

Log::~Log()
{
    os << std::endl;
    sWriter->write(os.str());
}

void Log::setReportingLevel(LogLevel level)
{
    ASSERT_LESS_THAN(level,LogLevel_MAX);
    sReportingLevel = level;
}

std::string Log::getFileName()
{
    return sFilename;
}

void Log::init(const wxString& testApplicationName, const wxString& applicationName)
{
    // File name initialization:
    // - Normally, <Temp>/ApplicationName_ProcessId.log is used.
    // - For module test, <Cwd>/Application::sTestApplicationName.log is used.
    wxString logFileName(applicationName);
    if (applicationName.IsSameAs(testApplicationName))
    {
        logFileName << ".log"; // For test, log file with fixed name in same dir as config file.
        logFileName = wxFileName(wxFileName::GetCwd(),logFileName).GetFullPath(); // Must be full path for debug report
    }
    else
    {
        logFileName << "_" << wxGetProcessId() << ".log";
        logFileName = wxFileName(wxStandardPaths::Get().GetTempDir(),logFileName).GetFullPath();	// Default in TEMP
    }
    sFilename = std::string(logFileName);

    // Start the logger
    sWriter = new LogWriter();
}

void Log::exit()
{
    delete sWriter;
    sWriter = 0;
}

std::ostringstream& Log::get(LogLevel level, const char* p_szFileName, size_t p_lLine, const char* p_szFunction)
{
    static const char* levelstring[] = {"NONE", "ERROR", "WARNING", "INFO", "DEBUG", "VIDEO", "AUDIO", "DETAIL", "ASSERT"};
    os << boost::format("%s% 8s t@%04x %s(%d) %s ") % wxDateTime::UNow().Format("%d-%m-%Y %H:%M:%S.%l") % levelstring[level] % wxThread::GetCurrentId() % p_szFileName % p_lLine % p_szFunction;
    return os;
}

std::ostringstream& Log::get(std::string category)
{
    os << boost::format("%s% 8s t@%04x ") % wxDateTime::UNow().Format("%d-%m-%Y %H:%M:%S.%l") % category % wxThread::GetCurrentId();
    return os;
}

////////////////////////////////////////////////////////////////////////////////
// CLASS LOGVARS
////////////////////////////////////////////////////////////////////////////////

LogVar::~LogVar()
{
    if (mAssert)
    {
        // NOTE: In this case, the debug break may cause the log line not to be written to file.
        //       Not an issue, since this is for developers only.
        Log().get(mLevel, mFileName, mLine, mFunction) << *mAssert << osVars.str() ;
        std::cout << std::endl << std::endl << *mAssert << std::endl << std::endl << osVars.str();
        IAssert::breakIntoDebugger(*mAssert);
    }
    else
    {
        Log().get(mLevel, mFileName, mLine, mFunction) << osVars.str();
    }
}