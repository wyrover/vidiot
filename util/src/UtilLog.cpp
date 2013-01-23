#include "UtilLog.h"

#include "Config.h"
#include "UtilAssert.h"
#include "UtilFifo.h"
#include <share.h> // _SH_DENYWR
#include <time.h>

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
        :   mEnabled(false)
        ,   mFifo(sMaximumBufferedLoglines)
        ,   mFile(0)
    {
        mFile = _fsopen(sFilename.c_str(),"w",_SH_DENYWR);
        if (!mFile)
        {
            // If file open fails, mFile == 0. Then, nothing will be logged.
            // Scenario to test this: First run release version of testmanual. Close application window, but not the dos box. Then run debug version. File is already in use.
            mEnabled = false;
        }
        else
        {
            int result = setvbuf ( mFile , 0 , _IOFBF , 102400 );
            if (result == 0)
            {
                mEnabled = true;
                mThread.reset(new boost::thread(boost::bind(&LogWriter::thread,this)));
            }
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
    void flush()
    {
        if (mFile)
        {
            fflush(mFile);
        }
    }
    bool isEnabled() const
    {
        return mEnabled;
    }
private:
    void thread()
    {
        std::string s;
        time_t previous = time(0);
        while (mEnabled)
        {
            s = mFifo.pop();
            fwrite(s.c_str(), 1, s.length(), mFile);
            //if (time(0) - previous > 1) // 1s
            {
                previous = time(0);
                fflush(mFile);
            }
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
        }
        fflush(mFile);
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

bool Log::isEnabled()
{
    return sWriter->isEnabled();
}

void Log::flush()
{
    sWriter->flush();
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

#ifdef _DEBUG
static int lastline;
static std::string lastfile;
#endif // _DEBUG

std::ostringstream& Log::get(LogLevel level, const char* p_szFileName, size_t p_lLine, const char* p_szFunction)
{
    // NOTE: Ensure the same width for the strings below.
    static const char* levelstring[] =
    {
        "NONE    ",
        "ERROR   ",
        "WARNING ",
        "INFO    ",
        "DEBUG   ",
        "VIDEO   ",
        "AUDIO   ",
        "DETAIL  ",
        "ASSERT  "
    };
#ifdef _DEBUG
    // This is done for detecting crashes upon shutdown. Often there's a crash because log lines are made AFTER the logging has been closed.
    // That results in out-of-stack-space errors, wxMBConv::FromWChar<->wxMBConv::WC2MB recursively (for converting the log time stamp, see the 'get' method).
    // Where the log line was made can be seen by inspecting these two variables.
    lastline = p_lLine;
    lastfile = p_szFileName;
#endif // _DEBUG
    get(levelstring[level])
        << p_szFileName
        << '(' << std::dec << p_lLine  << ") "
        << p_szFunction
        << ' ';
    return os;
}

std::ostringstream& Log::get(std::string category)
{
    os << wxDateTime::UNow().Format(wxT("%d-%m-%Y %H:%M:%S.%l "))
        << category
        << "t@" << std::setw(4) << std::setfill('0') << std::hex <<  wxThread::GetCurrentId()
        << ' ';
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