#include "UtilLog.h"

#include <wx/datetime.h>
#include <wx/thread.h>
#include <boost/format.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>
#include "UtilFifo.h"

/** todo use wxDebugReport */
/** todo use onfatalexception */
/** todo wxCrashReport::Generate() */

IMPLEMENTENUM(LogLevel);

static std::string sFilename("D:\\newlog.txt");
static const unsigned int sMaximumBufferedLoglines = 1000;

LogLevel Log::sReportingLevel = logINFO;

////////////////////////////////////////////////////////////////////////////////
// THREAD SAFETY & WRITING TO DISK
////////////////////////////////////////////////////////////////////////////////

class LogWriter;
static LogWriter* sWriter = 0;

class LogWriter : boost::noncopyable
{
public:
    LogWriter() 
        :   mEnabled(true)
        ,   mFifo(sMaximumBufferedLoglines)
        ,   mFile(0)
    { 
        /** /todo what if file not possible (no such disk)  use boost to determine first */
        mFile = _fsopen(sFilename.c_str(),"w",_SH_DENYWR);
        mThread.reset(new boost::thread(boost::bind(&LogWriter::thread,this)));
    }
    ~LogWriter()
    {
        mEnabled = false;
        if (mThread)
        {
            mThread->join();
        }
        mThread.reset();
        if (mFile != 0)
        {
            fclose(mFile);
            mFile = 0;
        }
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
        // (via Log::terminate()->~LogWriter()->mEnabled=false)
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

void Log::SetReportingLevel(LogLevel level)
{
	if (level > logDETAIL)
	{
		sReportingLevel = logDETAIL;
	}
	else
	{
		sReportingLevel = level;
	}
}

void Log::SetLogFile(std::string path)
{
    sFilename = path;
}

void Log::Init()
{
    sWriter = new LogWriter();
}

void Log::Terminate()
{
    delete sWriter;
    sWriter = 0;
}

std::ostringstream& Log::Get(LogLevel level, char* p_szFileName, size_t p_lLine, char* p_szFunction)
{
	static const char* levelstring[] = {"NONE", "ERROR", "WARNING", "INFO", "DEBUG", "VIDEO", "AUDIO", "DETAIL", "ASSERT"};
    os << boost::format("%s % 7s: t@%04x %s(%d): %s: ") % wxDateTime::UNow().Format("%d-%m-%Y %H:%M:%S.%l") % levelstring[level] % wxThread::GetCurrentId() % p_szFileName % p_lLine % p_szFunction;
	return os;
}

////////////////////////////////////////////////////////////////////////////////
// CLASS LOGVARS
////////////////////////////////////////////////////////////////////////////////

void breakIntoDebugger()
{
    //    wxMessageOutputMessageBox().Printf("%s",*mAssert);
    Log::Terminate();
#if (defined _MSC_VER) || (defined __BORLANDC__)
    __asm { int 3 };
#elif defined(__GNUC__)
    __asm ("int $0x3");
#else
#  error Please supply instruction to break into code
#endif
}

LogVar::~LogVar()
{
    if (mAssert)
    {
        /** /todo how to guarantee that this log line is always written to file, given the fact that the next step is termination of the logging... */
        Log().Get(mLevel, mFileName, mLine, mFunction) << *mAssert << osVars.str() ;
        breakIntoDebugger();
    }
    else
    {
        Log().Get(mLevel, mFileName, mLine, mFunction) << osVars.str();
    }
}
